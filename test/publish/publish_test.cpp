#include "Particle.h"

// Using the extended cloud publish requires >=6.3.0
#ifndef SYSTEM_VERSION_630
#error "This test requires Device OS 6.3.0 or later"
#endif

// use with retained name
#include "DeviceNameHelperRK.h"

// include file helper to get at posix system usage
#include "FileHelperRK.h"

// include queue publisher
#include "PublishQueueExtRK.h"
#include "LoggerUtils.h"
#include "LoggerPlatform.h"
#include "LoggerPublisher.h"

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(
  LOG_LEVEL_TRACE, { // Logging level for non-application messages
	{ "comm", LOG_LEVEL_INFO },
	{ "system", LOG_LEVEL_INFO }
});

unsigned long lastRun = 0;

void runPlatformStats();

// out of memory handler //
int outOfMemory = -1;

void outOfMemoryHandler(system_event_t event, int param) {
    outOfMemory = param;
}

bool nameRequested = false;

// // Open a serial terminal and see the IP address printed out
// void subscriptionHandler(const char *topic, const char *data)
// {
//     Log.info("topic=%s data=%s", topic, data);
// }

LoggerPublisher *publisher = new LoggerPublisher();

void setup() {
    // Enabling an out of memory handler is a good safety tip. If we run out of
    // memory a System.reset() is done.
    System.on(out_of_memory, outOfMemoryHandler);

    // For testing purposes, wait 10 seconds before continuing to allow serial to connect
	// before doing PublishQueue setup so the debug log messages can be read.
	waitFor(Serial.isConnected, 10000);
    delay(1000);

    // This allows a graceful shutdown on System.reset()
    Particle.setDisconnectOptions(CloudDisconnectOptions().graceful(true).timeout(5000));

    // publish queue
	PublishQueueExt::instance().setup();
    PublishQueueExt::instance().withFileQueueSize(100);

    // device name storage (check name once per restart to make sure it's still the same)
    DeviceNameHelperEEPROM::instance().setup(0);
    DeviceNameHelperEEPROM::instance().checkName();

    // from: https://build.particle.io/libs/PublishQueueExtRK/0.0.6/tab/example/2-test-suite.cpp
    publisher->setup();

    // TODO:
    // - implmement a LoggerPlatform static class that holds the info about flash size, memory size, wifi/cellular bools, etc. and is set basaed on platform_ID
    // - implmenet the Event publish as an extension of the PublishQueueExtRK class (check this function https://github.com/rickkas7/PublishQueueExtRK/blob/c6f147c5099abdf6120360acfa4833a04ea9136e/src/PublishQueueExtRK.cpp#L431)
    // - except if publishing the event fails --> write to disc OR if the connection is offline --> write to disc
    // - consider implementing an additional RAM queue that is used AFTER flash space is used up (depending on what number events are stored), 
    // something along these lines using up the RAM queue to keep processing (gets processed first when internet connectivity resumes):
    // https://github.com/rickkas7/PublishQueuePosixRK/blob/075dfeb6f57511af62f796da29200b455fdbf2b8/src/PublishQueuePosixRK.cpp#L82
}
// out of memory handler //


int counter = 0;
const std::chrono::milliseconds publishPeriod = 5s;

void loop() {

    // keep device name updated
    DeviceNameHelperEEPROM::instance().loop();

    // out of memory handler //
    if (outOfMemory >= 0) {
        // An out of memory condition occurred - reset device.
        Log.info("out of memory occurred size=%d", outOfMemory);
        delay(100);
        System.reset();
    } 
    // out of memory handler //


    if (millis() - lastRun > publishPeriod.count()) {

        lastRun = millis();
        runPlatformStats();

        Variant obj;
        obj.set("a", counter++);
        obj.set("b", 1.32);
        publisher->queueData(obj);
    }

    publisher->loop();

    // approach
}

void runPlatformStats() {

    using namespace LoggerPlatform;
    using LoggerUtils::checkNaN;

    // CloudEvent event;
    // Variant var;
    // var.set("a", 2.3435);
    // var.set("b", 5);
    // var.set("c", true);
    // var.set("d", "bla");
    // var.set("e", Variant());
    // event.name("test");
    // event.data(var);
    // Particle.publish(event);

    Variant sys = getSystemStatus();
    sys.remove("id"); // don't need to display this
    sys.set("% RAM", checkNaN(getFreeRAMPercent())); // add this
    sys.set("% flash", checkNaN(getFreeFlashPercent())); // add this
    Log.info("system status");
    Log.print(sys.toJSON().c_str()); Log.print("\n"); // full dump
}
