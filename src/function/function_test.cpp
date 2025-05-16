#include "Particle.h"
#include "LoggerFunction.h"
#include "LoggerModule.h"

// enable system treading
#ifndef SYSTEM_VERSION_v620
SYSTEM_THREAD(ENABLED);
#endif

// log handler
SerialLogHandler logHandler(LOG_LEVEL_INFO);

// my component class
class MyModule : LoggerModule {
    
    public:

        MyModule(const char* name) : LoggerModule(name) {}

        void registerCommands(LoggerFunction* func) {
            func->registerCommand(std::bind(&MyModule::myCmd, this, _1), m_name, "hello");
            func->registerCommand(std::bind(&MyModule::myCmd2, this, _1), m_name, "whatup");
        }

        bool myCmd(Variant& call) {
            Log.info("'hello' triggered %s", call.toJSON().c_str());
            call.set("new", 42);
            return(true);
        }

        bool myCmd2(Variant& call) {
            Log.info("'whatup' triggered: %s", call.toJSON().c_str());
            call.set("new2", 3.234);
            return(false);
        }

};

// example
LoggerFunction* func = new LoggerFunction("test");
MyModule* mod = new MyModule("mod1");
String cmds;

// setup
void setup() {

    // register all commands
    mod->registerCommands(func);
    cmds = func->getCommands().toJSON();
    Particle.variable("commands", cmds);

    // start listening to function calls
    func->setup();
}

// loop
void loop() {
    
}