#include <SparkFun_VL53L5CX_Library.h>  // VL53L5CX ToF sensor
#include <WiFi.h>                      // Wi‑Fi
#include <WiFiClientSecure.h>          // HTTPS client
#include <Wire.h>                      // I2C

// — Wi‑Fi credentials —
const char* ssid = "";

// — Firebase settings —
const char* host          = "";
const char* firebaseSecret = "";

// — LiDAR settings —
#define IMAGE_WIDTH           8
#define DETECTION_THRESHOLD   700    // mm: inside this distance counts as “in FOV”
#define MOVEMENT_THRESHOLD_MM 200    // mm: must move this far to count entry/exit

int16_t cell_array[IMAGE_WIDTH][IMAGE_WIDTH];
SparkFun_VL53L5CX sensor;
VL53L5CX_ResultsData measurementData;

// State for entry/exit logic
bool inFOV       = false;
float startDist  = 0;


void setup() 
{
  Serial.begin(115200);

  // — Wi‑Fi connect —
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(200);
    Serial.print(".");
  }
  Serial.println("\nWi‑Fi connected");

  // — I2C + sensor init —
  Wire.begin(21, 22);
  Wire.setClock(400000);
  if (!sensor.begin()) {
    Serial.println("VL53L5CX init failed");
    while (1);
  }
  sensor.setResolution(IMAGE_WIDTH * IMAGE_WIDTH);
  sensor.setRangingFrequency(15); // Hz
  sensor.startRanging();
}


void loop() 
{
  // 1) Grab a frame
  if (!sensor.isDataReady() || !sensor.getRangingData(&measurementData))
    return;

  // 2) Build grid & compute average distance of all valid cells
  long sumDist = 0;
  int  validCnt = 0;
  for (int y = 0; y < IMAGE_WIDTH; ++y) 
  {
    for (int x = 0; x < IMAGE_WIDTH; ++x) 
    {
      int idx = y * IMAGE_WIDTH + x;
      if (measurementData.nb_target_detected[idx] > 0) 
      {
        int16_t d = measurementData.distance_mm[idx];
        cell_array[x][y] = d;
        if (d < DETECTION_THRESHOLD) 
        {
          sumDist  += d;
          validCnt += 1;
        }
      } else {
        cell_array[x][y] = -1;
      }
    }
  }

  // 3) Determine entry/exit
  if (validCnt > 0) 
  {
    // Someone is in FOV
    float avgDist = float(sumDist) / validCnt;

    if (!inFOV) 
    {
      // 3a) They just appeared
      inFOV = true;
      startDist = avgDist;
    } else 
    {
      // 3b) They were here—check movement
      float delta = avgDist - startDist;

      if (delta > MOVEMENT_THRESHOLD_MM) 
      {
        // moved away => entry
        sendToFirebase(+1);
        Serial.println("ENTRY detected");
        inFOV = false;
      }
      else if (delta < -MOVEMENT_THRESHOLD_MM) 
      {
        // moved closer => exit
        sendToFirebase(-1);
        Serial.println("EXIT detected");
        inFOV = false;
      }
    }
  }
  else 
  {
    // 4) No one in FOV => reset state
    inFOV = false;
  }
}


void sendToFirebase(int motionStatus) 
{
  WiFiClientSecure client;
  client.setInsecure();
  if (!client.connect(host, 443)) 
  {
    Serial.println("Firebase connection failed");
    return;
  }

  unsigned long timestamp = millis();
  String body = "{\"motion_status\":" + String(motionStatus)
              + ",\"time_stamp\":\"" + String(timestamp) + "\"}";

  String url = "/3a/LiDAR.json?auth=" + String(firebaseSecret);
  client.print("PUT " + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " + body.length() + "\r\n\r\n" +
               body);

  // skip headers
  while (client.connected() && client.readStringUntil('\n') != "\r") {}
  String resp = client.readString();
  Serial.println("Firebase ↩ " + resp);
  client.stop();
}