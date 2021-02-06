#include <M5Stack.h>
#include <WiFiMulti.h>
#include <YoutubeApi.h>
#include <WiFiClientSecure.h>

// Configure YOUR experience here
#define USE_LEDS 1 // comment if you dont want to use leds
#define USE_SOUND 1 // comment if you dont want to use auditive feedback
#define WIFI_SSID "" // your wifi ssid
#define WIFI_PASSWORD "" // yout wifi password
#define API_KEY "" // your google apps API Token
#define CHANNEL_ID "" // makes up the url of channel
#define CHANNEL_NAME "" // channel name being displayed


#ifdef USE_LEDS
  #include <Adafruit_NeoPixel.h>
  #define M5STACK_FIRE_NEO_NUM_LEDS 10
  #define M5STACK_FIRE_NEO_DATA_PIN 15
#endif
 
WiFiClientSecure client;
YoutubeApi api(API_KEY, client);
int api_mtbs = 120000; //time between api requests
long api_lasttime;   //last time api request has been done
long lastSubsCnt = 0;
long lastViewCnt = 0;
long lastVideoCnt = 0;
WiFiMulti wifiMulti;
#ifdef USE_LEDS
  Adafruit_NeoPixel pixels = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);
#endif


void setup() {
  // start M5Stack
  m5.begin();
  m5.lcd.setBrightness(100);
  m5.Lcd.setFont();
  m5.Lcd.fillScreen(BLACK);
  m5.Lcd.setCursor(10, 10);
  m5.Lcd.setTextColor(WHITE);
  m5.Lcd.setTextSize(2);
  #ifdef USE_LEDS
    pixels.begin();
  #endif
  
  // Connect to WIFI
  int count = 0;
  Serial.println("Connecting to wifi...");
  m5.Lcd.println("Connecting to WiFi...");
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  //wifiMulti.addAP("ssid2", "password");
  while(wifiMulti.run() != WL_CONNECTED) {
    if(count < 20) {
      delay(500);
      Serial.print(".");
      m5.Lcd.printf(".");
    }
  }
  
  // successfully connected to WIFI
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  m5.Lcd.println("Wifi connection successful with IP: ");
  m5.Lcd.println(WiFi.localIP());
  delay(500);
  m5.Lcd.fillScreen(BLACK);
  updateChannelStats();
}

void loop() {
  if (millis() > api_lasttime + api_mtbs)  {
    updateChannelStats();
    api_lasttime = millis();
  }
}

void updateChannelStats() {
  if(api.getChannelStatistics(CHANNEL_ID)) {
      updateDisplay();
      // Check if subscribers count changed
      if (lastSubsCnt != api.channelStats.subscriberCount) {
        playNotification(lastSubsCnt < api.channelStats.subscriberCount);
      }
      lastSubsCnt = api.channelStats.subscriberCount;
      
      // Check if views count changed
      if (lastViewCnt != api.channelStats.viewCount) {
        ledBlink(255, 255, 0, 5);
      }
      lastViewCnt = api.channelStats.viewCount;

      // Check if video count changed
      if (lastVideoCnt != api.channelStats.videoCount) {
        ledBlink(0, 0, 255, 5);
      }
      lastVideoCnt = api.channelStats.videoCount;
  }
}

void updateDisplay() {
  char charBuf[50];
  // clear screen
  m5.Lcd.fillScreen(BLACK);
  // display channel name
  m5.Lcd.setTextColor(WHITE);
  m5.Lcd.setCursor(5, 5);
  m5.Lcd.setTextSize(4);
  m5.Lcd.printf(CHANNEL_NAME);
  // display subscribers
  String strSubscribers = String(api.channelStats.subscriberCount);
  strSubscribers.toCharArray(charBuf, 50);
  printTitleValue(15, 45, "Subscribers", charBuf, RED);

  // display views
  String strViews = String(api.channelStats.viewCount);
  strViews.toCharArray(charBuf, 50);
  printTitleValue(15, 110, "Views", charBuf, YELLOW);

  // display video count
  String strVideos = String(api.channelStats.videoCount);
  strVideos.toCharArray(charBuf, 50);
  printTitleValue(15, 175, "Video Count", charBuf, BLUE);
}

void printTitleValue(int x, int y, const char* title, const char* value, int color) {
  m5.Lcd.setTextColor(color);
  m5.Lcd.setCursor(x, y);
  m5.Lcd.setTextSize(2);
  m5.Lcd.printf(title);
  m5.Lcd.setCursor(x, y + 20);
  m5.Lcd.setTextSize(5);
  m5.Lcd.printf(value);
}

void setAllLeds(int r, int g, int b) {
  #ifdef USE_LEDS
    for(int i = 0; i < M5STACK_FIRE_NEO_NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(r, g, b));
    }
    pixels.show();
  #endif
}

void ledBlink(int r, int g, int b, int cnt) {
  for(int i = 0; i < cnt; i++) {
    setAllLeds(r,g,b);
    delay(75);
    setAllLeds(0,0,0);
    delay(75);
  }
}

void playNotification(bool success) {
  if (success) {
    #ifdef USE_SOUND
      m5.Speaker.tone(294, 75);
      delay(75);
      m5.Speaker.tone(350, 75);
      delay(75);
      m5.Speaker.tone(441, 75);
      delay(75);
      m5.Speaker.mute();
    #endif
    ledBlink(0, 255, 0, 5);
  } else {
    #ifdef USE_SOUND
      m5.Speaker.tone(441, 75);
      delay(75);
      m5.Speaker.tone(350, 75);
      delay(75);
      m5.Speaker.tone(294, 75);
      delay(75);
      m5.Speaker.mute();
    #endif
    ledBlink(255, 0, 0, 5);
  }
}
