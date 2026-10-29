void makeSureWiFiConnected(bool setupMode) {
 
  if (setupMode) {
    WiFi.setHostname(HOSTNAME);
    WiFi.mode(WIFI_STA);

    #ifdef TURN_OFF_WIFI_PS
      // Turn WiFi power saving mode off. Must run after WiFi.mode(), before it the driver isn't started
      WiFi.setSleep(false);
    #endif

    #ifdef FORCE_WIFI_BSSID
      WiFi.begin(WIFI_SSID, WIFI_PASS, 0, WIFI_BSSID);
    #else
      WiFi.begin(WIFI_SSID, WIFI_PASS, 0);
    #endif 

    delay(3000);
  }
  
  if (WiFi.status() != WL_CONNECTED) 
  {

    #ifdef FORCE_WIFI_BSSID
      WiFi.begin(WIFI_SSID, WIFI_PASS, 0, WIFI_BSSID); 
    #else
      WiFi.begin(WIFI_SSID, WIFI_PASS, 0);
    #endif     

    int _retries = WIFI_MAX_RECONNECT_TRIES;

    while (WiFi.waitForConnectResult() != WL_CONNECTED && _retries > 0) 
    {
    
      #ifdef DEBUG
          Serial.print("WiFi Connection Failed! Retries left: ");
          Serial.println(_retries);
      #endif

      _retries--;
      delay(3000);
    }

    if (WiFi.status() != WL_CONNECTED) 
    {
      #ifdef DEBUG
        Serial.println("Connection Failed! Rebooting...");
      #endif
      ESP.restart();
    }
  }
  if (setupMode) 
  {
    debug ("WiFi Ready. IP: " + WiFi.localIP().toString() + ", RSSI: " + String(WiFi.RSSI()));
  }
}

// Called through the debug() macro, only when DEBUG is defined
void debugPrint(const String &msg) {
  Serial.println(msg);

#ifdef MQTT_DEBUGGING
  if (mqttClient.connected()) {
    sendMQTTMessage(MQTT_DEBUG_TOPIC, msg.c_str());
  }
#endif
}