#include "Particle.h"
#include "LoggerPublisher.h"

bool LoggerPublisher::publish(const Variant &data) {
    return(true);
    // if (!event.isNew()) {
    //     // an event is already being sent --> need to queue it
    // }

    // // set up event
    // m_event.name(m_event_name);
    // m_event.data(data);

    // // can we publish?
    // if (!CloudEvent::canPublish(m_event.size())) {
    //     return(false);
    // }

    // _log.trace("publishing fileNum=%d event=%s", curFileNum, curEvent.name());

    // if (!Particle.publish(m_event)) {
    //     Log.error("published failed immediately, discarding");
    //     deleteCurEvent();
    //     stateHandler = &PublishQueueExt::stateWaitEvent;
    //     durationMs = waitBetweenPublish;
    //     return;
    // }


    // //return publish(event);
    // return publish(m_event);
}

void LoggerPublisher::queueData(const Variant &data) {
    m_burst_ongoing = true;
    m_last_burst_data = millis();
    m_burst_data.append(data);
}

void LoggerPublisher::queueBurst() {
    Variant burst;
    if (DeviceNameHelperEEPROM::instance().hasName()) {
        // device name is available
        burst.set("id", DeviceNameHelperEEPROM::instance().getName());
    } else {
        // no device name available -- use device ID instead
        burst.set("id", System.deviceID());
    }
    burst.set("b", m_burst_data);
    Log.trace("adding burst to queue: %s", burst.toJSON().c_str());
    m_data_queue.append(burst);
    m_burst_data.clear();

    // sd backup
    if (m_use_sd_backup) {
        Log.trace("backing up burst on SD card in file %s", m_sd_log_file.c_str());
        if (m_sd->available()) {
            m_sd->append(m_sd_log_file.c_str());
            m_sd->println(burst.toJSON().c_str());
            m_sd->syncFile();
        } else {
            Log.error("SD card unavailable, burst could not be backed up");
        }
    }

// } else if (System.freeMemory() < memory_reserve) {
//     out_of_memory = true;
//     Serial.printlnf("WARNING: state log '%s' NOT queued because free memory < memory reserve (%d bytes).",  state_log, memory_reserve);
//     saveStateLogToSD(); // check for SD save even if we're out of RAM
//   } else {
}

// setup and loop

void LoggerPublisher::setup() {
    if(m_use_sd_backup) {
        Log.info("starting logger (with SD backup)");
        m_sd->init();
    } else {
        Log.info("starting logger (without SD backup)");
    }
    m_sd_log_file = String::format("device_%s.log", System.deviceID().c_str());
}

void LoggerPublisher::loop() {

    // check for end of a data burst
    if (m_burst_ongoing && (millis() - m_last_burst_data) > m_wait_for_burst_data) {
        queueBurst();
        m_burst_ongoing = false;
    }

    // check on publish state
    switch(m_publish_state) {

        // waiting to connect
        case State::WAIT_CONNECT:
            if (Particle.connected()) {
                // connected!
                m_state_time = millis();
                m_publish_state = State::WAIT_PUBLISH;
            } else if ( !m_data_queue.isEmpty()) {
                // we've got data but no connection --> store on disc

            }
            break;

        // waiting for event
        case State::WAIT_PUBLISH:
            if (!Particle.connected()) {
                // disconnected!
                m_publish_state = State::WAIT_CONNECT;
            } else if ( !m_data_queue.isEmpty() && 
                (millis() - m_state_time) > m_wait_after_connect) {
                // we've got data and a stable connection
                
            }
            break;


        // case State::START:
        //     if (Particle.connected() && ((lastPublish == 0) || (millis() - lastPublish >= publishPeriod.count()))) {
        //         lastPublish = millis();
        //         state = State::SEND;
        //     }
        //     break;

        
            

        // case State::SEND:
        //     state = State::START;            
        //     if (publishSensors()) {
        //         state = State::WAIT_COMPLETION;
        //     }
        //     break;

        // case State::WAIT_COMPLETION:
        //     if (event.isSent()) {
        //         Log.info("publish succeeded");
        //         state = State::START;            
        //     }
        //     else 
        //     if (!event.isOk()) {
        //         Log.info("publish failed error=%d", event.error());
        //         state = State::START;            
        //     }
        //     break;        
    }
    

    // // nothing to do if event is new or still sending
    // if (m_event.isNew() || m_event.isSending()) {
    //     return;
    // }

    // if (event.isSent()) {
    //     Log.info("publish succeeded");
    //     stateHandler = &SensorPublish::start;
    //     return;
    // }
    // if (!event.isOk()) {
    //     Log.info("publish failed error=%d", event.error());
    //     stateHandler = &SensorPublish::start;
    //     return;
    // }

    // if (!m_event.isValid()) {
    //     _log.trace("publish failed invalid %d (discarding)", curFileNum);
    //     m_event.clear();
    // } else if (m_event.isSent()) {
    //     _log.trace("publish success %d", curFileNum);
    //     m_event.clear();
    // }
    // else {
    //     _log.trace("publish failed %d (retrying)", curFileNum);
    //     curFileNum = 0;
    //     durationMs = waitAfterFailure;
    // }

    // if (curEvent.isSending()) {
    //     // Stay in statePublishWait
    //     return;
    // }
}

// sd functions
void LoggerPublisher::useSdBackup(bool use) { 
    if (!m_use_sd_backup && use)
        Log.info("logger turning on SD backup");
    else if (m_use_sd_backup && !use)
        Log.info("logger turning off SD backup");
    m_use_sd_backup = use; 
};

bool LoggerPublisher::testSD() {
    Log.info("running SD write/read test");

    // check enabled
    if (!m_use_sd_backup) {
        Log.error("cannot test SD, logger is not using SD backup");
        return(false);
    }

    // check available
    if (!m_sd->available()) {
        Log.error("cannot test SD, SD card is not available");
        return(false);
    }

    // remove previous test file if still there
    long file_size = m_sd->size("sd_test.txt");
    if (file_size > -1) {
      Log.info("deleting test file from previous test");
      m_sd->removeFile("sd_test.txt");

      // check if it's been deleted
      if (m_sd->size("sd_test.txt") > -1) {
        Log.error("could not remove previous test file from SD card");
        return(false);
      }
    }
   
    // write test file
    Log.info("writing test file sd_test.txt");
    m_sd->append("sd_test.txt");
    m_sd->print("test");
    m_sd->syncFile();
      
    // now should have test file
    if (m_sd->size("sd_test.txt") == -1) {
        Log.error("could not write test file to SD card");
        return(false);
    } 

    // read back test file
    Log.info("reading back test file sd_test.txt");
    byte buffer[4] = {0, 0, 0, 0};
    m_sd->read(buffer, 4, "sd_test.txt");
    if (buffer[0] != 't' || buffer[1] != 'e' || buffer[2] != 's' || buffer[3] != 't')  {
        Log.error("could not confirm correct test file contents");
        return(false);
    } 
    
    Log.info("successfully recovered file content, SD card test complete");
    return(true);
}