#include <Arduino.h>
#include <Preferences.h>
#include <LXESP32DMX.h>
#include <FastLED.h>


#define HVERSION 0.1  // dmx node


// Common settings
//
#define SET_MODE    1     // 0:RECV / 1:SEND
#define SET_ADDRIN  1     // DMX input ADDR
int MODE;
int ADDRIN;

// LIGHT
//
const int NLEDS = 1;
CRGB leds[NLEDS];
void setLeds(CRGB color) 
{
  for (int k=0; k<NLEDS; k++) leds[k] = color;
  FastLED.show();
}

// RECV stuffs
//
uint8_t valueIN = 0;
bool valueChanged = false;

void receiveCallback(int slots) {
	if ( slots ) {
	  xSemaphoreTake( ESP32DMX.lxDataLock, portMAX_DELAY );
		if ( valueIN != ESP32DMX.getSlot(ADDRIN) ) {
			valueIN = ESP32DMX.getSlot(ADDRIN);
			valueChanged = true;
		}
		xSemaphoreGive( ESP32DMX.lxDataLock );
	}
}

// SEND stuffs
//
uint8_t dmxbuffer[DMX_MAX_FRAME];


void setup() 
{
  Serial.begin(115200);

  // LED
  //
  FastLED.addLeds<WS2812, 27, GRB>(leds, NLEDS);
  FastLED.setBrightness(100);
  setLeds(CRGB::Yellow);

  // Preferences
  //
  Preferences preferences;
  preferences.begin("HDMX", false);

  #ifdef SET_MODE
    preferences.putUInt("mode", SET_MODE);
  #endif
  MODE = preferences.getUInt("mode", 0);

  #ifdef SET_ADDRIN
    preferences.putUInt("addrin", SET_ADDRIN);
  #endif
  ADDRIN = preferences.getUInt("addrin", 0);

  preferences.end();


  // MODE RECV
  if (MODE == 0) 
  {
    ESP32DMX.setDataReceivedCallback(receiveCallback);
    pinMode(26, INPUT);
    ESP32DMX.startInput(26);
    setLeds(CRGB::Green);
    Serial.println("DMX->Serial");
  }


  // MODE SEND
  else if (MODE == 1) 
  {
    pinMode(32, OUTPUT);
    ESP32DMX.startOutput(32);
    setLeds(CRGB::Red);
    Serial.println("Serial->DMX");
  }
  
}


void loop() 
{

  // MODE RECV: DMX->Serial
  if (MODE == 0) 
  {
    if (valueChanged) 
    {
      Serial.println(valueIN);
      valueChanged = false;
      setLeds(CRGB::Lime);
    }
    else delay(5);
  }


  // MODE SEND: Serial->DMX
  else if (MODE == 1) 
  {
    // SERIAL READ
    //
    if(!Serial.available()) {
      delay(1);
      return;
    };
    char buffer[DMX_MAX_FRAME+10];
    int size = Serial.readBytesUntil('\n', buffer, DMX_MAX_FRAME+10);
    
    // DMX FRAME
    //
    if (size == DMX_MAX_FRAME) 
    {
      xSemaphoreTake( ESP32DMX.lxDataLock, portMAX_DELAY );
      for (int i=1; i<DMX_MAX_FRAME; i++)
          ESP32DMX.setSlot(i , buffer[i]);
      xSemaphoreGive( ESP32DMX.lxDataLock );
      setLeds(CRGB::Pink);
    }
    
  }

  // Serial.println(ESP.getFreeHeap());
}