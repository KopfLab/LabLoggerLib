#include "Particle.h"
#include "LoggerFunctionReturns.h"

bool LoggerFunctionReturns::hasReturnValue(Variant &call) {
    return(call.has("ret"));
}

int LoggerFunctionReturns::getReturnValue(Variant &call) {
    // return copy of return value
    return(call.get("ret").toInt());
}

void LoggerFunctionReturns::setReturnValue(Variant& call, int code, const char* message, bool overwrite) {
    
    if (call.has("ret") && call.get("ret").asInt() != LoggerFunctionReturns::CMD_SUCCESS) {
        // call already has a return value that is NOT the marker for success
        if (overwrite) {
            // something other than success is getting overwritten
            Log.warn("overwriting existing return value %d with new value %d = %s", call.get("ret").asInt(), code, message);
        } else {
            // keeping existing  value (overwrite is false)
            Log.warn("keeping existing return value (%d) and discarding new value %d = %s", call.get("ret").asInt(), code, message);
            return;
        }
    }
    call.set("ret", code);
    call.set("msg", message);
}

void LoggerFunctionReturns::setReturnValue(Variant& call, LoggerFunctionReturns::Warning warn) {
    setReturnValue(call, warn.code, warn.message, false);
}

void LoggerFunctionReturns::setReturnValue(Variant& call, LoggerFunctionReturns::Error err) {
    setReturnValue(call, err.code, err.message, true);
}

void LoggerFunctionReturns::setSuccess(Variant& call) {
    if (!hasReturnValue(call)) {
        call.set("ret", LoggerFunctionReturns::CMD_SUCCESS);
    } else if (call.get("ret").asInt() != LoggerFunctionReturns::CMD_SUCCESS) {
        // call already has a state set
        Log.warn("could not set call to success (%d), already has a different return code (%d)", LoggerFunctionReturns::CMD_SUCCESS, call.get("ret").asInt());
    }
}
