#pragma once

#include "Particle.h"

// file helper to get at flash system usage
// dependencies.FileHelperRK=0.0.3
#include "FileHelperRK.h"

namespace LoggerPlatform {

    // wifi
    #if (PLATFORM_ID == PLATFORM_PHOTON || PLATFORM_ID == PLATFORM_ARGON || PLATFORM_ID == PLATFORM_P2)
    const bool wifi = true;
    #else
    const bool wifi = false;
    #endif
    const bool hasWifi() { return wifi; };

    // cellular
    #if (PLATFORM_ID == PLATFORM_BORON)
    const bool cellular = true;
    #else
    const bool cellular = false;
    #endif
    const bool hasCellular() { return cellular; };

    // byte size constants
    const size_t B = 1;
    const size_t KB = 1024;
    const size_t MB = 1024 * 1024;

    // flash memory (in bytes)
    #if (PLATFORM_ID == PLATFORM_ARGON || PLATFORM_ID == PLATFORM_P2 || PLATFORM_ID == PLATFORM_BORON)
    const size_t flash_size = 2 * MB;
    #else
    const size_t flash_size = 0;
    #endif
    const bool hasFlash() { return (flash_size > 0); };
    const float getTotalFlash(const size_t units = B);
    float getUsedFlash(const size_t units = B);
    float getFreeFlash(const size_t units = B);
    float getFreeFlashPercent();

    // flash sectors
    const size_t sector_size = 4 * KB;
    const size_t flash_sectors = flash_size / sector_size; // available sectors

    // random access memory / RAM (in bytes) that is available to the user application
    #if (PLATFORM_ID == PLATFORM_PHOTON)
    const size_t ram_size = 60 * KB; // approximate
    #elif (PLATFORM_ID == PLATFORM_ARGON || PLATFORM_ID == PLATFORM_BORON)
    const size_t ram_size = 80 * KB; // approximate
    #elif (PLATFORM_ID == PLATFORM_P2)
    const size_t ram_size = 3 * MB; // approximate
    #else
    const size_t ram_size = 0;
    #endif
    const float getTotalRAM(const size_t units = B);
    float getUsedRAM(const size_t units = B);
    float getFreeRAM(const size_t units = B);
    float getFreeRAMPercent();

    // info
    Variant getSystemStatus();

}

/*** implementation ***/

// flash functions
const float LoggerPlatform::getTotalFlash(const size_t units) { 
    return( (float) flash_size/units); 
}
float LoggerPlatform::getUsedFlash(const size_t units) { 
    if (!hasFlash()) return (0.0);
    FileHelperRK::Usage usage;
    usage.measure("/");
    return( (float) (usage.sectors * sector_size) / units);
}
float LoggerPlatform::getFreeFlash(const size_t units) { 
    if (!hasFlash()) return (0.0);
    return( (getTotalFlash() - getUsedFlash())/units); 
};
float LoggerPlatform::getFreeFlashPercent() { 
    if (!hasFlash()) return(NAN);
    return(100. * getFreeFlash() / getTotalFlash()); 
};

// RAM functions
const float LoggerPlatform::getTotalRAM(const size_t units) { 
    return( (float) ram_size/units); 
}
float LoggerPlatform::getUsedRAM(const size_t units) { 
    return((getTotalRAM() - getFreeRAM()) / units);  
}
float LoggerPlatform::getFreeRAM(const size_t units) { 
    return((float) System.freeMemory()/units); 
};
float LoggerPlatform::getFreeRAMPercent() { 
    return(100. * getFreeRAM() / getTotalRAM()); 
};

// system status
Variant LoggerPlatform::getSystemStatus() {
    Variant sys;
    sys.set("firmware", System.version());
    sys.set("id", System.deviceID());
    sys.set("wifi", hasWifi());
    sys.set("cell", hasCellular());
    Variant mem;
    mem.set("total", getTotalRAM());
    mem.set("used", getUsedRAM());
    sys.set("RAM", mem);
    Variant flash;
    flash.set("total", getTotalFlash());
    flash.set("used", getUsedFlash());
    sys.set("flash", flash);
    return(sys);
}