#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <driver/i2s.h>
#include <arduinoFFT.h>
#include <math.h>
#include <map>

// — Wi‑Fi / Occupancy Sniffer Settings —
const char* ssid            = "";
const char* host            = "";
const char* firebaseSecret  = "";
#define RSSI_THRESHOLD         -70        // ignore weaker signals
#define REPORT_INTERVAL_MS   30000UL      // 30-second report window
#define CHANNEL_HOP_INTERVAL_MS 500UL     // hop every 0.5s
#define WIFI_CHANNEL_MAX         13       // channels 1–13

// — Microphone FFT Settings —
#define SAMPLES               1024
#define SAMPLING_FREQUENCY    44100
#define MIN_SPEECH_FREQ       100
#define MAX_SPEECH_FREQ      5000
#define I2S_WS                25
#define I2S_SD                33
#define I2S_SCK               32
#define I2S_PORT          I2S_NUM_0
#define RMS_THRESHOLD       100.0
#define AUDIO_WINDOW_MS    30000UL       // 30-second audio window

int16_t sBuffer[SAMPLES];
float   vReal[SAMPLES], vImag[SAMPLES];
ArduinoFFT<float> FFT(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);

// sliding-window of MAC → lastSeen timestamp
static std::map<String,unsigned long> macLastSeen;
volatile int   sniffCount = 0;
volatile bool  readySniff = false;
volatile float micAvg     = 0.0;
volatile bool  readyMic   = false;

static unsigned long lastSniffReport = 0;
static unsigned long lastChannelHop  = 0;
static int           currentChannel = 1;

// forward declarations
typedef struct {
  uint16_t frame_ctrl; uint16_t duration_id;
  uint8_t addr1[6]; uint8_t addr2[6]; uint8_t addr3[6]; uint16_t seq_ctrl;
} hdr_t;

void sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type);
void wifiSnifferTask(void* pvParameters);
void audioTask(void* pvParameters);
void firebaseTask(void* pvParameters);
void connectToWiFi();
void sendToFirebaseW(int count);
void sendToFirebaseM(float averageDb);
void i2s_install();
void i2s_setpin();

void setup() {
  Serial.begin(115200);
  delay(200);
  // Mic init
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);
  // WiFi sniffer init
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect(true);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(sniffer_callback);
  esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
  lastSniffReport = millis();
  lastChannelHop  = millis();
  // Tasks
  xTaskCreatePinnedToCore(wifiSnifferTask, "WIFI", 4096, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(audioTask,       "AUDIO",4096, nullptr, 1, nullptr, 1);
  xTaskCreatePinnedToCore(firebaseTask,    "HTTP", 8192, nullptr, 1, nullptr, 1);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}

void sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (type!=WIFI_PKT_MGMT && type!=WIFI_PKT_DATA) return;
  auto* pkt=(wifi_promiscuous_pkt_t*)buf;
  if(!pkt||!pkt->payload) return;
  if(pkt->rx_ctrl.rssi<RSSI_THRESHOLD) return;
  auto* h=(hdr_t*)pkt->payload;
  char mac[18];
  snprintf(mac,sizeof(mac),"%02X:%02X:%02X:%02X:%02X:%02X",
           h->addr2[0],h->addr2[1],h->addr2[2],h->addr2[3],h->addr2[4],h->addr2[5]);
  macLastSeen[String(mac)] = millis();
}

void wifiSnifferTask(void* pv) {
  (void)pv;
  for(;;) {
    unsigned long now=millis();
    // hop channel
    if(now-lastChannelHop>=CHANNEL_HOP_INTERVAL_MS) {
      currentChannel=(currentChannel%WIFI_CHANNEL_MAX)+1;
      esp_wifi_set_channel(currentChannel,WIFI_SECOND_CHAN_NONE);
      lastChannelHop=now;
    }
    // report
    if(now-lastSniffReport>=REPORT_INTERVAL_MS) {
      // expire old
      for(auto it=macLastSeen.begin();it!=macLastSeen.end();) {
        if(now-it->second>REPORT_INTERVAL_MS) it=macLastSeen.erase(it);
        else ++it;
      }
      sniffCount=macLastSeen.size();
      readySniff=true;
      lastSniffReport=now;
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void audioTask(void* pv) {
  (void)pv;
  unsigned long startTs=0;
  float dbSum=0;int dbCnt=0;
  for(;;) {
    size_t br;
    if(i2s_read(I2S_PORT,sBuffer,sizeof(sBuffer),&br,portMAX_DELAY)==ESP_OK&&br) {
      for(int i=0;i<SAMPLES;++i){vReal[i]=sBuffer[i];vImag[i]=0;}
      FFT.windowing(vReal,SAMPLES,FFT_WIN_TYP_HAMMING,FFT_FORWARD);
      FFT.compute(vReal,vImag,SAMPLES,FFT_FORWARD);
      FFT.complexToMagnitude(vReal,vImag,SAMPLES);
      double sum=0;int cnt=0;
      int sb=MIN_SPEECH_FREQ*SAMPLES/SAMPLING_FREQUENCY;
      int eb=MAX_SPEECH_FREQ*SAMPLES/SAMPLING_FREQUENCY;
      for(int i=sb;i<eb&&i<SAMPLES/2;++i){sum+=vReal[i]*vReal[i];++cnt;}
      float rms=cnt?sqrt(sum/cnt):0;
      float db=20*log10(rms+1);
      if(rms>RMS_THRESHOLD) {
        if(!startTs) startTs=millis();
        dbSum+=db;dbCnt++;
        if(millis()-startTs>=AUDIO_WINDOW_MS){
          micAvg=dbCnt?dbSum/dbCnt:0;
          dbSum=0;dbCnt=0;startTs=millis();
          readyMic=true;
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void firebaseTask(void* pv) {
  (void)pv;
  for(;;) {
    if(readySniff) {
      esp_wifi_set_promiscuous(false);
      connectToWiFi();
      WiFiClientSecure c; c.setInsecure();
      if(c.connect(host,443)){
        String url=String("/3d/WiFi_Sniffing.json?auth=")+firebaseSecret;
        String pld=String(sniffCount);
        c.printf("PUT %s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s",
                 url.c_str(),host,pld.length(),pld.c_str());
        while(c.connected()&&c.readStringUntil('\n')!="\r");
        c.stop();
      }
      esp_wifi_set_promiscuous(true);
      esp_wifi_set_promiscuous_rx_cb(sniffer_callback);
      esp_wifi_set_channel(currentChannel,WIFI_SECOND_CHAN_NONE);
      readySniff=false;
    }
    if(readyMic) {
      connectToWiFi();
      WiFiClientSecure c; c.setInsecure();
      if(c.connect(host,443)){
        String url=String("/3d/Microphone.json?auth=")+firebaseSecret;
        String pld=String(micAvg);
        c.printf("PUT %s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s",
                 url.c_str(),host,pld.length(),pld.c_str());
        while(c.connected()&&c.readStringUntil('\n')!="\r");
        c.stop();
      }
      readyMic=false;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void connectToWiFi() {
  if(WiFi.status()!=WL_CONNECTED) {
    WiFi.begin(ssid);
    unsigned long st=millis();
    while(WiFi.status()!=WL_CONNECTED && millis()-st<10000) vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void i2s_install(){
  i2s_config_t cfg={
    .mode=(i2s_mode_t)(I2S_MODE_MASTER|I2S_MODE_RX),
    .sample_rate=SAMPLING_FREQUENCY,
    .bits_per_sample=I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format=I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format=I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags=0,
    .dma_buf_count=8,
    .dma_buf_len=SAMPLES,
    .use_apll=false
  };
  i2s_driver_install(I2S_PORT,&cfg,0,nullptr);
}
void i2s_setpin(){
  i2s_pin_config_t p={
    .bck_io_num=I2S_SCK,.ws_io_num=I2S_WS,
    .data_out_num=-1,.data_in_num=I2S_SD
  };
  i2s_set_pin(I2S_PORT,&p);
}