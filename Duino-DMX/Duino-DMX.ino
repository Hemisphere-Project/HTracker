
#include <LXESP32DMX.h>
#include "LXESP32DMX1.h"
#include <WiFi.h>

#define DMX_DIRECTION_INPUT_PIN 32
#define DMX_SERIAL_INPUT_PIN 16

#define DMX_SERIAL_OUTPUT_PIN 21

#define DMX_CTRL_ADDR 42


uint8_t ctrl_value = 0;
uint8_t dataChanged = 1;

bool listenToUSB = false;


void receiveCallback(int slots) {
  int MAX_DMX = min(slots, DMX_MAX_FRAME);
	if ( slots ) {
	    xSemaphoreTake( ESP32DMX.lxDataLock, portMAX_DELAY );

      // GET CTRL VALUE
  		if ( ctrl_value != ESP32DMX.getSlot(DMX_CTRL_ADDR) ) {
  			ctrl_value = ESP32DMX.getSlot(DMX_CTRL_ADDR);
  			dataChanged = 1;

        if (ctrl_value == 0) listenToUSB = false;
  		}

      // DMX THRU
      if (ctrl_value == 0) {
        xSemaphoreTake( ESP32DMX1.lxDataLock, portMAX_DELAY );
        for (int i=1; i<MAX_DMX; i++) ESP32DMX1.setSlot(i , ESP32DMX.getSlot(i));
        xSemaphoreGive( ESP32DMX1.lxDataLock );
      }
      
  		xSemaphoreGive( ESP32DMX.lxDataLock );
	}
}




/************************************************************************
	setup
*************************************************************************/
void setup() {
  WiFi.mode(WIFI_OFF);
  btStop();

  Serial.begin(115200);

  // DISARM RX2 (using gpio15)
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  delay(200);

  // SET INPUT RX2
  ESP32DMX.setDirectionPin(DMX_DIRECTION_INPUT_PIN);
  ESP32DMX.setDataReceivedCallback(receiveCallback);
  ESP32DMX.startInput(DMX_SERIAL_INPUT_PIN);

  // SET OUTPUT TX1
  pinMode(DMX_SERIAL_OUTPUT_PIN, OUTPUT);
  ESP32DMX1.startOutput(DMX_SERIAL_OUTPUT_PIN);
  
  // REARM RX2 (using gpio15)
  delay(200);
  pinMode(15, INPUT);

  // USB READ TASK
  xTaskCreate(
      usbRead, /* Function to implement the task */
      "usbRead", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      1,  /* Priority of the task */
      NULL  /* Task handle. */
      );

}


/************************************************************************
	main loop just idles
	vTaskDelay is called to prevent wdt timeout
*************************************************************************/

void loop() {
  
  // CTRL change
  if ( dataChanged ) 
  {
    dataChanged = 0;
    Serial.print(DMX_CTRL_ADDR);
    Serial.print(":");
    Serial.println(ctrl_value);

    // RESET dmxbuffer (and wait for USB dmx)
    if (ctrl_value > 0) {
      xSemaphoreTake( ESP32DMX1.lxDataLock, portMAX_DELAY );
      ESP32DMX1.clearSlots();
      listenToUSB = true;
      xSemaphoreGive( ESP32DMX1.lxDataLock ); 
    }
    
  } else {
    delay(5);
  }

  
}
