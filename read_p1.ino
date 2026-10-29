// CRC state of the telegram being read, lines before the first header are ignored
bool telegramStarted = false;
unsigned int currentCRC = 0;

// CRC16 (polynomial 0xA001) as used by DSMR, computed over everything from '/' to '!' inclusive
unsigned int crc16(unsigned int crc, unsigned char *buf, int len) {
  for (int pos = 0; pos < len; pos++) {
    crc ^= (unsigned int)buf[pos];

    for (int i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

short getValueType(char * res, int len) {
  if (len > 10) {
    return -1; // treat longer than possible max int 2147483647 as strings
  }
  short dotsFound = 0;
  for (int i = 0; i < len; i++) {
    if (((res[i] < '0') || (res[i] > '9')) && (res[i] != '.' && res[i] != 0)) {
      return -1; // non-numeric
    }
    if (res[i] == '.') {
      dotsFound++;
    }
  }
  if (dotsFound > 1) {
    return -1; // more than 1 dot means something weird, not number for sure
  }
  return dotsFound; // 1 - float/double, 0 - integer
}

int findCharInArrayRev(char array[], char c, int len) {
  for (int i = len - 1; i >= 0; i--) {
    if (array[i] == c) {
      return i;
    }
  }

  return -1;
}

String getValue(char * buffer, int maxlen, char startchar, char endchar) {
  int s = findCharInArrayRev(buffer, startchar, maxlen - 2);
  int l = findCharInArrayRev(buffer, endchar, maxlen - 2) - s - 1;

  char res[128];

  if (s < 0 || l < 0 || l >= (int) sizeof(res)) // start or end char not found, or value too long
    return String(NA);

  memcpy(res, buffer + s + 1, l);
  res[l] = 0;

  short numType = getValueType(res, l);
  if (numType == 1) // float or double
    return String((endchar == VALUE_END_CHAR ? VALUE_NUMERIC_MULTIPLIER : 1) * atof(res), VALUE_FLOAT_DECIMAL_PLACES);
  else if (numType == 0) // integer
    return String((endchar == VALUE_END_CHAR ? VALUE_NUMERIC_MULTIPLIER : 1) * atoi(res));
  else
    return String(res);
}

/**
 *  Checks the CRC on the end line ('!') and commits the staged values when it matches.
 *  The 4 hex digits after '!' are the CRC; '!' itself is still part of the checksum.
 */
bool endTelegram() {
  currentCRC = crc16(currentCRC, (unsigned char *) telegram, 1);

  char messageCRC[5];
  strncpy(messageCRC, telegram + 1, 4);
  messageCRC[4] = 0;

  char *crcEnd;
  long receivedCRC = strtol(messageCRC, &crcEnd, 16);
  bool validCRC = crcEnd != messageCRC && receivedCRC == currentCRC;

  debug(String(validCRC ? "CRC valid" : "CRC invalid") + ", calculated " + String(currentCRC, HEX) + ", received " + String(messageCRC));

  for (int i = 0; i < telegramObjects.size(); i++) {
    // sendData stays set until published, so a change is never dropped
    if (validCRC && telegramObjects[i].pendingValue.length() > 0 && telegramObjects[i].value != telegramObjects[i].pendingValue) {
      telegramObjects[i].value = telegramObjects[i].pendingValue;
      telegramObjects[i].sendData = true;
    }

    telegramObjects[i].pendingValue = "";
  }

  return validCRC;
}

/**
 *  Decodes the telegram PER line. Not the complete message.
 *  Values are staged per line and only committed when the telegram CRC is valid.
 *  Returns true on the end line ('!') of a telegram with a valid CRC.
 */
bool decodeTelegram(int len) {
  debug(telegram);

  if (telegram[0] == '/') {
    // Header starts a new telegram, values staged from an incomplete one are dropped
    for (int i = 0; i < telegramObjects.size(); i++) {
      telegramObjects[i].pendingValue = "";
    }

    telegramStarted = true;
    currentCRC = crc16(0x0000, (unsigned char *) telegram, len);
    return false;
  }

  if (!telegramStarted) {
    return false;
  }

  if (telegram[0] == '!') {
    telegramStarted = false;
    return endTelegram();
  }

  currentCRC = crc16(currentCRC, (unsigned char *) telegram, len);

  // Loops throug all the telegramObjects to find the code in the telegram line
  // If it finds the code the value is staged, so it can be sent to the mqtt broker once the CRC is valid
  for (int i = 0; i < telegramObjects.size(); i++) {
    if (strncmp(telegram, telegramObjects[i].code, strlen(telegramObjects[i].code)) == 0) {
      String newValue = getValue(telegram, len, VALUE_START_CHAR, VALUE_END_CHAR);

      if (newValue == NA) {
        newValue = getValue(telegram, len, VALUE_START_CHAR, VALUE_NO_UNITS_END_CHAR);
      }

      telegramObjects[i].pendingValue = newValue;

      debug(telegramObjects[i].name + " = " + newValue);
      break;
    }
  }

  return false;
}

bool readP1Serial() {
  while (Serial2.available() > 0) {
    // Reads the telegram until it finds a return character
    // That is after each line in the telegram. 2 bytes are kept for '\n' and terminator
    int len = Serial2.readBytesUntil('\n', telegram, P1_MAXLINELENGTH - 2);

    telegram[len] = '\n';
    telegram[len + 1] = 0;

    // End of a telegram with a valid CRC, all values are read
    if (decodeTelegram(len + 1)) {
      return true;
    }
  }

  return false;
}
