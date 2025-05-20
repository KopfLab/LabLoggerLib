/**
 * test for LoggerFunction class
 * to use:
 *  - flash to device of joice
 *  - either call any commands directly with particle call DEVICE test "test4"
 *  - or start the set of auto-tests by calling particle call DEVICE test "auto_test"
 */

#include "Particle.h"
#include "LoggerFunction.h"
#include "LoggerFunctionReturns.h"
#include "LoggerModule.h"

// enable system treading
#ifndef SYSTEM_VERSION_v620
SYSTEM_THREAD(ENABLED);
#endif

// log handler
SerialLogHandler logHandler(LOG_LEVEL_INFO, { // Logging level for non-application messages
    { "app", LOG_LEVEL_TRACE } // Logging level for application messages (i.e. debug mode)
});

// custom return value examples
namespace LoggerFunctionReturns {
    inline constexpr Error MY_WARNING  = {100, "my favorite warning"};
    inline constexpr Error MY_ERROR  = {-100, "my favorite error"};
}

// custom component class examples
class MyModule : public LoggerModule {
    
    public:

        bool auto_test_running = false;

        MyModule(const char* name) : LoggerModule(name) {}

        // 'autotest'
        bool auto_test(Variant& call) {
            Log.info("starting auto test suite of commands");
            auto_test_running = true;
            return(true);
        }

        // 'hello'
        void registerHelloCommand(LoggerFunction* func, const char* cmd = "hello") {
            func->registerCommand(this, &MyModule::hello, getName(), cmd);   
        }

        bool hello(Variant& call) {
            Log.info("'hello' triggered %s", call.toJSON().c_str());
            setReturnValue(call, LoggerFunctionReturns::MY_WARNING);
            return(true);
        }

        // 'whatup'
        void registerWhatupCommand(LoggerFunction* func, const char* cmd = "whatup") {
            func->registerCommand(this, &MyModule::whatup, getName(), cmd);   
        }

        bool whatup(Variant& call) {
            Log.info("'whatup' triggered: %s", call.toJSON().c_str());
            setReturnValue(call, LoggerFunctionReturns::MY_ERROR);
            return(false);
        }

        // 'test'
        bool test(Variant& call) {
            Log.info("'test' with: %s", call.toJSON().c_str());
            return(true);
        }

};

// example classes
LoggerFunction* func = new LoggerFunction(
    "test",             // name of the Particle.function
    {"user", "note"},   // parameters (param=) interpreted by the call
    true,               // whether to log each command using the LoggerPublisher
    "commands",         // name of the Particle.variable where all available commands are stored (as JSON)
    "last_calls"        // name of the Particle.variable where the last received function calls are stored (as JSON)
);


MyModule* mod = new MyModule("mod1");
String available_cmds;
String last_cmd;

// setup
void setup() {

    // register a suite of test commands
    // start auto-test
    func->registerCommand(mod, &MyModule::auto_test, mod->getName(), "auto-test");

    // register all commands defined in the module class (usually all of them defined there)
    mod->registerHelloCommand(func);
    mod->registerWhatupCommand(func);
    mod->registerWhatupCommand(func, "WHATUP");

    // simple command
    func->registerCommand(mod, &MyModule::test, mod->getName(), "test1");

    // command that accepts on/off values
    func->registerCommandWithTextValues(mod, &MyModule::test, mod->getName(), "test2", {LoggerFunction::on, LoggerFunction::off});

    // command that accepts a/b/2 values but providing a value is optional (last param)
    func->registerCommandWithTextValues(mod, &MyModule::test, mod->getName(), "test3", {"a", "b", "2"}, true);

    // command that accepts numeric values (no units)
    func->registerCommandWithNumericValues(mod, &MyModule::test, mod->getName(), "test4");

    // command that accepts numeric values with specific units, providing the value is optional (last param)
    func->registerCommandWithNumericValues(mod, &MyModule::test, mod->getName(), "test5", {"sec", "min"}, true);

    // command that accepts mixed values with a few specific text values OR numeric values with specific units
    func->registerCommandWithMixedValues(mod, &MyModule::test, mod->getName(), "test6", {"manual"}, {"ms", "sec"});

    // start listening to function calls
    func->setup();
}

// testing commands
const Vector<String> calls {
    "non-existent-cmd",
    "whatup", "WHATUP",
    "mod-dne hello", "mod1 hello note=whatever is up with=that user=test user",
    "test1",
    "test2", "test2 on", "test2 blib", "test2 off extra user=test",
    "test3", "test3 2", "test3 2 kg", "test3 b note=hello #3",
    "test4", "test4 x", "test4 1kg", "test4 -2.352",
    "test5", "test5 y", "test5 4.2", "test5 -42what", "test5 1.3e3 myunit", "test5 -1sec user=test", "test5 24.1 min note=hello",
    "test6", "test6 manual", "test6 dne", "test6 42", "test6 -4.2ms"
};

unsigned long last_call = 0;
size_t call_i = 0;
const std::chrono::milliseconds wait = 2s;

// loop
void loop() {

    // mimic commands via the test calls
    if (mod->auto_test_running && millis() - last_call > wait.count()) {
        if (call_i >= calls.size()) call_i = 0;
        Log.print("\n");
        uint32_t mem_before = System.freeMemory();
        Log.info("CALL #%d (free mem: %.3f KB): '%s'", call_i, (float) System.freeMemory() / 1024., calls[call_i].c_str());
        func->receiveCall(String(calls[call_i]));
        uint32_t mem_after = System.freeMemory();
        Log.info("FREE MEM loss: %d B", mem_before - mem_after);
        Log.print("\n");
        call_i++;
        last_call = millis();
    }

}

