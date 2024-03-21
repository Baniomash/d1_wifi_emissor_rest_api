#include <Arduino.h>
#include <MPU.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>


#define INTERRUPTION_PIN 5 // D6
#define SDA_PIN 0 // D4
#define SCL_PIN 2 // D5

#define NEUTRAL 0
#define UP 1
#define RIGHT 2
#define DOWN 3
#define LEFT 4
#define SLEEP 5

// WiFi configs
const char* ssid = "FAMILIA MEDEIROS";
const char* password = "sl23jo316";

// String api = "http://10.0.0.104:3000/makeMove";
String api;


MPU motionSensor;
WiFiClient espClient;
HTTPClient http;

// void wake()
// {
// //  sleep_disable();
//   esp_default_wake_deep_sleep();
// }

String sendHttpPost(unsigned short int movementsSequence){
  http.begin(espClient, api);
  http.addHeader("Content-Type", "text/plain");
  DynamicJsonDocument doc(512);
  String buf;
  doc["movement"] = movementsSequence;
  serializeJson(doc, buf);
  int httpCode = http.POST(buf);
  String payload = http.getString();
  Serial.println(httpCode);
  http.end();
  return payload;
}

bool getApi(IPAddress broadcast){
  if (http.begin(espClient, "http://" + broadcast.toString() + ":3000/makeMove")) {
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
          api = "http://" + payload + ":3000/makeMove";
          http.end();
          return true;
        } else {
          Serial.print("Erro HTTP: ");
          Serial.println(httpCode);
          http.end();
          return false;
        }
    } else {
      Serial.println("Falha ao iniciar a solicitação HTTP");
      return false;
    }
  }
  return false;
}

void sleep()
{
//  attachInterrupt(digitalPinToInterrupt(INTERRUPTION_PIN), wake, HIGH);

//  esp_deep_sleep_enable_gpio_wakeup(1ULL << INTERRUPTION_PIN,ESP_GPIO_WAKEUP_GPIO_HIGH);
//  esp_deep_sleep_start();
}

unsigned char verifyMovement()
{
  short int xAxis = 0;
  short int yAxis = 0;
  short int zAxis = 0;

  motionSensor.readAccelerometer(&xAxis, &yAxis, &zAxis);
  
  if (yAxis < -5000)
  {
    Serial.println("Cima");
    return UP;
  }
  else if (zAxis > 5000)
  {
    Serial.println("Direita");
    return RIGHT;
  }
  else if (yAxis > 5000)
  {
    if (yAxis < 15000)
    {
      Serial.println("Baixo");
      return DOWN;
    }
    else
    {
      Serial.println("Sleep!");
      return SLEEP;
    }
  }
  else if (zAxis < -5000)
  {
    Serial.println("Esquerda");
    return LEFT;
  }
  else
  {
    return NEUTRAL;
  }
}

void wait(unsigned long milliseconds)
{
  unsigned long currentTime = millis();
  unsigned long previousTime = millis();

  while (currentTime - previousTime <= milliseconds)
  {
    currentTime = millis();
  }
}

void setupWifi(){
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  // Seting the ESP8266 as a client not an AP 
  WiFi.mode(WIFI_STA);
  // Starting the conection
  WiFi.begin(ssid, password);
  // Waiting conection been established
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  IPAddress broadcastIP = WiFi.localIP();
  broadcastIP[3] = 255;
  Serial.print("Enviando solicitação para o endereço de broadcast: ");
  Serial.println(broadcastIP);
  while(!getApi(broadcastIP)){
    Serial.print(".");
  }
  
  // Visual confirmation
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  
}

void setup()
{
  pinMode(INTERRUPTION_PIN, INPUT);
  

  motionSensor.initialize(SDA_PIN, SCL_PIN);

  motionSensor.disableTemperature();
  motionSensor.disableGyroscope();

  motionSensor.enableInterruption();
  Serial.begin(9600);

  setupWifi();
}

void loop()
{
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Desconectou da Rede!!!!");
  }
  unsigned long currentTime = 0;
  unsigned long previousTime = 0;

  unsigned char movementPerformed = NEUTRAL;
  unsigned char movementsAmount = 0;
  unsigned short int movementsSequence = 0;

  while (currentTime - previousTime <= 1000)
  {
    if (movementsAmount == 3)
    {
      String payload = sendHttpPost(movementsSequence);
      if(payload=="-1"){
        Serial.println("Error on sending");
      }

      movementsSequence -= movementPerformed;
      movementsSequence /= 10;

      movementsAmount = 2;
      wait(100);
    }
    else
    {
      movementPerformed = verifyMovement();
      
      if (movementPerformed != NEUTRAL)
      {
        if (movementPerformed == SLEEP)
        {
          sleep();
          // break;
        }
        else
        {
          if (movementsAmount >= 1)
          {
            movementsSequence = movementsSequence * 10;
          }

          movementsSequence = movementsSequence + movementPerformed;

          movementsAmount++;

          previousTime = millis();

          wait(250);
        }
      }
    }

    currentTime = millis();
  }
}