#include <Arduino.h>
#include <M5Stack.h>
#include <M5ez.h>
#include <can.h>
#include <receiver.h>
#include <teraranger.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

#define HVERSION 02  // merge sender / receiver
#define HVERSION 03  // Laser pointer
#define HVERSION 04  // Wifi and OTA
#define HVERSION 05  // Fixed Wifi


// Common settings
//
// #define SET_MODE  1     // 0:RECV / 1:TERA
// #define SET_HID   4    // HARDWARE ID
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

#define LASER_TEMPO       10
#define LASER_BTN_PIN     2
#define LASER_DISABLE_PIN 5
unsigned long laserUp = 0;

// Common variables
//
String txt_title  =  "HTracker ";
String txt_hid    =  "HID: ";
bool wifiConnected = false;


void wifiEvent(WiFiEvent_t event) 
{
  if (event == SYSTEM_EVENT_STA_DISCONNECTED) wifiConnected = false;
  else if (event == SYSTEM_EVENT_STA_GOT_IP)
  {
    ArduinoOTA.begin();
    wifiConnected = true;
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

// SETUP
// 
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

  pinMode(LASER_BTN_PIN, INPUT_PULLUP);
  pinMode(LASER_DISABLE_PIN, OUTPUT);
  digitalWrite(LASER_DISABLE_PIN, HIGH);

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
  ez.canvas.println("[WIFI] Trying to connect HTracker ");

  // WIFI
  //
  String hostname = "tracker-"+String(HID)+"-v"+String(HVERSION);
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(wifiEvent);
  WiFi.setHostname(hostname.c_str());
  WiFi.begin("HTracker", "supernet");
  ArduinoOTA.setHostname( hostname.c_str() );

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

    // LASER
    if (digitalRead(LASER_BTN_PIN) == LOW) 
    {
      digitalWrite(LASER_DISABLE_PIN, LOW);
      laserUp = millis();
    }
    else if (laserUp > 0 && millis()-laserUp > LASER_TEMPO*1000){
      digitalWrite(LASER_DISABLE_PIN, HIGH);
      laserUp = 0;
    }

  }

  // Serial.println(ESP.getFreeHeap());
  Serial.println(digitalRead(LASER_BTN_PIN));
  if (wifiConnected) ArduinoOTA.handle();
}

