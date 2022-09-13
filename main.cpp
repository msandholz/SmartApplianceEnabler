#include <Arduino.h>
#include <Wifi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ArduinoJson.h>

///////////////////////////////////////////////////////////////////////////

#define HEATING_PUMP_CONTROL 5
#define HEATING_PUMP_STATUS 39
#define HEATING_PUMP_POWER 5

#define HEATING_ROD_CONTROL 5
#define HEATING_ROD_STATUS 35
#define HEATING_ROD_POWER 5

const char* ssid="WLAN";
const char* password="74696325262072177928";
boolean debug = true; 

int heating_rod_power = 1480;
int heating_pump_power = 2000;

StaticJsonDocument<256> doc;
char json_msg[128];
AsyncWebServer server(80);

///////////////////////////////////////////////////////////////////////////

void setup() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    pinMode(BUILTIN_LED,OUTPUT);
    pinMode(HEATING_PUMP_CONTROL,OUTPUT);
    pinMode(HEATING_ROD_CONTROL,OUTPUT);

    pinMode(HEATING_PUMP_STATUS, INPUT);
    pinMode(HEATING_ROD_STATUS, INPUT);


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

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Herzlichen Glueckwunsch zum Geburtstag, Markus!");
    });

    server.on("/heating_pump/on", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(HEATING_PUMP_CONTROL, HIGH);
        request->send(200, "text/html", "on");
        if(debug){Serial.println("/heating_pump/on");}
    });

    server.on("/heating_pump/off", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(HEATING_PUMP_CONTROL, LOW);
        request->send(200, "text/html", "off");
        if(debug){Serial.println("/heating_pump/off");}
    });

    server.on("/heating_pump", HTTP_GET, [](AsyncWebServerRequest *request){       
        DynamicJsonDocument doc(512);
        doc["control"] = "TRUE";
        doc["status"] = digitalRead(HEATING_PUMP_STATUS);
        doc["power"] = heating_pump_power;
      
        String buf;
        serializeJson(doc, buf);
        request->send(200, "application/json", buf);
    });


    server.on("/heating_rod/on", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(HEATING_ROD_CONTROL, HIGH);
        request->send(200, "text/html", "on");
        if(debug){Serial.println("/heating_rod/off");}
    });

    server.on("/heating_rod/off", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(HEATING_ROD_CONTROL, LOW);
        request->send(200, "text/html", "off");
        if(debug){Serial.println("/heating_rod/off");}
    });

    server.on("/heating_rod", HTTP_GET, [](AsyncWebServerRequest *request){
        DynamicJsonDocument doc(512);
        doc["control"] = "FALSE";
        doc["status"] = digitalRead(HEATING_ROD_STATUS);
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
