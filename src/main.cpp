#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h> 
#include <TelnetStream.h>
#include <PubSubClient.h>


#include "Credentials.h"
#include "Ota.h"
#include "Controls.h"

// function declarations
// void mqtt_callback(char* topic, byte* message, unsigned int length);
void transmit_mqtt(const char * extTopic,const char * Field, const char * payload);

char ssid[20];
char password[20];

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Web server
ESP8266WebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(172, 217, 28, 1);
IPAddress netMsk(255, 255, 255, 0);


// MQTT stuff
WiFiClient espClient;
PubSubClient client(espClient);

const char* applicationUUID = "012345678";
const char* default_mqtt_server = "192.168.2.201";
const char* TOPIC = "climate/controller";
const char* Version = "V0.0.1";

// MQTT stuff end

void handleCaptive();

enum system_state
{
  unknown,
  booting,
  wifi_reset,
  wifi_connecting,
  wifi_ready,
  wifi_ap_mode
};

struct system_configuration
{
  enum system_state state = unknown;
} configuration;

struct settings
{
  char ssid[32];         // the SSID of the netwerk
  char password[32];     // the WifiPassword
  char energy_topic[32]; // the path to the topic on which the current wattage is found.
  char mqtt_server[32];  // Addres of the MQTT server
  unsigned long period;

} user_wifi = {};

const byte iotResetPin = D0;

unsigned long startMillis; // some global variables available anywhere in the program
unsigned long currentMillis;

// PWM Pin Definition
#define Relais_1 D0 
#define Relais_2 D5
#define Relais_3 D6
#define Relais_4 D7
#define Relais_5 D8
#define Relais_6 D3
#define Relais_7 D2
#define Relais_8 D1

// Renamed
#define _24VAC D0 
#define _Valve_A D5
#define _Valve_B D6
#define _HighFlow D7
#define _MediumFlow D8
#define _StandBy D3
#define _Relais_7 D2
#define _Relais_8 D1


void Load_defaults() {
  user_wifi.period = 1000; 
  strcpy(user_wifi.mqtt_server, default_mqtt_server);
}

void show_settings() {
  char value[32];
  snprintf(value, 32, "%ld", user_wifi.period );
  transmit_mqtt("settings","period",value);

  snprintf(value, 32, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
  transmit_mqtt("settings","ip",value);
  transmit_mqtt("settings","version",Version);
}

void Validate_settings() {
    if (user_wifi.period < 100 ) user_wifi.period = 100;
    if (user_wifi.period > 60000 ) user_wifi.period = 60000;
}


void mqtt_callback(char* topic, byte* message, unsigned int length) {
  // Serial.print("Message arrived on topic: ");
  // Serial.print(topic);
  // Serial.print(". Message: ");
  String messageTemp;
  
  // char mytopic[32];
  // snprintf(mytopic, 32, "light-%08X/set", ESP.getChipId());

  for (unsigned int i = 0; i < length; i++) {
    // Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  // Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  
    
  if (messageTemp == "settings=read" ) {
    EEPROM.get(0, user_wifi);
    Validate_settings();
  } else if (messageTemp == "settings=write" ) {
    EEPROM.put(0, user_wifi);
    EEPROM.commit();
  } else if (messageTemp == "settings=default" ) {
    Load_defaults();
    EEPROM.put(0, user_wifi);
    EEPROM.commit();
  } else if (messageTemp == "settings=show" ) {
    show_settings();
  } else if (messageTemp == "system=restart" ) {
    ESP.restart();
  }
}

void start_in_AP_Mode() {
  Serial.println("Starting in Access Point mode.");
  configuration.state = wifi_reset;
  WiFi.softAPConfig(apIP, apIP, netMsk);
    // don't use a passcode.
  WiFi.softAP(ssid); 
  Serial.print("Deivce IP address: ");
  Serial.println(WiFi.softAPIP());


  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

      /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server.on("/", handleCaptive);
  server.on("/wifi", handleCaptive);
  server.on("/wifisave", handleCaptive);
  server.on("/generate_204", handleCaptive);  // Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleCaptive);        // Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  server.onNotFound(handleCaptive);
  server.begin();  // Web server start
  Serial.println("HTTP server ready for captive page.");

  configuration.state = wifi_ap_mode;
}

void hardware_setup() {
  // Initialize PWM Pin
  pinMode(Relais_1, OUTPUT);
  pinMode(Relais_2, OUTPUT);
  pinMode(Relais_3, OUTPUT);
  pinMode(Relais_4, OUTPUT);
  pinMode(Relais_5, OUTPUT);
  pinMode(Relais_6, OUTPUT);
  pinMode(Relais_7, OUTPUT);
  pinMode(Relais_8, OUTPUT);

  digitalWrite(Relais_1, 0x00);
  digitalWrite(Relais_2, 0x00);
  digitalWrite(Relais_3, 0x00);
  digitalWrite(Relais_4, 0x00);
  digitalWrite(Relais_5, 0x00);
  digitalWrite(Relais_6, 0x00);
  digitalWrite(Relais_7, 0x00);
  digitalWrite(Relais_8, 0x00);

  delay(500);
  
  digitalWrite(Relais_1, 0x01); delay(200); digitalWrite(Relais_1, 0x00);
  digitalWrite(Relais_2, 0x01); delay(200); digitalWrite(Relais_2, 0x00);
  digitalWrite(Relais_3, 0x01); delay(200); digitalWrite(Relais_3, 0x00);
  digitalWrite(Relais_4, 0x01); delay(200); digitalWrite(Relais_4, 0x00);
  digitalWrite(Relais_5, 0x01); delay(200); digitalWrite(Relais_5, 0x00);
  digitalWrite(Relais_6, 0x01); delay(200); digitalWrite(Relais_6, 0x00);
  digitalWrite(Relais_7, 0x01); delay(200); digitalWrite(Relais_7, 0x00);
  digitalWrite(Relais_8, 0x01); delay(200); digitalWrite(Relais_8, 0x00);
}

void network_setup() {
  // Generate the SSID. Make it unique using the chip's UID
  sprintf(ssid, "MBE-%08X", ESP.getChipId());
  // sprintf(password, ACCESSPOINT_PASS);

  if (!digitalRead(iotResetPin))
  {
    start_in_AP_Mode();
  }
   else
  {
    configuration.state = wifi_connecting;
    // Read network data.
    EEPROM.get(0, user_wifi);

    Serial.printf("Wifi ssid: %s\n", user_wifi.ssid);
    Serial.printf("MQTT host: %s\n", user_wifi.mqtt_server);


    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true); // werkt dit?
    WiFi.begin(user_wifi.ssid, user_wifi.password);
    byte tries = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(2000);
      Serial.println("Retry connecting");
      if (tries++ > 15)
      {
        Serial.println("Failed, start ApMode()");
        start_in_AP_Mode();
        break;
      }
    }
  }

  
  if (WiFi.status() == WL_CONNECTED)
  {
    configuration.state = wifi_ready;
  }
}

void setup()
{
  
  pinMode(iotResetPin, INPUT_PULLUP);
  digitalWrite(iotResetPin, HIGH);


  Serial.begin(115200);
  Serial.println("Booting");

  hardware_setup();
  

  configuration.state = booting;
  EEPROM.begin(sizeof(struct settings));
  Load_defaults();
  
  network_setup();

  startMillis = millis(); // initial start time

  switch (configuration.state)
  {
  case wifi_connecting:
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
    break;

  case wifi_ap_mode:
    
    

    break;

  case wifi_ready:
    setupOTA();
    Serial.println("Ready for OTA");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    // set up mqtt stuff
    client.setServer(user_wifi.mqtt_server, 1883);
    client.setCallback(mqtt_callback);
    // on ready push settings to mqtt.
    show_settings();
    
    break;

  default:
    break;
  }
}



int connect_mqtt() {
  if (client.connected()) {
    return 1;
  }

  Serial.print("Attempting MQTT connection... ");

  // // Create a random client ID
  // String clientId = "ESP8266Client-";
  // sprintf(ssid, "light-%08X\n", ESP.getChipId());
  // clientId += String(random(0xffff), HEX);

  // Attempt to connect
  if (client.connect(ssid)) {
    Serial.println("connected");
    char topic[32];
    snprintf(topic, 32, "light-%08X/cmd", ESP.getChipId());
    Serial.println(topic);
    client.subscribe(topic);
    return 1;
  }

  Serial.print("failed, rc=");
  Serial.print(client.state());
  return 0;
}

void transmit_mqtt(const char * extTopic, const char * Field, const char * payload) {
  if (connect_mqtt()) {
    char topic[75];
    snprintf(topic, 75, "light-%08X/%s/%s", ESP.getChipId(), extTopic,Field);
    // Serial.println(topic);
    client.publish(topic, payload);
  } else {
    Serial.println("MQTT ISSUE!!");
  }
}

void transmit_mqtt_influx(const char * Field, float value) {
  char payload[75];
  snprintf(payload, 75, "climatic,host=MB-%08X %s=%f",ESP.getChipId(),Field, value);
  transmit_mqtt(TOPIC, "state", payload);
}

void transmit_mqtt_float(const char * Field, float value) {
  char payload[20];
  snprintf(payload, 20, "%f",value);
  transmit_mqtt(TOPIC, Field, payload);
}



void ShowClients()
{
  unsigned char number_client;
  struct station_info *stat_info;

  struct ip4_addr *IPaddress;
  IPAddress address;
  int cnt = 1;

  number_client = wifi_softap_get_station_num();
  stat_info = wifi_softap_get_station_info();

  Serial.print("Connected clients: ");
  Serial.println(number_client);

  while (stat_info != NULL)
  {
    IPaddress = &stat_info->ip;
    address = IPaddress->addr;

    Serial.print(cnt);
    Serial.print(": IP: ");
    Serial.print((address));
    Serial.print(" MAC: ");

    uint8_t *p = stat_info->bssid;
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X", p[0], p[1], p[2], p[3], p[4], p[5]);

    stat_info = STAILQ_NEXT(stat_info, next);
    cnt++;
    Serial.println();
  }
}


byte loper = 0x01;
unsigned long pwmStartMillis = 0;
int pwmCoolWrk = 0;
int pwmWarmWrk = 0;
void loop()
{
  switch (configuration.state)
  {
  case wifi_ready:
    // wifi connected to network. ready
    ArduinoOTA.handle();
    // mqtt loop. 
    client.loop();
    break;
  case wifi_ap_mode:
    // could not connect, waiting for new configuration.
    server.handleClient();
      // Handle DNS requests
    dnsServer.processNextRequest();
    break;

  default:
    break;
  }

 
  
  // Blinking led.
   currentMillis = millis();                  // get the current "time" (actually the number of milliseconds since the program started)

  if ((configuration.state == wifi_ap_mode) && ((currentMillis - startMillis) >= user_wifi.period) ) {
      ShowClients();
      startMillis = currentMillis;

  }

  if (((currentMillis - startMillis) >= user_wifi.period) && (configuration.state == wifi_ready) )// test whether the period has elapsed
  {
    // Serial.print("Cool: ");
    // Serial.print(pwmCoolWrk);
    // Serial.print(" ");
    // Serial.println(pwmCool);
    // Serial.print("Warm: ");
    // Serial.print(pwmWarmWrk);
    // Serial.print(" ");
    // Serial.println(pwmWarm);
    // Serial.println(encoderPos);
    // Serial.println(buttonState);
    // snprintf(value, 32, "%d", encoderPos );
    // transmit_mqtt("RotaryEncoder","encoderPos",value);
    // snprintf(value, 32, "%d", pwmCool );
    // transmit_mqtt("Cool","pwm",value);
    // snprintf(value, 32, "%d", pwmCoolWrk );
    // transmit_mqtt("TrueCool","pwm",value);    
    // snprintf(value, 32, "%d", pwmWarm );
    // transmit_mqtt("Warm","pwm",value);
    // snprintf(value, 32, "%d", pwmWarmWrk );
    // transmit_mqtt("TrueWarm","pwm",value);
    
    // Serial.print("Cool: ");
    // Serial.print(pwmCoolWrk);
    // Serial.print(" ");
    // Serial.println(pwmCool);
    // Serial.print("Warm: ");
    // Serial.print(pwmWarmWrk);
    // Serial.print(" ");
    // Serial.println(pwmWarm);
    // Serial.println(encoderPos);
    // Serial.println(buttonState);

    // digitalWrite(Heater_1, !digitalRead(Heater_1));  //if so, change the state of the LED.  Uses a neat trick to change the state
    
    startMillis = currentMillis; // IMPORTANT to save the start time of the current LED state.
  }
}

void handleCaptive()
{

  if (server.method() == HTTP_POST)
  {

    strncpy(user_wifi.ssid, server.arg("ssid").c_str(), sizeof(user_wifi.ssid));
    strncpy(user_wifi.password, server.arg("password").c_str(), sizeof(user_wifi.password));
    user_wifi.ssid[server.arg("ssid").length()] = user_wifi.password[server.arg("password").length()] = '\0';

    Serial.println(user_wifi.ssid);
    Serial.println(user_wifi.password);

    // load operational defaults
    strcpy(user_wifi.mqtt_server, default_mqtt_server);
   
    EEPROM.put(0, user_wifi);
    EEPROM.commit();
    Serial.println("Ready ");

    server.send(200, "text/html", "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title><style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}</style> </head> <body><main class='form-signin'> <h1>Wifi Setup</h1> <br/> <p>Your settings have been saved successfully!<br />Please restart the device.</p></main></body></html>");
  }
  else
  {
    // server.send(200, "text/html", "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title> <style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{cursor: pointer;border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1{text-align: center}</style> </head> <body><main class='form-signin'> <form action='/' method='post'> <h1 class=''>Wifi Setup</h1><br/><div class='form-floating'><label>SSID</label><input type='text' class='form-control' name='ssid'> </div><div class='form-floating'><br/><label>Password</label><input type='password' class='form-control' name='password'></div><br/><br/><button type='submit'>Save</button><p style='text-align: right'><a href='https://www.mbehrens.nl' style='color: #32C5FF'>MBehrens.nl</a></p></form></main> </body></html>");
     // Get list of available wifi networks
    String networks = "";
    int numNetworks = WiFi.scanNetworks();
    for (int i = 0; i < numNetworks; i++) {
      networks += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
    }
    // Generate HTML form for selecting wifi network and entering password
    String html = "<html><head><title>Captive Portal</title><style>body{background-color:#222;color:#fff;font-family:Arial,Helvetica,sans-serif}input[type=text],select{padding:12px;border-radius:5px;border:none;margin-bottom:10px}input[type=submit]{background-color:#4CAF50;color:#fff;padding:12px;border-radius:5px;border:none;margin-bottom:10px}input[type=submit]:hover{background-color:#3e8e41}</style></head><body><h1>Captive Portal</h1><form method=\"post\" action=\"/connect\"><label for=\"ssid\">Select Wifi Network:</label><br><select id=\"ssid\" name=\"ssid\">" + networks + "</select><br><label for=\"password\">Enter Wifi Password:</label><br><input type=\"text\" id=\"password\" name=\"password\"><br><input type=\"submit\" value=\"Connect\"></form></body></html>";
    // Send HTML form as response
    server.send(200, "text/html", html);
  }
}