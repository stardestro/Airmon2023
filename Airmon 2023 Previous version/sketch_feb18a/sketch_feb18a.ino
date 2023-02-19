//#include <WiFiNINA.h>
//#include <ThingSpeak.h>
#include "SoftwareSerial.h"
#include <TinyGPSPlus.h>

#define WIFISSID 
#define PASSWORD 
#define THINKSPEAKCHANNELID 2037739
#define THINKSPEAKWRITE YKGJ1TCXS652JBUP
#define THINKSPEAKREAD PW28Q12FZ8342CBP

enum p_State{
  S_PM,
  S_GPS,
  S_WIFI,
  S_NONE
};

SoftwareSerial wifi(10,9);

struct pms5003data {
  uint16_t framelength;
  uint16_t pm10standard, pm25standard, pm100standard;
  uint16_t pm10atm, pm25atm, pm100atm;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

// The TinyGPSPlus object
TinyGPSPlus gps;
static const uint32_t GPSBaud = 9600;
struct pms5003data d_data;
boolean readPMSdata(Stream *s);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //Air Quality module
  Serial3.begin(9600);

  //GPS module
  Serial2.begin(GPSBaud);

  Serial.println(F("DeviceExample.ino"));
  Serial.println(F("A simple demonstration of TinyGPSPlus with an attached GPS module"));
  Serial.print(F("Testing TinyGPSPlus library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial3.available()>0){
    if(readPMSdata(&Serial3)){
      Serial.println("--------------------------");
      Serial.println("Concentration Units (standard) u g/m3");
      Serial.print("PM1.0: "); Serial.println(d_data.pm10standard);
      Serial.print("PM2.5: "); Serial.println(d_data.pm25standard);
      Serial.print("PM10.0: "); Serial.println(d_data.pm100standard);
      Serial.println("--------------------------");
      Serial.println("Concentration Units (atmospheric) u g/m3");
      Serial.print("PM1.0: "); Serial.println(d_data.pm10atm);
      Serial.print("PM2.5: "); Serial.println(d_data.pm25atm);
      Serial.print("PM10.0: "); Serial.println(d_data.pm100atm);
      Serial.println("--------------------------");
      Serial.println("Concentration Units (atmospheric) u g/m3");
      Serial.print("# of particles(> 0.3um in 1L): "); Serial.println(d_data.particles_03um);
      Serial.print("# of particles(> 0.5um in 1L): "); Serial.println(d_data.particles_05um);
      Serial.print("# of particles(> 1.0um in 1L): "); Serial.println(d_data.particles_10um);
      Serial.print("# of particles(> 2.5um in 1L): "); Serial.println(d_data.particles_25um);
      Serial.print("# of particles(> 5.0um in 1L): "); Serial.println(d_data.particles_50um);
      Serial.print("# of particles(>10  um in 1L): "); Serial.println(d_data.particles_100um);
    }
  }
    if(Serial2.available()>0){
      if (gps.encode(Serial2.read())){
        displayInfo();
      }
      if (millis() > 5000 && gps.charsProcessed() < 10){
        Serial.println(F("No GPS detected: check wiring."));
        while(true);
      }
    }
    
  
  // This sketch displays information every time a new sentence is correctly encoded.
  
}

//Create function for PMS5003 convert endian format to decimal and print it out in an easy-to-read format

boolean readPMSdata(Stream *s){
  if(! s->available()){
    return false;
  }
  //Read a byte at a time until we get to the special '0x42' start byte
  if(s->peek() != 0x42){
    s->read();
    return false;
  }
  //now read all 32 bytes
  if(s->available() < 32){
    return false;
  }
  uint8_t buffer[32];
  uint16_t sum = 0;
  s->readBytes(buffer, 32);

  // get checksum ready
  for(uint8_t i=0; i<30; i++){
    sum += buffer[i];
  }

  
  uint16_t buffer_u16[15];
  //loop 16 times
  for (uint8_t i=0; i<15; i++){
    
//    //2*index is first int8
//    //(2*index) + 1 is second int8
//
//    //store first int8
//    uint8_t store = buffer[(2*i)];
//    //take second int8 into position of first int8
//    buffer[(2*i)] = buffer[(2*i)+1];
//    //place stored int8 into position of second int8
//    buffer[(2*i)+1] = store;

    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2+i*2] << 8);
  }

  memcpy((void *)&d_data, (void *)buffer_u16, 30);

  if (sum != d_data.checksum){
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}

void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
