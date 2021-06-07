#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>

#define STASSID "Honda-ASUS"
#define STAPSK  "mobydick"

#define INT_MAX 2147483647
#define LATCH D1
#define CLOCK D2
#define DATA D0
#define OE1 D3
#define OE2 D4
#define OE3 D5
#define OE4 D6

#define PUMPS1 D7
#define PUMPS2 D8

byte leds = 0;
bool lightsOn = false;


ESP8266WebServer server(80);

char ptr[] = "<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <title>Document</title>\n"
"</head>\n"
"\n"
"<body>\n"
"    <h1 id=\"displayPin\" style=\"text-align:center;height:30px\"></h1>\n"
"    <div>\n"
"        <button id=\"toggleLightsButton\" type=\"button\"\n"
"            style=\"background-color:green;font-size:1.5em; width:30%;padding:0.5em;\">&gt</button>\n"
"    </div>\n"
"    <h1 id=\"lightsStatus\" style=\"text-align:center;color:maroon;\">OFF</h4>\n"
"\n"
"        <script>\n"
"            let toggleLightsButton = document.getElementById(\"toggleLightsButton\");\n"
"            let lightsStatus = document.getElementById(\"lightsStatus\");\n"
"\n"
"            function toggleLights() {\n"
"                let XHR = new XMLHttpRequest();\n"
"                XHR.onload = () => {\n"
"                    console.log(XHR.responseText)\n"
"                    let body = JSON.parse(XHR.responseText);\n"
"\n"
"                    if (XHR.status == 200) {\n"
"                        lightsStatus.innerHTML = body[\"response\"];\n"
"                    } else {\n"
"                        lightsStatus.innerHTML = \"Error\";\n"
"                    }\n"
"                }\n"
"                XHR.onerror = () => { alert(\"Server Error\") };\n"
"                XHR.open(\"GET\", \"http://\" + window.location.hostname + \"/toggleLights\");\n"
"                XHR.send();\n"
"            }\n"
"\n"
"            toggleLightsButton.onclick = toggleLights;\n"
"        </script>\n"
"</body>\n"
"\n"
"</html>\n";


void handleRoot() {
    server.send(200, "text/html", ptr);
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void turnLightsON() {
  leds = INT_MAX;
  updateShiftRegister();
}

void turnLightsOff() {
  leds = 0;
  updateShiftRegister();
}

void toggleLights() {
  if (lightsOn) {
    turnLightsOff();
    lightsOn = false;
  }
  else {
    turnLightsOn();
    lightsOn = true;
  }
}

void setup() 
{
  Serial.begin(9600);   // Initiate a serial communication
  pinMode(LATCH, OUTPUT);
  pinMode(DATA, OUTPUT);  
  pinMode(CLOCK, OUTPUT);
  pinMode(OE1, OUTPUT); 
  pinMode(OE2, OUTPUT); 
  pinMode(OE3, OUTPUT); 
  pinMode(OE4, OUTPUT);

  turnLightsOff();
  setBrightness(255);

  
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Connect Failed! Rebooting...");
    delay(200);
    ESP.restart();
  }
    ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  ArduinoOTA.setHostname("hondaesp");
  ArduinoOTA.setPassword("mobydick");
  
  // <-------- API -------->
  server.on("/", HTTP_GET, handleRoot);

  server.on("/toggleLights", HTTP_GET, toggleLights);
  
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.print("http://");
  Serial.println(WiFi.localIP());

}





void loop() 
{
  ArduinoOTA.handle();
  server.handleClient();
}

void updateShiftRegister()
{
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, leds);
   digitalWrite(latchPin, HIGH);
}

void setBrightness(byte brightness) // 0 to 255
{
  analogWrite(OE1, 255-brightness);
  analogWrite(OE2, 255-brightness);
  analogWrite(OE3, 255-brightness);
  analogWrite(OE4, 255-brightness);
}
