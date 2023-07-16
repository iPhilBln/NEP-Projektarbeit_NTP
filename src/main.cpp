#include <Arduino.h>
#include "wifi.hpp"
#include "ntp_client.hpp"

#define button 18

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true); 
  delay(1000);

  pinMode(button, INPUT);

  if (digitalRead(button) == LOW) {
    delay(500);
    wifiCleanup();
  }

  wifiBegin();
}

unsigned long timestamp = 0;

void loop() {
  wifiProcess();
/*
  if (millis() - timestamp > 10 * 1000) {
    timestamp = millis();
    int memberCounter = NTPClient::getClientsCount();
    for(NTPClient& client : NTPClient::masters) {
        client.getTimestamp();
    }
    Serial.println();
  }

*/
  if (digitalRead(button) == LOW) {
    delay(500);
    ESP.restart();
  }
}
