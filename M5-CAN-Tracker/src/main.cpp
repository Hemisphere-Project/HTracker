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
#include <movingAvg.h>          // https://github.com/JChristensen/movingAvg

#define HVERSION 2  // merge sender / receiver
#define HVERSION 3  // Laser pointer
#define HVERSION 4  // Wifi and OTA
#define HVERSION 5  // Fixed Wifi
#define HVERSION 6  // Timings
#define HVERSION 7  // Moving Avg
#define HVERSION 9  // Stabilisator
#define HVERSION 10  // BTNs


// Common settings
//
// #define SET_MODE  1     // 0:RECV / 1:TERA
// #define SET_HID   1    // HARDWARE ID
int MODE;
int HID;

// Receiver variables
//
MeasureBook* book;

// Sender variables
//
#define KEEPALIVE_TIME 100
uint32_t last_send = 0;
uint16_t last_value = 0;

#define LASER_TEMPO       10
#define LASER_BTN_PIN     2
#define LASER_DISABLE_PIN 5
unsigned long laserUp = 0;
bool laserState = false;

#define STABSIZE 2
int stability_counter = 0;
movingAvg movingMeasure(STABSIZE);

// Common variables
//
String txt_title  =  "HTracker ";
String txt_hid    =  "HID: ";
bool wifiConnected = false;

void usbRead();
void wifiSetup();

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
  txt_title += " v"+String(HVERSION)+" ";
  if (MODE == 0)  txt_title += " -RECV";
  if (MODE == 1)  txt_title += " -TERA";

  pinMode(LASER_BTN_PIN, INPUT_PULLUP);
  pinMode(LASER_DISABLE_PIN, OUTPUT);
  digitalWrite(LASER_DISABLE_PIN, HIGH);
  movingMeasure.begin();

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

  // WIFI
  wifiSetup();

  delay(1000);
}


void loop() 
{
  // MODE RECV: listen and process
  if (MODE == 0) 
  {
    usbRead();

    CanMessage* msg = can_read();

    // Valid MSG
    if (msg != nullptr) 
    {
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
    }

    delete msg;
  }


  // MODE TERA: measure and send
  else if (MODE == 1) 
  {
    bool trig = false;
    uint16_t current_value = tera_read();
    
    if (current_value > 0) 
    {
      // Serial.println(current_value);
      movingMeasure.reading(current_value);

      // Stabilisator
      bool stabilized = true;
      int* myData = movingMeasure.getReadings();
      for(int k=0; k<movingMeasure.getCount(); k++) {
        if (abs(current_value-myData[k]) > 10) {
          stabilized = false;
          break;
        }
      }

      // Stabilized
      if (stabilized && abs(current_value - last_value) > 40) 
      {    
        last_value = current_value;
        trig = true;
        Serial.printf("\n[%d]\n", last_value);
      }
    }

    // keep alive
    if (millis() - last_send > KEEPALIVE_TIME+random(10)) trig = true;

    // SEND
    if (trig) 
    {
      // Send value
      CanMessage* msg = new CanMessage(HID, MEASURE, last_value);
      can_send(msg);

      last_send = millis();

      // Screen display
      ez.msgBox(txt_title, txt_hid+"\n"+String(msg->value()/1000.)+" m", "", false);

      delete msg;
    }

    // LASER TEMPO
    // if (digitalRead(LASER_BTN_PIN) == LOW) 
    // {
    //   digitalWrite(LASER_DISABLE_PIN, LOW);
    //   laserUp = millis();
    // }
    // else if (laserUp > 0 && millis()-laserUp > LASER_TEMPO*1000){
    //   digitalWrite(LASER_DISABLE_PIN, HIGH);
    //   laserUp = 0;
    // }

    // LASER SWITCH
    if (digitalRead(LASER_BTN_PIN) == LOW) {
      laserState = !laserState;
      digitalWrite(LASER_DISABLE_PIN, !laserState);
      delay(200);
    }

  }

  // Serial.println(ESP.getFreeHeap());
  // Serial.println(digitalRead(LASER_BTN_PIN));
  if (wifiConnected) ArduinoOTA.handle();
}


// WIFI
//

void wifiEvent(WiFiEvent_t event) 
{
  if (event == SYSTEM_EVENT_STA_DISCONNECTED) wifiConnected = false;
  else if (event == SYSTEM_EVENT_STA_GOT_IP)
  {
    ArduinoOTA.begin();
    wifiConnected = true;
    // Serial.print("IP address: ");
    // Serial.println(WiFi.localIP());
    ez.canvas.println("[WIFI] connected IP: "+String(WiFi.localIP().toString()));
  }
}

void wifiSetup() 
{ 
  String ssid = "HTracker";
  String pass = "supernet";
  // Ready
  ez.canvas.println("[WIFI] connect "+ssid+"/"+pass);

  String hostname = "tracker-"+String(HID)+"-v"+String(HVERSION);
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(wifiEvent);
  WiFi.setHostname(hostname.c_str());
  WiFi.begin(ssid.c_str(), pass.c_str());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  ArduinoOTA.setHostname( hostname.c_str() );
}

// USB READ
//
char incomingByte = 0;
char buffer = 0;
void usbRead() {
  if (Serial.available() > 0) {
    incomingByte = Serial.read();

    if (incomingByte == '\n') {
      if (buffer == 0) return;

      // Start wifi
      if (buffer == 'w') wifiSetup();

      // Reboot
      else if (buffer == 'x') {
        ESP.restart();
        while(true) delay(10000);
      }

      buffer = 0;
    } 
    else buffer = incomingByte;

  }
}