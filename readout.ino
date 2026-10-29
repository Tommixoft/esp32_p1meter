/**
  setupDataReadout()

  This method reads JSON string specified in DsmrMap variable in dsmr_map.h file
    and converts to telegramDecodedObjects MQTT vector.
*/
void setupDataReadout()
{

  JsonDocument _dsmrMapDocument;
  DeserializationError _error = deserializeJson(_dsmrMapDocument, DSMR_MAP);
  if (_error)
  {
    debug("deserializeJson() failed: ");
    debug(_error.c_str());
    return;
  }

  // Create TelegramObject vector which will be used to send MQTT messages
  for (JsonObject _item : _dsmrMapDocument.as<JsonArray>())
  {
    TelegramDecodedObject _tdo;
    strlcpy(_tdo.code, _item["OBIS"] | "", sizeof(_tdo.code));
    _tdo.name = _item["Name"].as<String>();

    telegramObjects.push_back(_tdo);
  }

#ifdef DEBUG
  debug("MQTT Topics initialized (" + String(telegramObjects.size()) + "):");

  for (int i = 0; i < telegramObjects.size(); i++)
  {
    debug(String(MQTT_ROOT_TOPIC) + "/" + telegramObjects[i].name + ". OBIS: " + String(telegramObjects[i].code));
  }

#endif
}
