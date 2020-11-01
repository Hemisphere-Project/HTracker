#include <Arduino.h>
#include <M5Stack.h>
#include <M5ez.h>
#include <can.h>
#include <receiver.h>
#include <teraranger.h>
#include <Preferences.h>


#define HVERSION 0.2  // merge sender / receiver


// Common settings
//
//#define SET_MODE  0     // 0:RECV / 1:TERA
//#define SET_HID   0    // HARDWARE ID
int MODE;
int HID;

// Receiver variables
//
MeasureBook* book;

// Sender variables
//
#define KEEPALIVE_TIME 500
uint32_t last_send = 0;
uint16_t last_value = 0;

// Common variables
//
String txt_title  =  "HTracker ";
String txt_hid    =  "HID: ";


void setup() 
{
  Serial.begin(115200);

  // Preferences
  //
  Preferences preferences;
  preferences.begin("HTRACKER", false);

  #ifdef SET_MODE
    preferences.putUInt("mode", SET_MODE);
  #endif
  MODE = preferences.getUInt("mode", 0);

  #ifdef SET_HID
    preferences.putUInt("hid", SET_HID);
  #endif
  HID = preferences.getUInt("hid", 0);

  preferences.end();

  // Prepare
  //
  txt_hid += String(HID);
  txt_title += String(HVERSION)+" ";
  if (MODE == 0)  txt_title += "RECV";
  if (MODE == 1)  txt_title += "TERA";

  // Display
  //
  #include <themes/dark.h>
  ez.begin();
  M5.Speaker.mute();

  ez.screen.clear();
  ez.header.show(txt_title);
  ez.canvas.lmargin(10);
  ez.canvas.font(&FreeSans9pt7b);
  ez.canvas.println("");
  ez.canvas.println("Hello");

  // MODE RECV: Create Measure Book
  if (MODE == 0) {
    book = new MeasureBook();
  }

  // MODE TERA: Starting TeraRanger I2C
  if (MODE == 1) {
    ez.canvas.print("[INIT ] Starting TeraBee I2C.. ");
    if (tera_setup()) ez.canvas.println("OK!");
    else ez.canvas.println("ERROR");
  }

  // Starting CAN interface
  ez.canvas.print("[INIT ] Starting CAN Bus.. ");
  if (can_setup()) ez.canvas.println("OK!");
  else ez.canvas.println("ERROR");

  // Ready
  ez.canvas.println("[READY] Waiting for data.. ");

  delay(1000);
}


void loop() 
{

  // MODE RECV: listen and process
  if (MODE == 0) 
  {
    CanMessage* msg = can_read();
    if (msg == nullptr) return;

    // Process
    book->process(msg);

    // UI list
    String list = "";
    for (int k=0; k<255; k++) {
      float v = book->value_m(k);
      if (v != -1) list += String(k)+": "+String(v)+" m"+"\n" ;
    }

    // Screen display
    ez.msgBox(txt_title, list, "", false);

    // Serial report
    Serial.println(String(msg->uid())+":"+String(msg->value())); 
     
    delete msg;
  }


  // MODE TERA: measure and send
  else if (MODE == 1) 
  {
    uint16_t current_value = tera_read();
    bool trig = (current_value/50 != last_value/50) || (millis() - last_send > KEEPALIVE_TIME);
    if (trig) 
    {
      // Send value
      CanMessage* msg = new CanMessage(HID, MEASURE, current_value);
      can_send(msg);

      last_value = current_value;
      last_send = millis();
      
      // Screen display
      ez.msgBox(txt_title, txt_hid+"\n"+String(msg->value()/1000.)+" m", "", false);

      delete msg;
    }
  }

  // Serial.println(ESP.getFreeHeap());
}