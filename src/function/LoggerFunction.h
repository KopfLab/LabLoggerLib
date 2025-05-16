#pragma once
#include "Particle.h"

class LoggerFunction {

    private:

        // name of the function that's registered with Particle.function
        const char* m_function;

    protected:

        struct Command {
            std::function<bool(Variant&)> callback;
            const char* module;
            const char* cmd;
        };
        Variant commandToVariant(Command cmd) {
            Variant var;
            var.set("m", cmd.module);
            var.set("c", cmd.cmd);
            return(var);
        }

        std::vector<Command> m_commands;


    public:

        LoggerFunction(const char* function) : m_function(function) {}

        void setup() {
            Log.info("registering particle function '%s'", m_function);
            Particle.function(m_function, &LoggerFunction::receiveCall, this);
        }

        // register cloud command
        void registerCommand(const std::function<bool(Variant&)>& cb, const char* module, const char* cmd) {
            Log.info("registering command '%s' for module '%s'", cmd, module);
            m_commands.push_back({cb, module, cmd});
        }

        Variant getCommands() {
            Variant cmds;
            for (auto& cmd : m_commands) {
                cmds.append(commandToVariant(cmd));
            }
            return(cmds);
        }

        // receive cloud command
        int receiveCall (String call) {
            Log.info("received: %s", call.c_str());
            Variant parsed = parseCall(call);
            Log.info("parsed: %s", parsed.toJSON().c_str());

            bool success = false;
            // run all callbacks
            //for (auto& cmd : m_commands) {
            //    success = cmd.callback(parsed);
            //}
            // FIXME: only call the one that was identified as the correct one
            // FIXME: provide some sort of static method in LoggerFunction::error and LoggerFunction.warning
            // that register parset.set("ret") and parsed.set("err") / parsed.set("warn")
            // find a good way to register the error codes, probably with a namespace and structure so everything always has a text AND a retval!!

            // register success or fail
            parsed.set("success", success);

            // if no ret val set yet
            if (success && !parsed.has("ret")) {
                parsed.set("ret", 0L); // FIXME: use placeholder for success
            } else if (!success && !parsed.has("ret")) {
                parsed.set("ret", -1L);
                // FIXME: use placeholders for these codes
                //LoggerFunction::error(ERROR_UNKNOWN); // registers the unknown error and the return value
                //LoggerFunction::warn(WARN_BLA); // errors overwrite all warnings, all warnings are reported if there are multiple (but no error) - Q: how to deal with return codes?
            } 

            // check
            Log.info("after callbacks: %s", parsed.toJSON().c_str());
            
            // report command to cloud / FIXME implement
            // add dt: timestamp
            // add t: type = cmd
            // LoggerPublisher::queueData(parsed);

            // return function
            return(parsed.get("ret").asInt());
        }

        Variant parseCall(String& call) {
            // started the parsed call variant
            Variant parsed;
            parsed.set("call", call.c_str());
            
            // make a mutable local copy for thread safety
            char *copy = strdup(call.c_str());

            // empty call?
            if (copy == nullptr) {
                parsed.set("err", "call is empty");
                return(parsed);
            }

            // get module
            char *part = strtok(copy, " ");
            parsed.set("m", part);

            // module exists?
            bool mod_found = false;
            for (auto& cmd : m_commands) {
                if (strcmp(part, cmd.cmd) == 0) {
                    Log.trace("module match: %s", cmd.cmd);
                    mod_found = true;
                    break;
                }
            }

            if (!mod_found) {
                parsed.set("err", "module not recognized");
                return(parsed);
            }

            // command exists?
            // FIXME: check all commands that match the module and the command
            // FIXME: pull out params user= and note= 
            // FIXME: in database, store ret as return_code, store err / warn in return_value
            // continue here

            //if (!part) {
            //    parsed.set("error", "");
            //}
            // return the parsed command
            return(parsed);
        }
};