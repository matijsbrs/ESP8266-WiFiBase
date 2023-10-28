/**
 * The settings structure for easy storing of data.
*/
#ifndef _SETTINGS
#define _SETTINGS SETTINGS

    const char* applicationUUID = "012345678";
    const char* default_mqtt_server = "192.168.2.201";
    const char* default_mqtt_topic = "climate/controller";
    const char* Version = "V0.0.1";


    struct settings
    {
    char ssid[32];         // the SSID of the netwerk
    char password[32];     // the WifiPassword
    char energy_topic[32]; // the path to the topic on which the current wattage is found.
    char mqtt_server[32];  // Addres of the MQTT server
    unsigned long period;

    };



    
#endif