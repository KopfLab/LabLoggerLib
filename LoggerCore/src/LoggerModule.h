#pragma once
#include "Particle.h"
#include "LoggerFunction.h"
#include "LoggerFunctionReturns.h"

// module class
class LoggerModule {
    
    protected:

        // name of the module
        const char* m_name;

    public:

        LoggerModule(const char* name) : m_name(name) {}

        const char* getName() {
            return(m_name);
        }

        /**
         * @brief call this while parsing a command to indicate a warning to the user that does not fail the command
         */
        void setReturnValue(Variant& call, LoggerFunctionReturns::Warning warn) {
            LoggerFunctionReturns::setReturnValue(call, warn);
        }

        /**
         * @brief call this while parsing a command to indicate an error to the user that fails the command
         */
        void setReturnValue(Variant& call, LoggerFunctionReturns::Error err) {
            LoggerFunctionReturns::setReturnValue(call, err);
        }

};
