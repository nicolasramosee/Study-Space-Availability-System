
#include <Wire.h>
#include <SparkFun_VL53L5CX_Library.h> 
#include <WiFi.h>                    
#include <WiFiClientSecure.h>         

// — Wi‑Fi credentials —
const char* ssid = "";

SparkFun_VL53L5CX myImager;
VL53L5CX_ResultsData measurementData;

// — Firebase settings —
const char* host = "";
const char* firebaseSecret = "";

#define IMAGE_WIDTH 8

void setup()
{
  Serial.begin(115200);

  connectToWiFi();

  Wire.begin(21, 22);
  Wire.setClock(400000);

  Serial.println("Initializing sensor board. Please wait up to 10s...");
  if (!myImager.begin())
  {
    Serial.println(F("Sensor not found – check wiring. Halting."));
    while (1);
  }

  // 8×8 grid
  myImager.setResolution(IMAGE_WIDTH * IMAGE_WIDTH);
  myImager.setRangingFrequency(15); // 15 Hz
  myImager.startRanging();
}


void loop()
{
  int32_t validSum = 0;
  int validCount = 0;

  if(myImager.isDataReady() && myImager.getRangingData(&measurementData))
  {
    // Print 8×8 grid
    for (int y = 0; y < IMAGE_WIDTH; y++)
    {
      for (int x = 0; x < IMAGE_WIDTH; x++)
      {
        int idx = y * IMAGE_WIDTH + x;
        uint8_t  nb = measurementData.nb_target_detected[idx];
        uint16_t d  = measurementData.distance_mm[idx];

        Serial.print("\t");
        if (nb > 0)
        {
          // At least one target → valid distance
          Serial.print(d);
          validSum  += d;
          validCount++;
        }
        else
        {
          // No target detected → “empty”
          Serial.print(-1);
        }
      }
      Serial.println();
    }
    Serial.println();
  }
  // Average of only the valid zones
  if (validCount > 0)
  {
    Serial.print("Avg (valid only): ");
    Serial.println(validSum / validCount);
    sendToFirebase(validSum / validCount);
  }
  else
  {
    Serial.println("Avg: no valid measurements");
  }
  delay(1000);
}


// — Wi‑Fi helper —
void connectToWiFi() 
{
  if(WiFi.status() != WL_CONNECTED) 
  {
    WiFi.begin(ssid);
    unsigned long st=millis();
    while(WiFi.status()!=WL_CONNECTED && millis()-st<10000) vTaskDelay(pdMS_TO_TICKS(200));
  }
}


void sendToFirebase(int avgDistance)
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

  String url = "/Calibration/LiDAR.json?auth=" + String(firebaseSecret);
  String payload = "{\"avg_distance\":" + String(avgDistance) + ",\"token\":\"" + token + "\"}";

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