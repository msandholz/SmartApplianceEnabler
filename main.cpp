#include <Arduino.h>
#include <Wifi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

///////////////////////////////////////////////////////////////////////////

#define VERSION 0.3
#define HEATING_PUMP_CONTROL 5
#define HEATING_PUMP_STATE 39
#define HEATING_PUMP_POWER 18

#define HEATING_ROD_CONTROL 8
#define HEATING_ROD_STATE 35
#define HEATING_ROD_POWER 19

const char* ssid="WLAN";
const char* password="74696325262072177928";
boolean debug = true; 

int heating_pump_power = 2000;
int heating_pump_control_state = LOW;
int heating_rod_power = 1480;
int heating_rod_control_state = LOW;

StaticJsonDocument<256> doc;
char json_msg[128];
AsyncWebServer server(80);

///////////////////////////////////////////////////////////////////////////

String processor(const String& var){
  Serial.println(var);
  if(var == "H_PUMP_CONTROL_STATE"){ 
    String pump_control_state = "off";
    if (heating_pump_control_state == HIGH) {pump_control_state = "on"; }
    return pump_control_state;  
    }
  if(var == "H_PUMP_STATE"){  return "STATUS_PUMP";  }
  if(var == "H_PUMP_POWER"){  return String(heating_pump_power);  }
  
  if(var == "H_ROD_CONTROL_STATE"){ 
    String rod_control_state = "off";
    if (heating_rod_control_state == HIGH) {rod_control_state = "on"; }
    return rod_control_state;  
    }
  if(var == "H_ROD_STATE"){  return "STATUS_ROD";  }
  if(var == "H_ROD_POWER"){  return String(heating_rod_power);  }
  if(var == "VERSION"){  return String(VERSION);  }
  
  return String();
}

void setup() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    pinMode(BUILTIN_LED,OUTPUT);
    pinMode(HEATING_PUMP_CONTROL,OUTPUT);
    digitalWrite(HEATING_PUMP_CONTROL, heating_pump_control_state);
    pinMode(HEATING_ROD_CONTROL,OUTPUT);
    digitalWrite(HEATING_ROD_CONTROL, heating_rod_control_state);

    pinMode(HEATING_PUMP_STATE, INPUT);
    pinMode(HEATING_PUMP_POWER, INPUT);
    pinMode(HEATING_ROD_STATE, INPUT);
    pinMode(HEATING_ROD_POWER, INPUT);

    if(debug){
        Serial.begin(115200);
        Serial.println("");
    }
    //Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(50);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(50);
        digitalWrite(LED_BUILTIN, LOW);
    }
    
    if(debug){
        Serial.println("WiFi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());    
    }
    
    if (SPIFFS.begin()){
        if(debug) {
            Serial.print("SPIFFS mounted correctly (");
            Serial.println(String(SPIFFS.usedBytes()) + " of " + String(SPIFFS.totalBytes()) + " bytes used)!");
        }
    }else{
        if(debug){Serial.println("!An error occurred during SPIFFS mounting");}
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        if(debug){Serial.println("Request Main page");}
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style.css","text/css");
    });

    server.on("/heating_pump/on", HTTP_GET, [](AsyncWebServerRequest *request){
        heating_pump_control_state = HIGH;
        digitalWrite(HEATING_PUMP_CONTROL, heating_pump_control_state);
        request->send(200, "text/html", "on");
        if(debug){Serial.println("/heating_pump/on");}
    });

    server.on("/heating_pump/off", HTTP_GET, [](AsyncWebServerRequest *request){
        heating_pump_control_state = LOW;
        digitalWrite(HEATING_PUMP_CONTROL, heating_pump_control_state);
        if(debug){Serial.println("/heating_pump/off");}
        request->send(200, "text/html", "off");
    });

    server.on("/heating_pump/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
        if (heating_pump_control_state == LOW) {
            heating_pump_control_state = HIGH;
        } else {
            heating_pump_control_state = LOW;
        }
        digitalWrite(HEATING_PUMP_CONTROL, heating_pump_control_state);
        if(debug){Serial.println("/heating_pump/toggle [" + String(heating_pump_control_state) + "]");}
        request->redirect("/");
    });

    server.on("/heating_pump", HTTP_GET, [](AsyncWebServerRequest *request){       
        DynamicJsonDocument doc(512);
        doc["control"] = heating_pump_control_state;
        doc["status"] = digitalRead(HEATING_PUMP_STATE);
        doc["power"] = heating_pump_power;
      
        String buf;
        serializeJson(doc, buf);
        request->send(200, "application/json", buf);
    });


    server.on("/heating_rod/on", HTTP_GET, [](AsyncWebServerRequest *request){
        heating_rod_control_state = HIGH;
        digitalWrite(HEATING_PUMP_CONTROL, heating_rod_control_state);
        if(debug){Serial.println("/heating_rod/off");}
        request->send(200, "text/html", "on");
    });

    server.on("/heating_rod/off", HTTP_GET, [](AsyncWebServerRequest *request){      
        heating_rod_control_state = HIGH;
        digitalWrite(HEATING_ROD_CONTROL, heating_rod_control_state);
        if(debug){Serial.println("/heating_rod/off");}
        request->send(200, "text/html", "off");
    });

    server.on("/heating_rod/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
        if (heating_rod_control_state == LOW) {
            heating_rod_control_state = HIGH;
        } else {
            heating_rod_control_state = LOW;
        }
        digitalWrite(HEATING_ROD_CONTROL, heating_rod_control_state);
        if(debug){Serial.println("/heating_rod/toggle [" + String(heating_rod_control_state) + "]");}
         request->redirect("/");
    });

    server.on("/heating_rod", HTTP_GET, [](AsyncWebServerRequest *request){
        DynamicJsonDocument doc(512);
        doc["control"] = heating_rod_control_state;
        doc["status"] = digitalRead(HEATING_ROD_STATE);
        doc["power"] = heating_rod_power;
      
        String buf;
        serializeJson(doc, buf);
        request->send(200, "application/json", buf);
    });


    AsyncElegantOTA.begin(&server);
    server.begin();
    if(debug){Serial.println("WebServer started!");}
}

void loop() {

}

