#pragma once
#include "Particle.h"
#include "LoggerFunctionReturns.h"
#include "LoggerModule.h"

/**
 * extension of return codes
 */
namespace LoggerFunctionReturns {
    inline constexpr Error CALL_ERR_UNKNOWN       = { -1, "undefined error"};
    inline constexpr Error CALL_ERR_EMPTY         = { -2, "call is empty"};
    inline constexpr Error CALL_ERR_AMBIGUOUS     = { -3, "command ambiguous (exists in multiple modules), specify module"};
    inline constexpr Error CALL_ERR_CMD_MOD_UNREC = { -4, "module/command not recognized"};
    inline constexpr Error CALL_ERR_CMD_MISS      = { -5, "module found but no command provided"};
    inline constexpr Error CALL_ERR_CMD_UNREC     = { -6, "module found but command not recognized"};
    inline constexpr Error CALL_ERR_VAL_MISS      = { -7, "value required but none provided"};
    inline constexpr Error CALL_ERR_VAL_NAN       = { -8, "value is not a valid number"};
    inline constexpr Error CALL_ERR_VAL_UNREC     = { -9, "value not recognized"};
    inline constexpr Error CALL_ERR_UNIT_UNEXP    = {-11, "unit after number value but no unit was expected"};
    inline constexpr Error CALL_ERR_UNIT_MISS     = {-10, "unit required but none provided"};
    inline constexpr Error CALL_ERR_UNIT_UNREC    = {-12, "unit not recognized"};
}

/**
 * class that manages a function call for the logger
 */
class LoggerFunction {

    protected:

        // name of the function and variables that are registered with Particle.function/variable
        const char* m_function;
        const char* m_var_available_commands;
        char m_value_available_commands[particle::protocol::MAX_FUNCTION_ARG_LENGTH];
        const char* m_var_last_calls;
        char m_value_last_calls[particle::protocol::MAX_FUNCTION_ARG_LENGTH];

        // call parameters (xyz=, abc=) to interpret/capture
        const Vector<String> m_params;

        // whether to log received calls with LoggerPublisher
        bool m_log;

        // return value indicating a parsing error
        const size_t PARSING_ERROR = std::numeric_limits<size_t>::max();

        // command object for registering commands
        struct Command {
            std::function<bool(Variant&)> callback;
            const char* module;
            const char* cmd;
            const Vector<String> text_values = {};// if specific text values are allowed (can be fixed number values too)
            bool allow_numeric_values = false;
            const Vector<String> numeric_units = {}; // if allow_numeric = true and the value should have units
            bool value_optional = false; // whether providing a value is required or optional
            bool expect_value = true; // if either text_values are provided or numeric_values are allowed
            bool use = true; // flag when command is deactivated for some reason

            Command(std::function<bool(Variant&)> callback, const char* module, const char* cmd, 
                const Vector<String>& text_values, bool allow_numeric_values, 
                const Vector<String>& numeric_units, bool value_optional) : 
                callback(callback), module(module), cmd(cmd), text_values(text_values), allow_numeric_values(allow_numeric_values), numeric_units(numeric_units), 
                value_optional(value_optional), expect_value(allow_numeric_values || text_values.size() > 0), use(true) {}

            /**
             * generate a variant with the command, this is in an optimized JSON format with
             * variables only included when necessary and true/false represented as 1/0
             */
            Variant toVariant();
        };

        // vector of commands        
        // this is a non-const Variant but since it is not modified during runtime (only during startup)
        // it is not a memory problem
        Vector<Command> m_commands;

        // register a full cloud command with a std:function call, used by other registerCommmand... calls
        void registerCommand(const std::function<bool(Variant&)>& cb, const char* module, const char* cmd, 
            const Vector<String>& text_values, bool allow_numeric_values,
            const Vector<String>& numeric_units, bool value_optional);

        // parses the function call
        // returns the m_commands index of the command that fits the call (or PARSED_ERROR if parsing error)
        size_t parseCall(Variant& parsed);

    public:

        // common text values used a lot
        inline static const char* on = "on";
        inline static const char* off = "off";

        // default constructor that's used for lablogger devices
        // copy into your class to make these explicit for your device
        // don't change the string constants for compatibility with the LabLogger framwork
        LoggerFunction() : 
            LoggerFunction(
                "device",           // name of the Particle.function call
                {"user", "note"},   // parameters (param=) interpreted by the call
                true,               // whether to log each command using the LoggerPublisher
                "commands",         // name of the Particle.variable where all available commands are stored (as JSON)
                "last_calls"        // name of the Particle.variable where the last received function calls are stored (as JSON)
            )
         {}

        // constructor with user defined parameters
        LoggerFunction(const char* function, const Vector<String>& params, bool log, const char* var_available_commands, const char* var_last_calls) : m_function(function), m_var_available_commands(var_available_commands), m_var_last_calls(var_last_calls), m_params(params), m_log(log) {}

        /**
         * @brief must be called at the end of setup() to register the cloud function+variables and start listening to commands - note that any registerCommand that is called AFTER setup is not included in the available commands
         */
        void setup();

        /**
         * @brief register a simple cloud command without any value additions
         * usually called during setup
         */
        template <typename T>
        // defined here instead of in cpp for full flexibility
        void registerCommand(T* instance, bool (T::*method)(Variant&), const char* cmd) {
            std::function<bool(Variant&)> cb = [instance, method](Variant& v) {
                return (instance->*method)(v);
            };
            const Vector<String> empty = {};
            if constexpr (std::is_convertible_v<T*, LoggerModule*>) {
                LoggerModule* m = static_cast<LoggerModule*>(instance); 
                registerCommand(cb, m->getName(), cmd, empty, false, empty, false);
            } else {
                registerCommand(cb, "", cmd, empty, false, empty, false);
            }
        }

        /**
         * @brief register a cloud command with a list of specific text values allowed (value required by default)
         * usually called during setup
         */
        template <typename T>
        // defined here instead of in cpp for full flexibility
        void registerCommandWithTextValues(T* instance, bool (T::*method)(Variant&), const char* cmd, const Vector<String>& text_values, bool value_optional = false) {
            std::function<bool(Variant&)> cb = [instance, method](Variant& v) {
                return (instance->*method)(v);
            };
            const Vector<String> empty = {};
            if constexpr (std::is_convertible_v<T*, LoggerModule*>) {
                LoggerModule* m = static_cast<LoggerModule*>(instance); 
                registerCommand(cb, m->getName(), cmd, text_values, false, empty, value_optional);
            } else {
                registerCommand(cb, "", cmd, text_values, false, empty, value_optional);
            }
        }

        /**
         * @brief register a cloud command with numeric values and optionally defined units and value required by default
         * usually called during setup
         */
        template <typename T>
        // defined here instead of in cpp for full flexibility
        void registerCommandWithNumericValues(T* instance, bool (T::*method)(Variant&), const char* cmd, const Vector<String>& numeric_units = {}, bool value_optional = false) {
            std::function<bool(Variant&)> cb = [instance, method](Variant& v) {
                return (instance->*method)(v);
            };
            const Vector<String> empty = {};
            if constexpr (std::is_convertible_v<T*, LoggerModule*>) {
                LoggerModule* m = static_cast<LoggerModule*>(instance); 
                registerCommand(cb, m->getName(), cmd, empty, true, numeric_units, value_optional);
            } else {
                registerCommand(cb, "", cmd, empty, true, numeric_units, value_optional);
            }
        }

         /**
         * @brief register a cloud command with mixed test and numeric values, optionally defined units, and value required by default
         * usually called during setup
         */
        template <typename T>
        // defined here instead of in cpp for full flexibility
        void registerCommandWithMixedValues(T* instance, bool (T::*method)(Variant&), const char* cmd, const Vector<String>& text_values, const Vector<String>& numeric_units = {}, bool value_optional = false) {
            std::function<bool(Variant&)> cb = [instance, method](Variant& v) {
                return (instance->*method)(v);
            };
            if constexpr (std::is_convertible_v<T*, LoggerModule*>) {
                LoggerModule* m = static_cast<LoggerModule*>(instance); 
                registerCommand(cb, m->getName(), cmd, text_values, true, numeric_units, value_optional);
            } else {
                registerCommand(cb, "", cmd, text_values, true, numeric_units, value_optional);
            }
        }

        /**
         * @brief get back a Variant with all the commands 
         * this information is stored in a Particle.variable() if var_available_commands is set
         * it is optimized for minimal JSON (e.g. true/false = 1/0) to accomodate as many commands as possible
         */
        Variant getCommands();

        /**
         * @brief internal function that's registered with the Particle cloud to process user commands
         * can be called directly for testing purposes
         */
        int receiveCall (String call);

};