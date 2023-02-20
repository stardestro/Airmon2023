#include <TinyGPS++.h>
#include <SoftwareSerial.h>
/*
   This sample sketch demonstrates the normal use of a TinyGPS++ (TinyGPSPlus) object.
   It requires the use of Serial3, and aSerial3umes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/

enum p_State{
  S_PM,
  S_GPS,
  S_WIFI,
  S_NONE
};
static p_State s;

SoftwareSerial wifi(10,9);

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};
struct pms5003data data;  // declare struct for the pms data


struct Data_frame{
//  double lat;
//  double lng;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
};

struct Data_frame wifi_data;

static const uint32_t GPSBaud = 9600;
// The TinyGPS++ object
TinyGPSPlus gps;
// The serial connection to the GPS device


double g_lat,g_lng;


void setup()
{
  s = S_PM;
  Serial.begin(115200);
  Serial2.begin(9600);
  Serial3.begin(GPSBaud);
  wifi.begin(4800);

  Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println();
}

void loop()
{
  
  switch(s){
    case S_PM:
      if(Serial2.available()>0){
        if (readPMSdata(&Serial2)) {
          // reading data was successful!
          wifi_data.pm10_standard = data.pm10_standard;
          wifi_data.pm25_standard = data.pm25_standard;
          wifi_data.pm100_standard = data.pm100_standard;
          wifi_data.pm10_env = data.pm10_env;
          wifi_data.pm25_env = data.pm25_env;
          wifi_data.pm100_env = data.pm100_env;
          wifi_data.particles_03um = data.particles_03um;
          wifi_data.particles_05um = data.particles_05um;
          wifi_data.particles_10um = data.particles_10um;
          wifi_data.particles_25um = data.particles_25um;
          wifi_data.particles_50um = data.particles_50um;
          wifi_data.particles_100um = data.particles_100um;
          s = S_GPS;
        }
      break;
    case S_GPS:
      if (Serial3.available() > 0){
        if (gps.encode(Serial3.read())){
          displayInfo();
          g_lat = gps.location.lat();
          g_lng = gps.location.lng();
          s = S_WIFI;
        }
      }
      break;
    case S_WIFI:
      Serial.println("---------------------------------------");
      Serial.println("Concentration Units (standard)");
      Serial.print("PM 1.0: "); Serial.print(wifi_data.pm10_standard);
      Serial.print("\t\tPM 2.5: "); Serial.print(wifi_data.pm25_standard);
      Serial.print("\t\tPM 10: "); Serial.println(wifi_data.pm100_standard);
      Serial.println("---------------------------------------");
      Serial.println("Concentration Units (environmental)");
      Serial.print("PM 1.0: "); Serial.print(wifi_data.pm10_env);
      Serial.print("\t\tPM 2.5: "); Serial.print(wifi_data.pm25_env);
      Serial.print("\t\tPM 10: "); Serial.println(wifi_data.pm100_env);
      Serial.println("---------------------------------------");
      Serial.print("Particles > 0.3um / 0.1L air:"); Serial.println(wifi_data.particles_03um);
      Serial.print("Particles > 0.5um / 0.1L air:"); Serial.println(wifi_data.particles_05um);
      Serial.print("Particles > 1.0um / 0.1L air:"); Serial.println(wifi_data.particles_10um);
      Serial.print("Particles > 2.5um / 0.1L air:"); Serial.println(wifi_data.particles_25um);
      Serial.print("Particles > 5.0um / 0.1L air:"); Serial.println(wifi_data.particles_50um);
      Serial.print("Particles > 10.0 um / 0.1L air:"); Serial.println(wifi_data.particles_100um);
      Serial.println("---------------------------------------");
//          Serial.print(F("Location: ")); 
//          Serial.print(wifi_data.lat, 6);
//          Serial.print(F(","));
//          Serial.println(wifi_data.lng, 6);
//          wifi.print("1");
//          Serial.println("sned:1");
//          wifi.flush();
//          Serial.println("flush");
      wifi.write((const char*)&wifi_data, sizeof(wifi_data));
      wifi.write((const char*)&(g_lat), sizeof(double));
      wifi.write((const char*)&(g_lng), sizeof(double));
      delay(1000);
      Serial.println("send all data");
      while(true){
        if(wifi.available()){
          if(wifi.read() == '1'){
            Serial.println("get response");
            break;
          }else{
            s = S_WIFI;
            return;
          }
        }
      }
      s = S_PM;
      break;
    }
  }  
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

boolean readPMSdata(Stream *s) {
  if (! s->available()) {
    return false;
  }
  
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }
 
  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }
    
  uint8_t buffer[32];    
  uint16_t sum = 0;
  s->readBytes(buffer, 32);
 
  // get checksum ready
  for (uint8_t i=0; i<30; i++) {
    sum += buffer[i];
  }
 
  
  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }
 
 
  memcpy((void *)&data, (void *)buffer_u16, 30);
 
  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}
