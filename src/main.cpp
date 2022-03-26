#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ConfigPortal8266.h>
#include <DHTesp.h>
#include <SSD1306.h>

SSD1306             display(0x3c, 4, 5, GEOMETRY_128_32);

char *ssid_pfix = (char *)"CaptivePortal";
String user_config_html = "";

void temp();
void humi();
void handleNotFound();

void readDHT22();

DHTesp dht;
int interval = 2000;
unsigned long lastDHTReadMillis = 0;
float humidity = 0;
float temperature = 0;

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

void setup()
{
  Serial.begin(115200);
  dht.setup(14, DHTesp::DHT22);
  delay(1000);

  loadConfig();
  // *** If no "config" is found or "config" is not "done", run configDevice ***
  if (!cfg.containsKey("config") || strcmp((const char *)cfg["config"], "done"))
  {
    configDevice();
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin((const char *)cfg["ssid"], (const char *)cfg["w_pw"]);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  dht.setup(14, DHTesp::DHT22);
  
  Serial.println(); Serial.println("Humidity \tTemperature ");

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);

  // main setup
  Serial.printf("\nIP address : ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("eunwifi"))
  {
    Serial.println("MDNS responder started");
  }

  webServer.on("/temp", temp);
  webServer.on("/humi", humi);
  webServer.onNotFound(handleNotFound);

  webServer.begin();
}

void readDHT22()
{
  unsigned long currentMillis = millis();

  if (currentMillis - lastDHTReadMillis >= interval)
  {
    lastDHTReadMillis = currentMillis;

    humidity = dht.getHumidity();
    temperature = dht.getTemperature();
  }
}

void temp()
{
  char mBuf[500];
  char tmplt[] = "<html><head><meta charset=\"utf-8\">"
                 "<meta http-equiv='refresh' content='5'/>"
                 "<title>온도계</title></head>"
                 "<body>"
                 "<script></script>"
                 "<center><p>"
                 "<h1><p>온도 : %.2f</h1>"
                 "</center>"
                 "</body></html>";
  sprintf(mBuf, tmplt, temperature);
  Serial.println("temperature serving");
  webServer.send(200, "text/html", mBuf);
}

void humi()
{
  char mBuf[500];
  char tmplt[] = "<html><head><meta charset=\"utf-8\">"
                 "<meta http-equiv='refresh' content='5'/>"
                 "<title>습도계</title></head>"
                 "<body>"
                 "<script></script>"
                 "<center><p>"
                 "<h1><p>습도 : %.2f</h1>"
                 "</center>"
                 "</body></html>";
  sprintf(mBuf, tmplt, humidity);
  Serial.println("humidity serving");
  webServer.send(200, "text/html", mBuf);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  webServer.send(404, "text/plain", message);
}

void loop()
{
  MDNS.update();
  readDHT22();
  webServer.handleClient();

  Serial.printf("%.1f\t %.1f\n", temperature, humidity);

  display.clear();
  display.drawString(0, 0, "Temp : " + String(temperature));
  display.drawString(0, 15, "Humi : " + String(humidity));
  display.display();

  delay(1000);
}
