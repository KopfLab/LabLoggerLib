#include "Particle.h"
#include "LoggerFunction.h"
#include "LoggerFunctionReturns.h"

Variant LoggerFunction::Command::toVariant() {
    Variant var;
    var.set("c", cmd);
    if (allow_numeric_values) {
        // numeric values are allowed (1 = shorter in JSON than true)
        var.set("n", 1); 
    }
    if ( expect_value && value_optional ) {
        // value attribute is optional (1 = shorter in JSON than true)
        var.set("o", 1);
    }
    if (!text_values.isEmpty()) {
        // what are the allowed values?
        Variant vals;
        for (size_t i = 0; i < text_values.size(); ++i)
            vals.append(text_values[i].c_str());
        var.set("v", vals);
    }
    if (allow_numeric_values && !numeric_units.isEmpty()) {
        // what units are allowed?
        Variant units;
        for (size_t i = 0; i < numeric_units.size(); ++i)
            units.append(numeric_units[i].c_str());
        var.set("u",units);
    }
    return(var);
}

Variant LoggerFunction::getCommands() {
    Variant cmds;
    for (auto& cmd : m_commands) {
        // group by module to further optimize the JSON
        if (!cmds.has(cmd.module))
            cmds.set(cmd.module, Variant());
        cmds[cmd.module].append(cmd.toVariant());
    }
    return(cmds);
}

void LoggerFunction::setup() {
    Log.info("registering particle function '%s'", m_function);
    Particle.function(m_function, &LoggerFunction::receiveCall, this);

    // available commands variable
    if (m_var_available_commands != nullptr) {
        Log.info("registering particle variable '%s'", m_var_available_commands);
        String cmds_json = getCommands().toJSON();
        if (cmds_json.length() < particle::protocol::MAX_FUNCTION_ARG_LENGTH) {
            // commands fit into the particle variable
            snprintf(m_value_available_commands, particle::protocol::MAX_FUNCTION_ARG_LENGTH, "%s", cmds_json.c_str());
        } else {
            // commands don't fit
            Variant trunc;
            trunc.set("trunc", true);
            trunc.set("error", 
                String::format("commands are too long (%d chars) and don't fit into the size limit of a particle variable (%d)",
                    cmds_json.length(), particle::protocol::MAX_FUNCTION_ARG_LENGTH));
            snprintf(m_value_available_commands, particle::protocol::MAX_FUNCTION_ARG_LENGTH, "%s", trunc.toJSON().c_str());
        }
        Particle.variable(m_var_available_commands, m_value_available_commands);    
    }

    // last call variable
    if (m_var_last_calls != nullptr) {
        // starting value is just an empty json array since there are no commands yet
        snprintf(m_value_last_calls, particle::protocol::MAX_FUNCTION_ARG_LENGTH, "%s", "[]");
        Particle.variable(m_var_last_calls, m_value_last_calls);
    }
}

void LoggerFunction::registerCommand(const std::function<bool(Variant&)>& cb, const char* module, const char* cmd, 
            const Vector<String>& text_values, bool allow_numeric_values,
            const Vector<String>& numeric_units, bool value_optional) {
    Log.info("registering command '%s' for module '%s'", cmd, module);

    // check for issues
    size_t i = 0;
    bool overwrite = false;
    for (; i < m_commands.size(); ++i) {
        if (strcmp(module, m_commands[i].module) == 0 && strcmp(cmd, m_commands[i].cmd) == 0) {
            Log.warn("cmd (%s) already exists for this module (%s), overwriting  existing", cmd, module);
            overwrite = true;
            break;
        }
        if (strcmp(cmd, m_commands[i].module) == 0 || strcmp(module, m_commands[i].cmd) == 0 || strcmp(cmd, module) == 0) {
            // should this be here or elsewhere? not sure it's visible on logger startup
            Log.error("identically named module and command (%s) can cause confusion and is not permitted", cmd);
            return;
        }
    }

    // add/overwrite
    if (overwrite)
        m_commands[i].use = false; // flag for ignoring (=overwrite)
    m_commands.append({cb, module, cmd, text_values, allow_numeric_values, numeric_units, value_optional}); // add
}
        
int LoggerFunction::receiveCall (String call) {

    using namespace LoggerFunctionReturns;

    // store call and basic info in the Variant and then parse it
    // important: this is NOT a member variable on purpose because variants
    // that are modified lead to memory fragmentation
    Variant parsed;
    parsed.set("call", call.c_str());
    parsed.set("dt", Time.format(Time.now(), "%Y-%m-%d %H:%M:%S %Z"));
    parsed.set("lt", "cmd"); // log type
    size_t cmd_idx = parseCall(parsed); 

    // any issues? 
    if (cmd_idx == PARSING_ERROR) {

        // parsing error
        Log.trace("parsing error: %s", parsed.toJSON().c_str());
        parsed.set("success", false);

    }  else {

        // found a command while parsing, execute the callback
        Log.trace("execute callback with: %s", parsed.toJSON().c_str());
        bool success = m_commands[cmd_idx].callback(parsed);
        parsed.set("success", success);

        // if no ret val set yet
        if (success && !hasReturnValue(parsed)) {
            setSuccess(parsed);
        } else if (!success && !hasReturnValue(parsed)) {
            setReturnValue(parsed, CALL_ERR_UNKNOWN);
        } 
    }

    // report command to cloud if logging is on
    if (m_log) {
        // FIXME: implement
        // LoggerPublisher::queueData(parsed);
        // this will also print the published data like below instead of here
        Log.trace("after callback:");
        Log.print(parsed.toJSON().c_str());
        Log.print("\n");
    }

    // update last call variable?
    if (m_var_last_calls != nullptr) {
        
        // restore from JSON (stored in char to avoid memory fragmentation)
        Variant call_log = Variant::fromJSON(m_value_last_calls);

        // store the last call in the call log
        call_log.append(parsed);
        size_t call_log_size = call_log.toJSON().length();
        while (call_log_size  >= particle::protocol::MAX_FUNCTION_ARG_LENGTH && !call_log.isEmpty()) {
            // remove the oldest entries until they fit
            call_log.removeAt(0);
            call_log_size = call_log.toJSON().length();
        }

        // set call_log
        if (Log.isTraceEnabled()) {
            Log.trace("new value for Particle.variable('%s') from %d commands in call log stack", m_var_last_calls, call_log.size());
            Log.print(call_log.toJSON().c_str());
            Log.print("\n");
        }

        // assign call log
        snprintf(m_value_last_calls, particle::protocol::MAX_FUNCTION_ARG_LENGTH, "%s", call_log.toJSON().c_str());
    }

    // return return value
    return(getReturnValue(parsed));
}

size_t LoggerFunction::parseCall(Variant& parsed) {

    // logger function returns
    using namespace LoggerFunctionReturns;
    
    // get call back out
    String call = parsed.get("call").toString();
    if (call.length() == 0) {
        setReturnValue(parsed, CALL_ERR_EMPTY);
        return(PARSING_ERROR);
    }

    // make a mutable local copy for thread safety 
    // use a smart pointer for memory management
    auto copy = std::make_unique<char[]>(call.length() + 1);
    std::strcpy(copy.get(), call.c_str());

    // get module
    char *part = strtok(copy.get(), " ");

    // module or cmd exists?
    bool mod_found = false;
    size_t cmd_idx = 0;
    uint n_cmds_found = 0;
    for (size_t i = 0; i < m_commands.size(); ++i) {
        if (!m_commands[i].use) continue;
        if (strcmp(part, m_commands[i].module) == 0) {
            parsed.set("m", part);
            mod_found = true;
        }
        if (strcmp(part, m_commands[i].cmd) == 0) {
            Log.trace("cmd match: %s", part);
            parsed.set("c", part);
            n_cmds_found++;
            cmd_idx = i;
        }
    }

    // what was found?
    if (mod_found && n_cmds_found == 0) {
        // all good, found a module and now looking for a command
        part = strtok(nullptr, " ");
        if (part == nullptr) {
            // no command provided --> error
            setReturnValue(parsed, CALL_ERR_CMD_MISS);
            return(PARSING_ERROR);
        }
        for (size_t i = 0; i < m_commands.size(); ++i) {
            if (!m_commands[i].use) continue;
            if (strcmp(parsed.get("m").asString().c_str(), m_commands[i].module) == 0 && strcmp(part, m_commands[i].cmd) == 0) {
                // found the command
                Log.trace("cmd match: %s", part);
                parsed.set("c", part);
                n_cmds_found++;
                cmd_idx = i;
                break;
            }
        }
        if (n_cmds_found == 0) {
            // no command of those that are registered for the module fits
            setReturnValue(parsed, CALL_ERR_CMD_UNREC);
            return(PARSING_ERROR);
        }
    } else if (!mod_found && n_cmds_found == 1) {
        // all good, found a single command --> set the module accordingly
        parsed.set("m", m_commands[cmd_idx].module);
    } else if (!mod_found && n_cmds_found == 0) {
        // was neither a comand nor a module -> error
        setReturnValue(parsed, CALL_ERR_CMD_MOD_UNREC);
        return(PARSING_ERROR);
    } else if (!mod_found && n_cmds_found > 1) {
        // command is ambiguous (might be in multiple modules)
        setReturnValue(parsed, CALL_ERR_AMBIGUOUS);
        return(PARSING_ERROR);
    }

    // safety check to avoid segfaults // REMOVE?
    if (cmd_idx >= m_commands.size()) {
        Log.error("this should never happen, cmd_idx (%d) is too large, there are only %d commands", cmd_idx, m_commands.size());
        return(PARSING_ERROR);
    }

    // values expected?
    if (m_commands[cmd_idx].expect_value) {
        part = strtok(nullptr, " ");
        if (part == nullptr) {
            if (!m_commands[cmd_idx].value_optional) {
                // no value provided and value is not optional --> error
                setReturnValue(parsed, CALL_ERR_VAL_MISS);
                return(PARSING_ERROR);
            }
            // empty value but that's okay
            parsed.set("vtext", Variant());
        } else {
            // got a value!
            parsed.set("vtext", part);
            bool valid_value = false;
            
            // let's see if it matches any of the allowed text values
            if (m_commands[cmd_idx].text_values.size() > 0) {
                 for (size_t i = 0; i < m_commands[cmd_idx].text_values.size(); ++i) {
                    if (strcmp(part, m_commands[cmd_idx].text_values[i].c_str()) == 0) {
                        // found the value (already correctly assigned "vtext")
                        Log.trace("value match: %s", part);
                        valid_value = true;
                        break;
                    }
                }
                // nothing found and numeric values not allowed?
                if (!valid_value && !m_commands[cmd_idx].allow_numeric_values) {
                    // --> error
                    setReturnValue(parsed, CALL_ERR_VAL_UNREC);
                    return(PARSING_ERROR);
                }
            }

            // no match yet? let's see if it's a valid numeric value (if they're allowed)
            if (!valid_value && m_commands[cmd_idx].allow_numeric_values) {

                char* num_end = nullptr;

                // convert the initial numeric part to double
                double number = strtod(part, &num_end);

                if (num_end == part) {
                    // not a valid number
                    setReturnValue(parsed, CALL_ERR_VAL_NAN);
                    return(PARSING_ERROR);
                } else {
                    // yay number
                    parsed.set("vnum", number);
                    Log.trace("numeric value: %s", parsed.get("vnum").toString().c_str());
                }

                if (*num_end != '\0' && m_commands[cmd_idx].numeric_units.isEmpty()) {
                    // found units directly after the number but none were expected! --> error
                    parsed.set("u", num_end);
                    setReturnValue(parsed, CALL_ERR_UNIT_UNEXP);
                    return(PARSING_ERROR);
                }

                // check for units
                if (!m_commands[cmd_idx].numeric_units.isEmpty()) {
                    bool valid_units = false;
                    if (*num_end != '\0') {
                        // found units directly after the number
                        parsed.set("u", num_end);
                    } else {
                        // fetch next part
                        part = strtok(nullptr, " ");
                        if (part == nullptr) {
                            setReturnValue(parsed, CALL_ERR_UNIT_MISS);
                            return(PARSING_ERROR);
                        }
                        parsed.set("u", part);
                    }
                    // check if the units fit any of the expected
                    for (size_t i = 0; i < m_commands[cmd_idx].numeric_units.size(); ++i) {
                        if (strcmp(parsed.get("u").asString().c_str(), m_commands[cmd_idx].numeric_units[i].c_str()) == 0) {
                            // found the unit (already correctly assigned "u")
                            Log.trace("unit match: %s", parsed.get("u").asString().c_str());
                            valid_units = true;
                            break;
                        }
                    }
                    
                    // did we find valid units?
                    if (!valid_units) {
                        // no --> units not recognized
                        setReturnValue(parsed, CALL_ERR_UNIT_UNREC);
                        return(PARSING_ERROR);
                    }
                }
            }
        }
    }

    // check for params if function interprets them
    if (!m_params.isEmpty()) {
        bool found_param = false;
        size_t current_param = 0;
        String param_value;
        part = strtok(nullptr, " ");
        while (part != nullptr) {
            bool new_param = false;

            // starts with a 'param='?
            for (size_t i = 0; i < m_params.size(); ++i) {
                String prefix = m_params[i] + "=";
                if (strncmp(part, prefix.c_str(), prefix.length()) == 0) {
                    // found a param!
                    if (found_param) {
                        // store the previous param in the variant
                        Log.trace("param: %s='%s'", m_params[current_param].c_str(), param_value.c_str());
                        parsed.set(m_params[current_param].c_str(), param_value.c_str());
                    }
                    // start the new param
                    found_param = true;
                    new_param = true;
                    current_param = i;
                    param_value = part + prefix.length(); // start new valuew without the prefix
                    break;
                }
            }

            // append value to current param value
            if (found_param && !new_param) {
                param_value += " ";
                param_value += part;
            }

            // continue the search
            part = strtok(nullptr, " ");
        }

        // any param to wrap up?
        if (found_param) {
            // store the previous param in the variant
            Log.trace("param: %s='%s'", m_params[current_param].c_str(), param_value.c_str());
            parsed.set(m_params[current_param].c_str(), param_value.c_str());
        }
    }
    
    // parsing complete
    return(cmd_idx);
}
