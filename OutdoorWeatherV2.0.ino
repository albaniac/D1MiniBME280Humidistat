/*
Used a lot of help from the examples in:
-the Arduino BME280 library.  Copyright (C) 2016  Tyler Glenn
-HTTPsecureUpdate in the ESP8266 arduino library
-Thingspeak library

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Written: 2 September 2017.
  Last Updated: 2 September 2017.

  Connecting the BME280 Sensor:
  Sensor              ->  Board
  -----------------------------
  Vin (Voltage In)    ->  3.3V
  Gnd (Ground)        ->  Gnd
  SDA (Serial Data)   ->  A4 on Uno/Pro-Mini, 20 on Mega2560/Due, 2 Leonardo/Pro-Micro
  SCK (Serial Clock)  ->  A5 on Uno/Pro-Mini, 21 on Mega2560/Due, 3 Leonardo/Pro-Micro

*/

/* ==== Includes ==== */
#include <BME280I2C.h>
#include <Wire.h>  // Needed for legacy versions of Arduino.
#include "ThingSpeak.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
/* ====  END Includes ==== */

/* ==== Defines ==== */
#define SERIAL_BAUD 115200
char ssid[] = "<yourSSIDhere>";    //  your network SSID (name)
char pass[] = "<yourpasswordhere>";   // your network password
int status = WL_IDLE_STATUS;
WiFiClient  wifi;
/* ==== thingspeak credentials ==== */
unsigned long myChannelNumber = <yourchannelhere>;
const char * myWriteAPIKey = "<yourwritekeyhere>";
/* ==== END ts credentials ==== */

/* ==== Secure web updater ==== */
const char* host = "outdoorSensor";
const char* update_path = "/firmware";
const char* update_username = "<youradminhere>";
const char* update_password = "<youradminpasswordhere>";
/* ==== END Secure web updater ==== */

/* ==== END Defines ==== */

/* ==== Global Variables ==== */

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

BME280I2C bme;                   // Default : forced mode, standby time = 1000 ms
// Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
bool metric = false;

unsigned long lastTime = millis();
unsigned long lastReport = millis();

/* ==== END Global Variables ==== */


/* ==== Prototypes ==== */
/* === Print a message to stream with the temp, humidity and pressure. === */
void printBME280Data(Stream * client);

/* ==== END Prototypes ==== */

/* ==== Setup ==== */
void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial) {} // Wait
  while (!bme.begin()) {
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
   while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
    /* ==== begin HTTP update setup ==== */
 MDNS.begin(host);
   httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();
   MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);
  /* ==== End HTTP update setup ==== */
  
  ThingSpeak.begin(wifi);
}
/* ==== END Setup ==== */

/* ==== Loop ==== */
void loop() {

  doReport(millis());
  httpServer.handleClient();
}
/* ==== End Loop ==== */

/* ==== Functions ==== */
void printBME280Data(Stream* client) {
  float temp(NAN), hum(NAN), pres(NAN);
  uint8_t pressureUnit(2);                                           // unit: 0 = Pa, 1 = hPa, 2 = Hg, 3 = atm, 4 = bar, 5 = torr, 6 = N/m^2, 7 = psi
  bme.read(pres, temp, hum, metric, pressureUnit); // Parameters: (float& pressure, float& temp, float& humidity, bool celsius = false, uint8_t pressureUnit = 0x0)

  pres += 0.8;
    //do thingspeak report
  tSreport(temp, hum, pres);
  
  client->print("\t\tTemp: ");
  client->print(temp);
  client->println("°"+ String(metric ? 'C' : 'F'));
  client->print("\t\tHumidity: ");
  client->print(hum);
  client->println("% RH");
  client->print("\t\tPressure: ");
  client->print(pres);
  client->println(" inHg");

}
void doReport(unsigned long thisReport) {

  if ((thisReport - lastReport) > 30000) {
    Serial.println("calling BME update...");
    printBME280Data(&Serial);
    lastReport = millis();
  }
}
void tSreport(float t, float h, float p) {
  ThingSpeak.setField(1, t);
    Serial.print("temperature = ");
    Serial.println(t);
  ThingSpeak.setField(2, h);
    Serial.print("   humidity = ");
    Serial.println(h);
  ThingSpeak.setField(3, p);
    Serial.print("   pressure = ");
    Serial.println(p);
    
    Serial.print("Sending to ThingSpeak...");
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    Serial.println("done!");
}
/* ==== END Functions ==== */
