#include <Arduino.h>
#include <MPU.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
// #include <PubSubClient.h>


#define INTERRUPTION_PIN 5 // D6
#define SDA_PIN 0 // D4
#define SCL_PIN 2 // D5

#define NEUTRAL 0
#define UP 1
#define RIGHT 2
#define DOWN 3
#define LEFT 4
#define SLEEP 5

// #define UUU 111
// #define UUR 112
// #define UUD 113
// #define UUL 114
// #define URU 121
// #define URR 122
// #define URD 123
// #define URL 124
// #define UDU 131
// #define UDR 132
// #define UDD 133
// #define UDL 134
// #define ULU 141
// #define ULR 142
// #define ULD 143
// #define ULL 144
// #define RUU 211
// #define RUR 212
// #define RUD 213
// #define RUL 214
// #define RRU 221
// #define RRR 222
// #define RRD 223
// #define RRL 224
// #define RDU 231
// #define RDR 232
// #define RDD 233
// #define RDL 234
// #define RLU 241
// #define RLR 242
// #define RLD 243
// #define RLL 244
// #define DUU 311
// #define DUR 312
// #define DUD 313
// #define DUL 314
// #define DRU 321
// #define DRR 322
// #define DRD 323
// #define DRL 324
// #define DDU 331
// #define DDR 332
// #define DDD 333
// #define DDL 334
// #define DLU 341
// #define DLR 342
// #define DLD 343
// #define DLL 344
// #define LUU 411
// #define LUR 412
// #define LUD 413
// #define LUL 414
// #define LRU 421
// #define LRR 422
// #define LRD 423
// #define LRL 424
// #define LDU 431
// #define LDR 432
// #define LDD 433
// #define LDL 434
// #define LLU 441
// #define LLR 442
// #define LLD 443
// #define LLL 444

// WiFi configs
const char* ssid = "FAMILIA MEDEIROS";
const char* password = "sl23jo316";

String api = "http://10.0.0.104:3000/makeMove";

// // MQTT Config
// const char *mqtt_broker = "MQTTIP";
// const char *topic = "MPU/Keyboard";
// const char *mqtt_username = "";
// const char *mqtt_password = "";
// const int mqtt_port = 1883;

MPU motionSensor;
WiFiClient espClient;
HTTPClient http;
// PubSubClient client(espClient);

// unsigned short int commandsSequence[31][2] = {{UUU, 10},
//                                               {UUR, 11},
//                                               {UUD, 12},
//                                               {UUL, 13},
//                                               {URU, 14},
//                                               {URR, 15},
//                                               {URD, 16},
//                                               {URL, 17},
//                                               {UDU, 18},
//                                               {UDR, 19},
//                                               {UDD, 20},
//                                               {UDL, 21},
//                                               {ULU, 22},
//                                               {ULR, 23},
//                                               {ULD, 24},
//                                               {ULL, 25},
//                                               {RUU, 26},
//                                               {RUR, 27},
//                                               {RUD, 28},
//                                               {DUU, 29},
//                                               {LUU, 30}};

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
  return payload;
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
  
  // Visual confirmation
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  
}

// void reconnectMQTT(){
//   while(!client.connected()){
//     if(client.connect("ESP8266", mqtt_username, mqtt_password)){
//       Serial.println("Conected on Broker!!");
//     } else {
//       Serial.print("failed with state ");
//       Serial.print(client.state());
//       delay(2000);
//     }
//   }
// }

// void setupMQTT(){
//   // Seting things here
//   client.setServer(mqtt_broker, mqtt_port);
//   Serial.println("Conecting to Broker...");
  
//   // Enlace until conects to the broker
//   reconnectMQTT();

//   client.subscribe(topic);
// }

// void pubDefualt(unsigned short int number)
// {
//     char convertion[2];
//     itoa(number, convertion, 10);
//     char msg[3];
//     strcpy(msg, "C");
//     strcpy(msg, convertion);
//     client.publish(topic, msg);
// }

void setup()
{
  pinMode(INTERRUPTION_PIN, INPUT);
  

  motionSensor.initialize(SDA_PIN, SCL_PIN);

  motionSensor.disableTemperature();
  motionSensor.disableGyroscope();

  motionSensor.enableInterruption();
  Serial.begin(9600);

  setupWifi();
  // setupMQTT(); 
}

void loop()
{
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Desconectou da Rede!!!!");
  }
  // if(!client.connected()){
  //   reconnectMQTT();
  // }
  // client.loop();
  unsigned long currentTime = 0;
  unsigned long previousTime = 0;

  unsigned char movementPerformed = NEUTRAL;
  unsigned char movementsAmount = 0;
  unsigned short int movementsSequence = 0;

  while (currentTime - previousTime <= 1000)
  {
    if (movementsAmount == 3)
    {
      // for (int row = 0; row < 21; row++)
      // {
      //   if (movementsSequence == commandsSequence[row][0])
      //   {
      //     pubDefualt(commandsSequence[row][1]);
          
      //     Serial.println("Comando enviado ");
      //     Serial.print(commandsSequence[row][0]);
      //     Serial.print(" | ");
      //     Serial.print(commandsSequence[row][1]);
      //     break;
      //   }
      // }
      String payload = sendHttpPost(movementsSequence);
      Serial.println(payload);

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