#pragma once
#include "Particle.h"
#include "LoggerFunctionReturns.h"

/**
 * extension of return codes
 */
namespace LoggerFunctionReturns {
    inline constexpr Error CMD_ERR_UNKNOWN   = {-1, "undefined error"};
    inline constexpr Error CMD_ERR_EMPTY     = {-2, "call is empty"};
    inline constexpr Error CMD_ERR_AMBIGUOUS = {-3, "command ambiguous (exists in multiple modules), specify module"};
    inline constexpr Error CMD_ERR_CMD_MOD   = {-4, "module/command not recognized"};
    inline constexpr Error CMD_ERR_CMD_MISS  = {-5, "module found but no command provided"};
    inline constexpr Error CMD_ERR_CMD       = {-6, "module found but command not recognized"};
}

/**
 * class that manages a function call for the logger
 */
class LoggerFunction {

    private:

        // name of the function that's registered with Particle.function
        const char* m_function;
        const size_t PARSING_ERROR = std::numeric_limits<size_t>::max();

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

        void setup();

        /**
         * @brief register a cloud command with an instance and method
         */
        template <typename T>
        void registerCommand(T* instance, bool (T::*method)(Variant&), const char* module, const char* cmd) {
            std::function<bool(Variant&)> cb = std::bind(method, instance, _1);
            registerCommand(cb, module, cmd);
        }

        /**
         * @brief register a cloud command directly with a std:function call
         */
        void registerCommand(const std::function<bool(Variant&)>& cb, const char* module, const char* cmd);

        /**
         * @brief get back a Variant with all the commands
         */
        Variant getCommands();

        /**
         * @brief internal function that's registered with the Particle cloud to process user commands
         */
        int receiveCall (String call);

        /**
         * @brief parse the function call and store the results in the parsed Variant
         * @return the m_commands index of the command that fits the call (or PARSED_ERROR if parsing error)
         */
        size_t parseCall(Variant& parsed);

};