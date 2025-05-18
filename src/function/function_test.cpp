#include "Particle.h"
#include "LoggerFunction.h"
#include "LoggerFunctionReturns.h"
#include "LoggerModule.h"

// enable system treading
#ifndef SYSTEM_VERSION_v620
SYSTEM_THREAD(ENABLED);
#endif

// log handler
SerialLogHandler logHandler(LOG_LEVEL_TRACE);

namespace LoggerFunctionReturns {
    inline constexpr Error MY_WARNING  = {100, "my favorite warning"};
    inline constexpr Error MY_ERROR  = {-100, "my favorite error"};
}

// my component class
class MyModule : LoggerModule {
    
    public:

        MyModule(const char* name) : LoggerModule(name) {}

        // command 1
        void registerHelloCommand(LoggerFunction* func, const char* cmd = "hello") {
            func->registerCommand(this, &MyModule::hello, getName(), cmd);   
        }

        bool hello(Variant& call) {
            Log.info("'hello' triggered %s", call.toJSON().c_str());
            call.set("new", 42);
            return(true);
        }

        // command 2
        void registerWhatupCommand(LoggerFunction* func, const char* cmd = "whatup") {
            func->registerCommand(this, &MyModule::whatup, getName(), cmd);   
        }

        bool whatup(Variant& call) {
            Log.info("'whatup' triggered: %s", call.toJSON().c_str());
            call.set("new2", 3.234);
            setReturnValue(call, LoggerFunctionReturns::MY_ERROR);
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
    mod->registerHelloCommand(func);
    mod->registerWhatupCommand(func);
    mod->registerWhatupCommand(func, "WHATUP");

    // safe commands in the commands variable
    cmds = func->getCommands().toJSON();
    Particle.variable("commands", cmds);

    // start listening to function calls
    func->setup();
}

// loop
void loop() {
    
}