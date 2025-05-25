#include "Adafruit_VL53L1X.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>

// Wi-Fi credentials
const char* ssid = "";

// Firebase settings
const char* host = "";  // without https://
const char* firebaseSecret = "";  // Firebase secret

#define THRESHOLD 1500
#define SDA_PIN 21
#define SCL_PIN 22
Adafruit_VL53L1X vl53;
int16_t distance;

void setup() 
{
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  // Connect to Wi-Fi
  connectToWiFi();
  vl53.begin(0x29, &Wire);
  vl53.startRanging();
  vl53.setTimingBudget(15);
}

void loop() 
{
  if (vl53.dataReady()) 
  {
    distance = vl53.distance();
    //Serial.println(distance);
    if(distance != -1 && distance < THRESHOLD)
    {
      sendToFirebase(-1);
    }
    vl53.clearInterrupt();
  }
}

void connectToWiFi() 
{
  if(WiFi.status() != WL_CONNECTED) 
  {
    WiFi.begin(ssid);
    unsigned long st=millis();
    while(WiFi.status()!=WL_CONNECTED && millis()-st<10000) vTaskDelay(pdMS_TO_TICKS(200));
  }
}

// — Firebase helper —
void sendToFirebase(int motionStatus)
{
  WiFiClientSecure client;
  client.setInsecure();
  if (!client.connect(host, 443))
  {
    Serial.println("Firebase connect failed");
    return;
  }

  unsigned long timestamp = millis();
  String token = String(timestamp);

  String url = "/3c/LiDAR.json?auth=" + String(firebaseSecret);
  String payload = "{\"motion_status\":" + String(motionStatus) + ",\"time_stamp\":\"" + token + "\"}";

  client.print(String("PUT ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " + payload.length() + "\r\n\r\n" +
               payload);

  
  // Skip headers
  while(client.connected())
    if (client.readStringUntil('\n') == "\r") break;

  String response = client.readString();
  Serial.println("Response:\n" + response);
  
  client.stop();
}