#include <ESP8266WiFi.h>

#ifndef _WEBPAGE
#define _WEBPAGE WEBPAGE

String html_base_style = " \
<style> \
  body{ \
    background-color:#222; \
    color:#fff; \
    font-family: \
    Arial,Helvetica,sans-serif \
  } \
  input[type=text],select{ \
    padding:12px; \
    border-radius:5px; \
    border:none; \
    margin-bottom:10px \
  } \
  input[type=submit]{ \
    background-color:#4CAF50; \
    color:#fff; \
    padding:12px; \
    border-radius:5px; \
    border:none; \
    margin-bottom:10px \
  } \
  input[type=submit]:hover{ \
    background-color:#3e8e41 \
  } \
</style>";

String html_text_input(String name, String Title, String value) {
  return "\
  <label for=\""+name+"\">"+Title+":</label><br> \
  <input type=\"text\" id=\""+name+"\" name=\""+name+"\" value=\""+value+"\"><br> \
  ";
}

String webpage_CaptivePortal(String Title, String networks, String mqttIp) {
  return "\
      <!DOCTYPE html> \
      <html> \
        <head> \
          <title>"+ Title +"</title>"
          + html_base_style +
        "</head> \
        <body> \
          <h1>"+ Title +"</h1> \
          <form method=\"post\" action=\"/connect\"> \
            <label for=\"ssid\">Select Wifi Network:</label><br> \
            <select id=\"ssid\" name=\"ssid\">" 
            + networks + 
            "</select><br> \
            <label for=\"password\">Enter Wifi Password:</label><br> \
            <input type=\"text\" id=\"password\" name=\"password\"><br>"
            + html_text_input("mqtt","Mqtt Broker", mqttIp) +
            "<input type=\"submit\" value=\"Connect\"> \
          </form> \
        </body> \
      </html>";
}

String webpage_Setup_Ready(String Title, String Network,String Password, String mqttIp) {
  return "\
      <!DOCTYPE html> \
      <html> \
        <head> \
          <title>"+ Title +"</title>"
          + html_base_style +
        "</head> \
        <body> \
          <h1>"+ Title +"</h1> \
          <p> \
          SSID:"+Network+" <br>\
          Password: "+Password+" <br>\
          MqttBrokerIP: "+mqttIp+"<br>\
          </p> \
        </body> \
      </html>";
}

#endif


extern String webpage_Setup_Ready(String Title, String Network,String Password, String mqttIp);
extern String webpage_CaptivePortal(String Title, String networks, String mqttIp) ;
extern String html_text_input(String name, String Title, String value);