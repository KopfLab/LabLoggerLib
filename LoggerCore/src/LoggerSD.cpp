#include "Particle.h"
#include "LoggerSD.h"

void LoggerSD::init() {
    present = false;   
    Log.trace("checking for SD card reader at I2C address 0x%02X", i2c_address);
    Wire.begin();
    Wire.beginTransmission(i2c_address);
    present = (Wire.endTransmission() == 0);
    if (present) {
        Log.info("SD card reader found at I2C address 0x%02X.", i2c_address);
        Log.trace("initializing SD card");    
        present = OpenLog::begin(i2c_address);
        if (present) {
            Log.info("SD card initialized successfully.");
        } else {
            Log.warn("cannot use SD card, card failed to initialize (card missing?)");
        }
    } else {
        Log.warn("cannot use SD card, no SD card reader found at I2C address 0x%02X.", i2c_address);
    }
}

bool LoggerSD::available() {
    if (!present) init();
    return(present);
}

bool LoggerSD::syncFile() {
    // reset present if card available but sync fails
    if (available()) present = OpenLog::syncFile();
    if (!present) Log.error("writing to SD card failed");
    return(present);
}