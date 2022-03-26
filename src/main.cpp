#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ConfigPortal8266.h>
#include <DHTesp.h>

char*               ssid_pfix = (char*)"CaptivePortal";
String              user_config_html = "";

void thermo();
void readDHT22();
void handleNotFound();

DHTesp dht;
int interval = 2000;
unsigned long lastDHTReadMillis = 0;
float humidity = 0;
float temperature = 0;

ESP8266WebServer server(80);

/*
 *  ConfigPortal library to extend and implement the WiFi connected IOT device
 *
 *  Yoonseok Hur
 *
 *  Usage Scenario:
 *  0. copy the example template in the README.md
 *  1. Modify the ssid_pfix to help distinquish your Captive Portal SSID
 *          char   ssid_pfix[];
 *  2. Modify user_config_html to guide and get the user config data through the Captive Portal
 *          String user_config_html;
 *  2. declare the user config variable before setup
 *  3. In the setup(), read the cfg["meta"]["your field"] and assign to your config variable
 *
 */

void setup() {
    Serial.begin(115200);

    loadConfig();
    // *** If no "config" is found or "config" is not "done", run configDevice ***
    if(!cfg.containsKey("config") || strcmp((const char*)cfg["config"], "done")) {
        configDevice();
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    dht.setup(14, DHTesp::DHT22);
    // main setup
    Serial.printf("\nIP address : "); Serial.println(WiFi.localIP());

    if (MDNS.begin("eunwifi")) {
        Serial.println("MDNS responder started");
    }

    server.on("/temp_hum", thermo);
  
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    MDNS.update();
    server.handleClient();
    readDHT22();
}

void thermo(){
  char mBuf[500];
  char tmplt[] = "<html><head><meta charset=\"utf-8\">"
  "<meta http-equiv='refresh' content='5'/>"
  "<title>온습도계</title></head>"
  "<body>"
  "<script></script>"
  "<center><p>"
  "<h1><p>온도 : %.2f"
  "<p>습도 : %.2f</h1>"
  "</center>"
  "</body></html>";
  sprintf(mBuf, tmplt, temperature, humidity);
  Serial.println("serving");
  server.send(200, "text/html", mBuf);
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  server.send(404, "text/plain", message);
}

void readDHT22(){
  unsigned long currentMillis = millis();

  if(currentMillis - lastDHTReadMillis >= interval){
    lastDHTReadMillis = currentMillis;

    humidity = dht.getHumidity();
    temperature = dht.getTemperature();
  }
}