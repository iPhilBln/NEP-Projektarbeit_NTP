#pragma once

    #include <Arduino.h>
    #include <FS.h>                   //this needs to be first, or it all crashes and burns...
    #include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

    #ifdef ESP32
    #include <SPIFFS.h>
    #endif

    #include <Arduino.h>
    #include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

    #include "ntp_client.hpp"

    // global functions

    void        wifiBegin(void);
    void        wifiProcess(void);
    void        wifiCleanup(bool eraseSPIFF = true);
    int         wifiGetParamCounter(void);
    const char* wifiGetParamValue(uint8_t paramNum);