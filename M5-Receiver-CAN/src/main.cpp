#include <Arduino.h>
#include <M5Stack.h>
#include <M5ez.h>
#include <can.h>

canMsg message;

void setup() 
{
  Serial.begin(115200);

  #include <themes/dark.h>
  ez.begin();
  ez.screen.clear();
  ez.header.show("H-Ranger Receiver CAN");

  ez.canvas.println();
  ez.canvas.println("Hello!");
  
  // Starting CAN interface
  ez.canvas.print("Starting CAN Bus.. ");
  if (can_setup()) ez.canvas.println("OK!");
  else ez.canvas.println("ERROR");
}


void loop() 
{
  message = can_read();
  if (message.code == NIET) return;
  
  Serial.print(message.code);
  Serial.print(" "); 
  Serial.print(message.uid);
  Serial.print(" ");
  Serial.print(message.length);
  Serial.print(" ");
  if (message.length >= 2)
    Serial.print((message.data[0]<<8) + message.data[1]);
  Serial.println();  

}