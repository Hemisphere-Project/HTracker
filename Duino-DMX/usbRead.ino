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

      // newline: EXECUTE 
      if (incomingByte == '\n') {

        if (key == 2) {
          ctrlADDR(value);
        }
        
        if (key == 1) {
          if (listenToUSB) 
          {    
            // 0:0 => Clear all
            if (addr == 0 && value == 0) {
              xSemaphoreTake( ESP32DMX1.lxDataLock, portMAX_DELAY );
              ESP32DMX1.clearSlots();
              xSemaphoreGive( ESP32DMX1.lxDataLock );
            }
    
            // x:y => Set addr x at value y 
            else if (addr >= 1 && addr <= DMX_MAX_FRAME && value >= 0 && value <= 255) 
            {
              xSemaphoreTake( ESP32DMX1.lxDataLock, portMAX_DELAY );
              ESP32DMX1.setSlot(addr, value);
              xSemaphoreGive( ESP32DMX1.lxDataLock );
              //Serial.printf("#DMX set %d => %d\n", addr, value);
            }
            else Serial.printf("#ERROR: %d => %d\n", addr, value);
          }
        }

        addr = 0;
        value = 0;
        key = 0;
      }

      // REBOOT
      else if (incomingByte == 'x') {
        ESP.restart();
        while(true) delay(10000);
      }

      // set CTRL addr
      else if (incomingByte == '-') {
        key = 2;
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

      // increment CTRL addr
      else if (key == 2) {
        numericByte = (incomingByte-'0');
        if (numericByte >-1 && numericByte < 10) value = value*10+numericByte;
      }

      
    }
    else delay(1);
  }

  vTaskDelete( NULL );
}
