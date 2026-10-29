#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <tiny-collections.h>
#include "settings.h"
#include "dsmr_map.h"

// debug() compiles to nothing without DEBUG, so its String arguments are never built
#ifdef DEBUG
  #define debug(msg) debugPrint(msg)
#else
  #define debug(msg)
#endif

WiFiClient espClient;
PubSubClient mqttClient(espClient);

/***********************************
            Main Setup
 ***********************************/
void setup() {
  // Force CPU frequency if constant is specified
  if (CPU_FREQ > 0) {
    setCpuFrequencyMhz(CPU_FREQ);
  }

  Serial.begin(BAUD_RATE);

  #ifdef DEBUG
  debug("Booting - DEBUG mode on");
  #endif

  makeSureWiFiConnected(true);

  MDNS.begin(HOSTNAME);
  delay(1000);

  setupOTA();

  mqttClient.setServer(MQTT_HOST, atoi(MQTT_PORT));
  makeSureMqttConnected();

  setupDataReadout();

  // RX buffer size must be set before begin(), otherwise it stays at the 256 byte default
  Serial2.setRxBufferSize(1700);
  Serial2.begin(BAUD_RATE, SERIAL_8N1, RXD2, TXD2, true);

  debug("System initialised successfully!");
}

/***********************************
            Main Loop
 ***********************************/
void loop() {
  unsigned long now = millis();

  if (REBOOT_INTERVAL > 0 && now >= REBOOT_INTERVAL) {
    ESP.restart();
  }

  makeSureWiFiConnected(false);
  ArduinoOTA.handle();
  makeSureMqttConnected();
  mqttClient.loop();

  // Metrics are not retained, so unchanged values are republished on this interval
  if (now - LAST_FULL_UPDATE_SENT > UPDATE_FULL_INTERVAL) {
    LAST_FULL_UPDATE_SENT = now;
    markAllForSending();
  }

  // Read every telegram so the UART buffer never holds stale data, publish changes on interval
  if (readP1Serial() && now - LAST_UPDATE_SENT > UPDATE_INTERVAL) {
    LAST_UPDATE_SENT = now;
    sendDataToBroker();
  }
}

/***********************************
            Setup Methods
 ***********************************/
/**
   Over the Air update setup
*/
void setupOTA() {
  ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR)
        Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR)
        Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR)
        Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR)
        Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR)
        Serial.println("End Failed");
    });
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPasswordHash(OTA_PASSWORD_HASH);
  ArduinoOTA.begin();
}