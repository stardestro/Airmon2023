#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

#include <SoftwareSerial.h>

int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

struct Data_frame{
//  double lat;
//  double lng;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
};

double g_lat,g_lng;
struct Data_frame d_data;

enum p_State{
  S_IDLE,
  S_PRE_RECV,
  S_RECV,
  S_SEND,
  S_NONE
};

static p_State s;

SoftwareSerial data(10,9);

void setup() {
  g_lat = -1.0;
  g_lng = -1.0;
  s = S_IDLE;
  data.begin(4800);
  Serial.begin(115200);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);  // don't continue
  }
  
  String fv = WiFi.firmwareVersion();
  if (fv != "1.0.0") {
    Serial.print("Firmware version:");
    Serial.println(fv);
  }
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(SECRET_SSID, SECRET_PASS); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(500);     
    } 
    Serial.println("\nConnected.");
  }
  switch(s){
    case S_IDLE:
//      Serial.println("IDLE");
      if(data.available())
        s = S_RECV;
      break;
    case S_PRE_RECV:
      Serial.println("S_PRE_RECV");
      if(data.available()){
        if(data.read() == "1"){
          s = S_RECV;
        }
      }
      break;
    case S_RECV:
      Serial.println("S_RECV");
      if(data.available()){
          data.readBytes((char*)&d_data, sizeof(Data_frame)) == sizeof(Data_frame);
          data.readBytes((char*)&g_lat, sizeof(double)) == sizeof(double);
          data.readBytes((char*)&g_lng, sizeof(double)) == sizeof(double);
          s = S_SEND;
        }
      break;
    case S_SEND:
//    Serial.println("S_SEND");
      Serial.println("---------------------------------------");
      Serial.println("Concentration Units (standard)");
      Serial.print("PM 1.0: "); Serial.print(d_data.pm10_standard);
      Serial.print("\t\tPM 2.5: "); Serial.print(d_data.pm25_standard);
      Serial.print("\t\tPM 10: "); Serial.println(d_data.pm100_standard);
      Serial.println("---------------------------------------");
      Serial.println("Concentration Units (environmental)");
      Serial.print("PM 1.0: "); Serial.print(d_data.pm10_env);
      Serial.print("\t\tPM 2.5: "); Serial.print(d_data.pm25_env);
      Serial.print("\t\tPM 10: "); Serial.println(d_data.pm100_env);
      Serial.println("---------------------------------------");
      Serial.print("Particles > 0.3um / 0.1L air:"); Serial.println(d_data.particles_03um);
      Serial.print("Particles > 0.5um / 0.1L air:"); Serial.println(d_data.particles_05um);
      Serial.print("Particles > 1.0um / 0.1L air:"); Serial.println(d_data.particles_10um);
      Serial.print("Particles > 2.5um / 0.1L air:"); Serial.println(d_data.particles_25um);
      Serial.print("Particles > 5.0um / 0.1L air:"); Serial.println(d_data.particles_50um);
      Serial.print("Particles > 10.0 um / 0.1L air:"); Serial.println(d_data.particles_100um);
      Serial.println("---------------------------------------");
      Serial.print(F("Location: ")); 
      Serial.print(g_lat, 6);
      Serial.print(F(","));
      Serial.println(g_lng, 6);
      Serial.println("");

      int pm10 = d_data.pm10_env;
      int pm25 = d_data.pm25_env;
      int pm100 = d_data.pm100_env;
      
      ThingSpeak.setField(1, pm10);
      ThingSpeak.setField(2, pm25);
      ThingSpeak.setField(3, pm100);
      ThingSpeak.setField(4, (float)g_lat);
      ThingSpeak.setField(5, (float)g_lng);
      int x = ThingSpeak.writeFields(SECRET_CH_ID, SECRET_WRITE_APIKEY);
      if(x == 200){
        Serial.println("Channel update successful.");
      }else{
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }
      delay(16000);
      data.write('1');
      s = S_IDLE;
      break;
  }

}
