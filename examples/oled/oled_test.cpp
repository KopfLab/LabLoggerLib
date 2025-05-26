#include "Particle.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

// enable system treading
#ifndef SYSTEM_VERSION_v620
SYSTEM_THREAD(ENABLED);
#endif

// log handler
SerialLogHandler logHandler(LOG_LEVEL_INFO);

// Use I2C with OLED RESET pin
#define OLED_RESET D10 // D10 on photon2, D8 on argon/boron
Adafruit_SSD1306 display(OLED_RESET);

void setup() {
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
    display.display(); // show splashscreen
    display.setTextSize(1);
    display.setTextColor(WHITE);
}

unsigned long last_run = 0;
int counter = 0;
const std::chrono::milliseconds wait = 5s;

void loop() {
    if (millis() - last_run > wait.count()) {
        last_run = millis();
        Log.info("hello %d", counter);

        display.setCursor(0, 0);
        display.clearDisplay();
        display.printf("hello %d...", counter);
        display.display();

        counter++;
    }
}