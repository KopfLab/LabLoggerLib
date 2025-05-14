#pragma once

#include "Particle.h"
#include "LoggerSD.h"

// device name logger
// dependencies.DeviceNameHelperRK=0.0.1
#include "DeviceNameHelperRK.h"

/**
 * @brief collects data for publication into bursts and then manages burst publication, backup (if SD available), 
 * and temporary storage in flash(if available) and RAM during internet disconnects 
 */
class LoggerPublisher {

    protected:

        // publishing event
        const char *m_event_name; // name of the event the logger is for
        CloudEvent m_event; // event used for publishing

        // SD card backup
        LoggerSD* m_sd = new LoggerSD();
        bool m_use_sd_backup; // whether to backup data on external SD card
        String m_sd_log_file;

        // data bursts
        VariantArray m_burst_data; // stack of data from a burst
        unsigned long m_last_burst_data = 0; // millis() when last data arrived
        const uint m_wait_for_burst_data; // ms to wait for more burst data to arrive
        bool m_burst_ongoing = false; // flag for when we're in a data burst

        // memory queue for publishing
        Variant m_data_queue; // stack of data from collection of bursts
        const uint m_RAM_reserve; // memory reserve in bytes
        void queueBurst(); // internal method to move a burst into the queue

        // state machine
        enum struct State {
            WAIT_CONNECT,
            WAIT_PUBLISH,
            START,
            SEND,
            WAIT_COMPLETION,
        };
        State m_publish_state = State::WAIT_CONNECT;

        // state time & constants
        unsigned long m_state_time = 0; // millis() when entering state
        const unsigned long m_wait_after_connect = 500; // ms to wait after Particle.connected() before publishing
        const unsigned long m_wait_after_failure = 30000; // ms to wait after a failed publish before trying again

        /// data queue
        // FIXME: calculate m_data size by adding it to cloudevent?

        

    public:

        LoggerPublisher() : LoggerPublisher(
            "publish-test", // event name 
            true,           // use_sd_backup
            500,            // wait_for_burst_data (in ms) --> default: 500 ms
            10 * 1024       // RAM_reserve (in bytes) --> default: 10 kb
        ) {};

        LoggerPublisher(const char *event_name, const bool use_sd_backup, const uint wait_for_burst_data, const uint RAM_reserve) : 
            m_event_name(event_name), m_use_sd_backup(use_sd_backup), m_wait_for_burst_data(wait_for_burst_data), m_RAM_reserve(RAM_reserve) {};

        bool publish(const Variant &data);
        
        void queueData(const Variant &data);
        bool hasData() { return(m_burst_data.isEmpty()); };

        int getQueueSize() { return(m_burst_data.size()); };

        /**
         * @brief must be callsed from the global setup
         */
        void setup();

        /**
         * @brief must be called from the global loop
         */
        void loop();
        
        // SD backup functions

        /**
         * @brief decide whether to use SD backup
         */
        void useSdBackup(bool use);

        /**
         * @brief method to test SD reading/writing capabilities
         */
        bool testSD();
};