#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <time.h>
#include <OneWire.h>
#include <DallasTemperature.h>
extern "C" {
#include "user_interface.h"
}


#define ONE_WIRE_BUS 2  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

WiFiUDP Udp;
IPAddress remoteIP(10,10,10,10); //graphite server
unsigned int remotePort = 2003;  //graphite server port
unsigned int localPort = 2003;

const char* ssid     = "wifi_network_ssid";
const char* password = "wifi_network_password";


void setup() {

  Serial.begin(115200);
  delay(200);
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println("Wait for WiFi... ");

  while(WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //void configTime(int timezone, int daylightOffset_sec, const char* server1, const char* server2, const char* server3)
  configTime(0, 0, "0.europe.pool.ntp.org");

  //checking if time is synced properly, if not - retrying
  Serial.println("Waiting for time");
  while(time(nullptr) <= 100000) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("NTP synced");

  //blinking led if wifi is connected and time is synced
  for (int i=1;i<10;i++){
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);                      
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50); 
  }

}

void loop() {

    send_temperature_to_graphite();
    send_wifi_rssi_to_graphite();
    
    Serial.println("Delay for 50 seconds");
    delay(50000);

}

void send_temperature_to_graphite() {

  time_t now = time(nullptr);

  // temperature section
  char graphite_temperature[100];
  float temp;
  DS18B20.requestTemperatures();
  delay(5000);
  temp = DS18B20.getTempCByIndex(0);

  /* example of graphite_temperature_string: 
   * esp.temp 23.87 1540228668
   */

  String graphite_temperature_string = "esp.temp " + String(temp) + " " + now;
  graphite_temperature_string.toCharArray(graphite_temperature, 100);

  Udp.beginPacket(remoteIP, remotePort);
  Udp.write(graphite_temperature);
  Udp.endPacket();
}

void send_wifi_rssi_to_graphite() {

  time_t now = time(nullptr);
  
  // wifi rssi section
  char graphite_rssi[100];
  int rssi = WiFi.RSSI();
  String ssid = WiFi.SSID();

  /* example of graphite_rssi_string: 
   * esp.rssi.JAN -52 1540228502
   */
  String graphite_rssi_string = "esp.rssi." + ssid + " " + rssi + " " + now;
  graphite_rssi_string.toCharArray(graphite_rssi, 100);

  Udp.beginPacket(remoteIP, remotePort);
  Udp.write(graphite_rssi);
  Udp.endPacket();

}
