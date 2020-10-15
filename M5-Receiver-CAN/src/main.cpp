#include <Arduino.h>
#include <M5Stack.h>
#include <M5ez.h>
#include <can.h>

const char* title =  "HTracker v0.1 - RECEIVER";

CanMessage* msg;

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
  
  // Starting CAN interface
  ez.canvas.print("[INIT ] Starting CAN Bus.. ");
  if (can_setup()) ez.canvas.println("OK!");
  else ez.canvas.println("ERROR");

  // Ready
  ez.canvas.println("[READY] Waiting for data.. ");
}


void loop() 
{
  msg = can_read();
  if (msg == nullptr) return;

  // Screen display
  ez.msgBox(title, String(msg->uid())+": "+String(msg->value()/1000.)+" m", "", false);

  // Serial report
  Serial.println(String(msg->uid())+":"+String(msg->value()));  

  delete msg;
  msg = NULL;
}