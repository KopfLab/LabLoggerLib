#pragma once

#include "Particle.h"

/**
 * @brief namespace that defines return codes for LoggerFunctions
 * expand this namespace in other classes to register additional return codes
 */
namespace LoggerFunctionReturns {

    struct Error {
        const int code;
        const char* message;
        constexpr Error(int c, const char* msg) : code(c), message(msg) {}
    };

    struct Warning {
        const int code;
        const char* message;
        constexpr Warning(int c, const char* msg) : code(c), message(msg) {}
    };

    const int CMD_SUCCESS =  0;

     /**
     * @brief check if the call has a return value set
     */
    bool hasReturnValue(Variant &call);

    /**
     * @brief get the return value from the call Variant as an integer
     */
    int getReturnValue(Variant &call);

    /**
     * @brief set the return value from code and message
     * (usually not called directly but via void setReturnValue(Variant& call, Warning warn) andvoid setReturnValue(Variant& call, Error err))
     */
    void setReturnValue(Variant& call, int code, const char* message, bool overwrite);

    /**
     * @brief set the return value to a Warning
     */
    void setReturnValue(Variant& call, Warning warn);

    /**
     * @brief set the return value to an Error
     */
    void setReturnValue(Variant& call, Error err);

    /**
     * @brief set the return value to success
     */
    void setSuccess(Variant& call);

}

