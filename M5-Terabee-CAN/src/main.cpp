#include <Arduino.h>
#include <M5Stack.h>
#include <M5ez.h>
#include "teraranger.h"
#include "can.h"

#define REPEAT_TIME 500

uint32_t last_send = 0;
uint16_t last_value = 0;

void setup() 
{
  Serial.begin(115200);

  #include <themes/dark.h>
  ez.begin();
  ez.screen.clear();
  ez.header.show("H-Ranger Terabee CAN");

  ez.canvas.println();
  ez.canvas.println("Hello!");
  
  // Starting TeraRanger I2C
  ez.canvas.print("Starting TeraBee I2C.. ");
  if (tera_setup()) ez.canvas.println("OK!");
  else ez.canvas.println("ERROR");

  // Starting CAN interface
  ez.canvas.print("Starting CAN Bus.. ");
  if (can_setup()) ez.canvas.println("OK!");
  else ez.canvas.println("ERROR");
}


void loop() 
{
  uint16_t current_value = tera_read();

  bool trig = (current_value/20 != last_value/20) || (millis() - last_send > REPEAT_TIME);
  if (trig) 
  {
    canMsg msg = {MEASURE, 0x100, 2, {
        (current_value >> 8) & 0xff, 
        current_value & 0xff
      }};

    can_send(msg);
    Serial.println(current_value);
    last_value = current_value;
    last_send = millis();
  }

}