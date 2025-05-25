#include <SparkFun_VL53L5CX_Library.h>  // VL53L5CX ToF sensor
#include <WiFi.h>                      // Wi‑Fi
#include <WiFiClientSecure.h>          // HTTPS client
#include <Wire.h>                      // I2C

// — Wi‑Fi credentials —
const char* ssid = "";

// — Firebase settings —
const char* host           = "";
const char* firebaseSecret = "";

// — LiDAR settings —
#define imageWidth 8
int16_t threshold = 2000;             // mm
int16_t cell_array[imageWidth][imageWidth];

SparkFun_VL53L5CX sensorLR;
VL53L5CX_ResultsData measurementData;

// forward declarations
void connectToWiFi();
void sendToFirebase(int motionStatus);
void printGrid();

void setup()
{
  Serial.begin(115200);
  connectToWiFi();

  Wire.begin(21, 22);
  Wire.setClock(400000);

  if (!sensorLR.begin())
  {
    Serial.println("VL53L5CX failed to initialize");
    while (1);
  }

  sensorLR.setResolution(imageWidth * imageWidth); // 8×8 grid
  sensorLR.setRangingFrequency(7);                // 7 Hz
  sensorLR.startRanging();
}

void loop()
{
  int motion = 0;

  // 1) Build the grid array, no Serial prints here
  if (sensorLR.isDataReady() && sensorLR.getRangingData(&measurementData))
  {
    for (int y = 0; y < imageWidth; y++)
    {
      for (int x = 0; x < imageWidth; x++)
      {
        int idx = y * imageWidth + x;
        if (measurementData.nb_target_detected[idx] > 0)
          cell_array[x][y] = measurementData.distance_mm[idx];
        else
          cell_array[x][y] = -1;
      }
    }

    // 2) Motion detection using row1, row2 logic
    int leftCounter = 0;
    int rightCounter = 0;

    for(int r = 0; r <= imageWidth - 1; r++)
    {
    // Check right side: columns imageWidth-1 and imageWidth-2
      for(int c = imageWidth - 1; c >= imageWidth - 2; --c)
      {
        int16_t d1 = cell_array[r][c];
        int16_t d2 = cell_array[r][c];
        if ((d1 > 0 && d1 < threshold) || (d2 > 0 && d2 < threshold))
        {
          motion = 1;  // motion left→right
          leftCounter++;
        }
      }
    }

    if(leftCounter >= 8 && motion != 0)
    {
      //printGrid();
      //sendToFirebase(motion);
      Serial.println(motion);
      return;
    }

    for(int r = 0; r <= imageWidth - 1; r++)
    {
    // If no right motion, check left side: columns 0 and 1
      for (int c = 0; c <= 1; ++c)
      {
        int16_t d1 = cell_array[r][c];
        int16_t d2 = cell_array[r][c];
        if ((d1 > 0 && d1 < threshold) || (d2 > 0 && d2 < threshold))
        {
          motion = -1; // motion right→left
          rightCounter++;
        }
      }
    }

    if(rightCounter >= 8 && motion != 0)
    {
      Serial.println(motion);
    } 
  }
}


// — Wi‑Fi helper —
void connectToWiFi()
{
  Serial.print("Connecting to Wi‑Fi");
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWi‑Fi connected");
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

  unsigned long ts      = millis();
  String        payload = "{\"motion_status\":" + String(motionStatus)
                        + ",\"time_stamp\":\"" + String(ts) + "\"}";
  String        url     = "/1b/LiDAR.json?auth=" + String(firebaseSecret);

  client.print("PUT " + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " + payload.length() + "\r\n\r\n" +
               payload);

  // Skip headers
  while (client.connected() && client.readStringUntil('\n') != "\r") {}
  // Print response
  Serial.println("Response:\n" + client.readString());
  client.stop();
}