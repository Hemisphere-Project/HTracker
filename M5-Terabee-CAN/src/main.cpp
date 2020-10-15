#include <Arduino.h>
#include <M5Stack.h>
#include <M5ez.h>
#include "teraranger.h"
#include "can.h"

#define HID 37

const char* title =  "HTracker v0.1 - TERABEE";

#define KEEPALIVE_TIME 500

uint32_t last_send = 0;
uint16_t last_value = 0;

void setup() 
{
  Serial.begin(115200);

  #include <themes/dark.h>
  ez.begin();
  M5.Speaker.mute();

  ez.screen.clear();
  ez.header.show(title);
  ez.canvas.lmargin(10);
  ez.canvas.font(&FreeSans9pt7b);
  ez.canvas.println("");
  ez.canvas.println("Hello");
  
  // Starting TeraRanger I2C
  ez.canvas.print("[INIT ] Starting TeraBee I2C.. ");
  if (tera_setup()) ez.canvas.println("OK!");
  else ez.canvas.println("ERROR");

  // Starting CAN interface
  ez.canvas.print("[INIT ] Starting CAN Bus.. ");
  if (can_setup()) ez.canvas.println("OK!");
  else ez.canvas.println("ERROR");

  // Ready
  ez.canvas.println("[READY] Starting data emission.. ");
  
}


void loop() 
{
  uint16_t current_value = tera_read();

  bool trig = (current_value/30 != last_value/30) || (millis() - last_send > KEEPALIVE_TIME);
  if (trig) 
  {
    CanMessage* msg = new CanMessage(HID, MEASURE, current_value);

    can_send(msg);

    last_value = current_value;
    last_send = millis();

    ez.msgBox(title, String(msg->value()/1000.)+" m", "", false);
  }

}