void usbRead( void * parameter) {

  byte incomingByte = 0;
  int numericByte = 0;
  int key = -1;
  int addr = 0;
  int value = 0;
  bool validated = false;
  
  while(true) {

    // READ
    if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();

      // newline: reset
      if (incomingByte == '\n') {
        if (key == 1) {
          validated = true;
          key = -1;
        }
        else {
          addr = 0;
          value = 0;
          key = 0;
        }
        
      }

      // addr to value 
      else if (incomingByte == ':') {
        key += 1;
      }

      // increment addr
      else if (key == 0) {
        numericByte = (incomingByte-'0');
        if (numericByte >-1 && numericByte < 10) addr = addr*10+numericByte;
      }

      // increment value
      else if (key == 1) {
        numericByte = (incomingByte-'0');
        if (numericByte >-1 && numericByte < 10) value = value*10+numericByte;
      }

      // DATA push
      if (validated) {
        validated = false;

        if (addr >= 1 && addr <= DMX_MAX_FRAME && value >= 0 && value <= 255) {
          xSemaphoreTake( ESP32DMX1.lxDataLock, portMAX_DELAY );
          ESP32DMX1.setSlot(addr, value);
          xSemaphoreGive( ESP32DMX1.lxDataLock );
          //Serial.printf("%d => %d\n", addr, value);
        }
        else Serial.printf("ERROR: %d => %d\n", addr, value);
        
        key = 0;
        addr = 0;
        value = 0;
      }
    }
    else delay(1);
  }

  vTaskDelete( NULL );
}
