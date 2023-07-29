
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>  
#include <ArduinoOTA.h>

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

const uint16_t PixelCount = 120;
const uint8_t PixelPin = 3;

//WS2813
//NeoPixelBus<NeoGrbFeature, NeoEsp8266DmaWs2813Method> strip(PixelCount, PixelPin);
//WS2812B
NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> strip(PixelCount, PixelPin); 

#define PROTOCOL_PORT 6454
#define MAX_BUFFER_PROTOCOL 512
#define PROTOCOL_DATA_START 18
uint8_t packetBuffer[MAX_BUFFER_PROTOCOL];
uint8_t dataPacket;
uint8_t* data; 
int pixelIndex;

// Network Settings
char ssid[] = "";  
char pass[] = ""; 
     
WiFiUDP udp;

void setup()
{

  Serial.begin(115200);
  delay(random(3000, 7000)); // random Delay for connection start

  IPAddress ip(2, 0, 0, 137);
  IPAddress gateway(2, 0, 0, 0);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, pass);
  WiFi.mode(WIFI_STA);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(random(200,500));
  }
  
  udp.begin(PROTOCOL_PORT);
  strip.Begin();

  //let first pixel glow white to indicate connection
  strip.SetPixelColor(0, RgbColor (100, 100, 100));  
  strip.Show();

  
  //////////// OTA Stuff
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("LEDStripe");

  // No authentication by default
  ArduinoOTA.setPassword("********");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.end();
  /////////////
  
}

void loop()
{ 
    dataPacket = udp.parsePacket();
    if(dataPacket) {
     udp.read(packetBuffer, MAX_BUFFER_PROTOCOL);
     data = packetBuffer + PROTOCOL_DATA_START;
     pixelIndex = 0;
     for (int i = 0; i < PixelCount; i++) {
      strip.SetPixelColor(i, RgbColor (data[pixelIndex], data[(pixelIndex + 1)], data[(pixelIndex + 2)]));
      pixelIndex+=3;
     }   
     strip.Show();
  }
  /////////OTA Stuff
  ArduinoOTA.handle();
  ////////
}
