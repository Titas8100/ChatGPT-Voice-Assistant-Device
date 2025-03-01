#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Audio.h"

#define uart_en 15
#define RXp2 16
#define TXp2 17
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

const char* ssid = "OnePlus 7T";
const char* password = "Sayan@10";
const char* chatgpt_token = "";
const char* temperature = "0";
const char* max_tokens = "20";

Audio audio;
String Question;

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXp2,TXp2);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();


  while (!Serial);

  // wait for WiFi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(250);
  audio.connecttospeech("Starting ", "en"); // Google TTS
}

void loop() {
  Serial.print("Ask your Question : ");
  
  //while (!Serial.available()) {
    //audio.loop();
  //}
  while(!Serial2.available())
  {
    audio.loop();
  }
  //while (Serial.available()) {  //...........................asking question on Serial Monitor
    //char add = Serial.read();
    //Question = Question + add;
    //delay(1);
  //}

  if (Serial2.available()){
    Question = Serial2.readString();
    Serial.println(Question);
    //audio.connecttospeech(Answer.c_str(), "en");
  }
  //audio.loop();

  int len = Question.length();
  Question = Question.substring(0, (len - 1));
  Question = "\"" + Question + "\"";
  Serial.println(Question);

  DynamicJsonDocument jsonDocument(200);
  jsonDocument["model"] = "gpt-3.5-turbo-instruct";
  jsonDocument["prompt"] = Question;
  jsonDocument["temperature"] = atof(temperature);
  jsonDocument["max_tokens"] = atoi(max_tokens);  // Convert to integer


  String payload;
  serializeJson(jsonDocument, payload);

  Serial.print("Payload: ");
  Serial.println(payload);

  HTTPClient https;

  if (https.begin("https://api.openai.com/v1/completions")) {  // HTTPS
    https.addHeader("Content-Type", "application/json");
    String token_key = String("Bearer ") + chatgpt_token;
    https.addHeader("Authorization", token_key);

    // start connection and send HTTP header
    int httpCode = https.POST(payload);

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String payload = https.getString();

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      String Answer = doc["choices"][0]["text"];
      Answer = Answer.substring(2);
      Serial.print("Answer : "); Serial.println(Answer);
      audio.connecttospeech(Answer.c_str(), "en");
    }
    else {
      Serial.printf("[HTTPS] POST... failed, HTTP response code: %d\n", httpCode);
      String payload = https.getString();
      Serial.printf("[HTTPS] Response payload: %s\n", payload.c_str());
    }
    
    https.end();
  }
  else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }

  Question = "";
}


void audio_info(const char *info) {
  Serial.print("audio_info: "); Serial.println(info);
}
