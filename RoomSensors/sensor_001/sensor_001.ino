/*
 * Room sensor with display for home automation project.
 * Wakes up only to send data. Otherwise stays in deep sleep to prevent heat up sensors
 * inside same enclosure.
 * Sends data to SQL server with httpPOST request once in 10 minutes.
 * 
 * by: Mikko Nopanen
 * latest update: 2020-12-19
 */
 
#ifdef ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#endif

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_BMP280 bmp;

double temperature = 0;
double pressure = 0;
double humidity = 0;

const char* ssid     = "<your ssid>";
const char* password = "<your password>";

const char* serverName = "<your server ip>";

String apiKeyValue = "<your api key>";

String sensorName = "001";
String sensorLocation = "Lobby";

void setup() {

  // SERIAL
  Serial.begin(9600);
  Serial.setTimeout(2000);

  // SENSORS
  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  temperature = bmp.readTemperature();
  temperature = temperature - 0.5;
  pressure = bmp.readPressure() / 100;


  // DISPLAY
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(pressure, 0);
  display.print(" ppi");
  display.setCursor(64, 0);
  //display.print(humidity, 0);
  display.print("XX");
  display.print(" %");
  display.setTextSize(3);
  display.setCursor(10, 12);
  display.print(temperature, 1);
  display.setTextSize(1);
  display.print(" o ");
  display.setTextSize(2);
  display.print("C");
  display.display();

  // DATA TRANSMIT
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    http.begin(serverName);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "api_key=" + apiKeyValue
                             + "&sensor=" + sensorName
                             + "&location=" + sensorLocation
                             + "&temperature=" + temperature
                             + "&humidity=" + humidity
                             + "&pressure=" + pressure;

    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);

    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }

  // WAIT A MOMENT AND SLEEP 10 MINUTES
  delay(2000); // Pause for 2 seconds

  Serial.println("Nukun minuutin");

  ESP.deepSleep(597000000); // calibrated to 10 minutes interval

}

void loop() {
  // Nothing here
}
