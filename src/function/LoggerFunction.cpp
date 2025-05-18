#include "Particle.h"
#include "LoggerFunction.h"
#include "LoggerFunctionReturns.h"

void LoggerFunction::setup() {
    Log.info("registering particle function '%s'", m_function);
    Particle.function(m_function, &LoggerFunction::receiveCall, this);
}

void LoggerFunction::registerCommand(const std::function<bool(Variant&)>& cb, const char* module, const char* cmd) {
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
    if (!overwrite) {
        m_commands.push_back({cb, module, cmd}); // add
    } else {
        m_commands[i] = {cb, module, cmd}; // overwrite
    }
}

Variant LoggerFunction::getCommands() {
    Variant cmds;
    for (auto& cmd : m_commands) {
        cmds.append(commandToVariant(cmd));
    }
    return(cmds);
}

        
int LoggerFunction::receiveCall (String call) {

    using namespace LoggerFunctionReturns;
    
    Log.trace("received: %s", call.c_str());

    // store call and basic info in parsed Variant and send it off to parse
    Variant parsed;
    parsed.set("call", call.c_str());
    parsed.set("dt", Time.format(Time.now(), "%Y-%m-%d %H:%M:%S %Z"));
    parsed.set("lt", "cmd"); // log type
    size_t cmd_idx = parseCall(parsed); 

    // any issues? 
    if (cmd_idx == PARSING_ERROR) {
        Log.trace("parsing error: %s", parsed.toJSON().c_str());
        parsed.set("success", false);
        return(getReturnValue(parsed));
    } 

    // found a command while parsing, execute the callback
    Log.trace("call parsed: %s", parsed.toJSON().c_str());
    bool success = m_commands[cmd_idx].callback(parsed);
    parsed.set("success", success);

    // if no ret val set yet
    if (success && !hasReturnValue(parsed)) {
        setSuccess(parsed);
    } else if (!success && !hasReturnValue(parsed)) {
        setReturnValue(parsed, CMD_ERR_UNKNOWN);
    } 
    
    // debug check
    Log.trace("after callback: %s", parsed.toJSON().c_str());

    // report command to cloud / FIXME implement
    // LoggerPublisher::queueData(parsed);

    // return function
    return(getReturnValue(parsed));
}

size_t LoggerFunction::parseCall(Variant& parsed) {

    // logger function returns
    using namespace LoggerFunctionReturns;
    
    // make a mutable local copy for thread safety
    char *copy = strdup(parsed.get("call").toString().c_str());

    // empty call?
    if (copy == nullptr) {
        setReturnValue(parsed, CMD_ERR_EMPTY);
        return(PARSING_ERROR);
    }

    // get module
    char *part = strtok(copy, " ");

    // module or cmd exists?
    bool mod_found = false;
    size_t cmd_idx = 0;
    uint n_cmds_found = 0;
    for (size_t i = 0; i < m_commands.size(); ++i) {
        if (strcmp(part, m_commands[i].module) == 0) {
            Log.trace("module match: %s", part);
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
            setReturnValue(parsed, CMD_ERR_CMD_MISS);
            return(PARSING_ERROR);
        }
        for (size_t i = 0; i < m_commands.size(); ++i) {
            Log.trace("mod: %s = %s?, cmd: %s = %s?", parsed.get("m").asString().c_str(), m_commands[i].module, part, m_commands[i].cmd);
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
            setReturnValue(parsed, CMD_ERR_CMD);
            return(PARSING_ERROR);
        }
    } else if (!mod_found && n_cmds_found == 1) {
        // all good, found a single command --> set the module accordingly
        parsed.set("m", m_commands[cmd_idx].module);
    } else if (!mod_found && n_cmds_found == 0) {
        // was neither a comand nor a module -> error
        setReturnValue(parsed, CMD_ERR_CMD_MOD);
        return(PARSING_ERROR);
    } else if (!mod_found && n_cmds_found > 1) {
        // command is ambiguous (might be in multiple modules)
        setReturnValue(parsed, CMD_ERR_AMBIGUOUS);
        return(PARSING_ERROR);
    }

    // so far so good, continue parsing 
    // FIXME: check all commands that match the module and the command
    // FIXME: pull out params user= and note= 
    // FIXME: in database, store ret as return_code, store err / warn in return_value
    // continue here

    //if (!part) {
    //    parsed.set("error", "");
    //}
    // succesfully parsed
    return(cmd_idx);
}
