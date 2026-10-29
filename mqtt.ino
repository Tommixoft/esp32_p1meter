bool sendMQTTMessage(const char *topic, const char *payload) {
  return mqttClient.publish(topic, payload, false); // metrics are not retained
}

void makeSureMqttConnected() {
  uint _retries = MQTT_MAX_RECONNECT_TRIES;

  while (!mqttClient.connected() && _retries > 0) {
    if (mqttClient.connect(HOSTNAME, MQTT_USER, MQTT_PASS, MQTT_STATUS_TOPIC, 2, true, "offline")) {
      mqttClient.publish(MQTT_STATUS_TOPIC, "online", true);
      // The broker may have restarted and lost every value, so send them all again
      markAllForSending();
    }
    else {
      debug("MQTT Connection Failed! Retries left: " + String(_retries));
      _retries--;
      delay(5000);
    }
  }

  if (!mqttClient.connected()) {
    debug ("Connection to MQTT Failed!");
#ifdef MQTT_RESTART_ON_ERROR
    debug ("Rebooting...");
    ESP.restart();
#endif
  }
}

bool sendMetric(const String &name, const String &metric) {
  String topic = String(MQTT_ROOT_TOPIC) + "/" + name;
  debug(topic);
  return sendMQTTMessage(topic.c_str(), metric.c_str());
}

void markAllForSending() {
  for (int i = 0; i < telegramObjects.size(); i++)
  {
    telegramObjects[i].sendData = true;
  }
}

void sendDataToBroker() {
  for (int i = 0; i < telegramObjects.size(); i++)
  {
    // Values not read from the meter yet are skipped, an empty payload would show up as a blank sensor
    if (telegramObjects[i].value.length() == 0)
    {
      continue;
    }

    // Keep the flag when publishing fails, so the value is retried with the next telegram
    if (telegramObjects[i].sendData && sendMetric(telegramObjects[i].name, telegramObjects[i].value))
    {
      debug("Sent: " + telegramObjects[i].name + " value: " + telegramObjects[i].value);
      telegramObjects[i].sendData = false;
    }
  }
}
