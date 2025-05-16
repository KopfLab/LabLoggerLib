#pragma once
#include "Particle.h"
#include "LoggerFunction.h"

// module class
class LoggerModule {
    
    protected:

        // name of the module
        const char* m_name;

    public:

        LoggerModule(const char* name) : m_name(name) {}

        virtual void registerCommands(LoggerFunction* func);

};
