#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;
#include <SPI.h>
#include <Ethernet.h>
#include <WiFiUdp.h>
#include <ntp.h>
#include <Time.h>
#include <Ticker.h>
#include <OneWire.h>
#include <DallasTemperature.h>
extern "C" {
#include "user_interface.h"
}
#define ONE_WIRE_BUS 2  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
 
time_t getNTPtime(void);
NTP NTPclient;
Ticker clock;
bool colon = true;
bool updateTime = true;
define GMT +0 //timezone
WiFiUDP Udp;
IPAddress remoteIP(87,99,5,234); //graphite server
unsigned int remotePort = 2003;  //graphite server port
unsigned int localPort = 8888;

void setup() {

  Serial.begin(115200);
  delay(200);
  pinMode(LED_BUILTIN, OUTPUT);
  // We start by connecting to a WiFi network
  WiFiMulti.addAP("JAN", "kluczsieciowy");
  WiFiMulti.addAP("Maciej");

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while(WiFiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Udp.begin(localPort);
  while(timeStatus() == timeNotSet) {
    Serial.println("waiting for ntp");
    NTPclient.begin("time.nist.gov", GMT);
    setSyncInterval(SECS_PER_HOUR);
    setSyncProvider(getNTPtime);
    delay(1000);
  }
  clock.attach(0.5, toggleColon);
  //blinking led if wifi is connected and time is synced
  for (int i=1;i<10;i++){
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);                      
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50); 
  }
}

void loop() {
  if(timeStatus() == timeNotSet)
    Serial.println("waiting for ntp");
  else {
    Udp.beginPacket(remoteIP, remotePort);
    float temp;
    DS18B20.requestTemperatures(); 
    temp = DS18B20.getTempCByIndex(0);
    Udp.write("esp.temp ");
    Udp.print(temp); 
    Udp.write(" ");
    Udp.println(now());
    Serial.write("esp.temp ");
    Serial.print(temp); 
    Serial.write(" ");
    Serial.println(now());
    Udp.endPacket();
    
    Udp.beginPacket(remoteIP, remotePort);
    char graphite_rssi[50];
    int rssi = WiFi.RSSI();
    String ssid = WiFi.SSID();
    String graphite_data = "esp.rssi." + ssid + " ";
    graphite_data.toCharArray(graphite_rssi, 50);
    Udp.write(graphite_rssi);
    Udp.print(rssi); 
    Udp.write(" ");
    Udp.println(now());    
    Serial.write(graphite_rssi);
    Serial.print(rssi); 
    Serial.write(" ");
    Serial.println(now());
    Udp.endPacket(); }
  delay(55000);
}

time_t getNTPtime(void)
{
  return NTPclient.getNtpTime();
}
void toggleColon(void)
{
   updateTime = true;  
}
