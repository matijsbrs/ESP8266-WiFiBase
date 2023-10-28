#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#include "wifi/webpage.h"
#include "settings.h"

extern ESP8266WebServer server;

    extern struct settings user_wifi;
    extern const char* applicationUUID;
    extern const char* default_mqtt_server;
    extern const char* default_mqtt_topic;
    extern const char* Version;


void handleCaptive()
{

  if (server.method() == HTTP_POST)
  {

    strncpy(user_wifi.ssid, server.arg("ssid").c_str(), sizeof(user_wifi.ssid));
    strncpy(user_wifi.password, server.arg("password").c_str(), sizeof(user_wifi.password));
    strncpy(user_wifi.mqtt_server, server.arg("mqtt").c_str(), sizeof(user_wifi.mqtt_server));
    user_wifi.ssid[server.arg("ssid").length()] = user_wifi.password[server.arg("password").length()] = user_wifi.mqtt_server[server.arg("mqtt").length()] = '\0';

    Serial.println(user_wifi.ssid);
    Serial.println(user_wifi.password);
    Serial.println(user_wifi.mqtt_server);

    EEPROM.put(0, user_wifi);
    EEPROM.commit();
    Serial.println("Ready ");


    // Send HTML form as response
    server.send(200, "text/html", webpage_Setup_Ready("Configuration ready", user_wifi.ssid, user_wifi.password, user_wifi.mqtt_server ));
  }
  else
  {
    // Get list of available wifi networks
    String networks = "";
    int numNetworks = WiFi.scanNetworks();
    for (int i = 0; i < numNetworks; i++) {
      networks += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
    }
    // Send HTML form as response
    server.send(200, "text/html", webpage_CaptivePortal("Captive Portal", networks, default_mqtt_server));
  }
}