#include <Arduino.h>
#include <NewPing.h>
#include <LiquidCrystal_I2C.h>

// http
#include <HttpClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>

// http addons
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

// standard modules
#include <cstring>
#include <iostream>
#include <vector>

#define INITIAL_STATE 0
#define PLAYING_STATE 1
#define ENDING_STATE 2

#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>
#include <NimBLEDevice.h>
#include <wire.h>
#include <SPI.h>
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// sonic sensor
NewPing sonar (2,15, 400);

// LCD
LiquidCrystal_I2C lcd(0x27,16,2);

// game variable
float targetDistance;
unsigned long startTime;
unsigned long gameTime;
int score;
int player;
int state; // Game state
int time_counter;
std::vector<int> scoring;
std::vector<int> timer;
String server = "3.145.1.5";


// laser and receiver 
int laserValue1;
int laserValue2;
int laserTransmitterPin1 = 12;  // GPIO pin for the laser transmitter (KY-008)
int laserReceiverPin1 = 13;     // GPIO pin for the laser receiver (Laser Sensor Module)
int laserTransmitterPin2 = 26;  // GPIO pin for the laser transmitter (KY-008)
int laserReceiverPin2 = 27;     // GPIO pin for the laser receiver (Laser Sensor Module)



//HTTP STUFF
// This example downloads the URL "http://arduino.cc/"
// char ssid[50]; // your network SSID (name)
// char pass[50]; // your network password (use for WPA, or use

char ssid[50] = "Freddie morales";
char pass[50] = "9517964377";
               // as key for WEP)
// Name of the server we want to connect to
const char kHostname[] = "worldtimeapi.org";
// Path to download (this is the bit after the hostname in the URL // that you want to download
const char kPath[] = "/api/timezone/Europe/London.txt";
// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

void nvs_access()
{
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  // Open
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  // Close
  nvs_close(my_handle);
}



class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
  std::string value = pCharacteristic->getValue();
    int num = stoi(value);
    gameTime = num * 1000;
  }
};

void setup() {
  nvs_access();
  delay(2000); 
  Serial.begin(9600);
  Serial.println("download success");
  // http stuff
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  //laser and transmitter
  pinMode(laserTransmitterPin1, OUTPUT);
  pinMode(laserReceiverPin1, INPUT); 
  pinMode(laserTransmitterPin2, OUTPUT);
  pinMode(laserReceiverPin2, INPUT); 

  // LCD
  lcd.init();
  lcd.backlight();

  //BLE//
  Serial.begin(9600);
  Serial.println("Let's play basketball!");

  BLEDevice::init("BasketBall Game");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                        CHARACTERISTIC_UUID,
                                        NIMBLE_PROPERTY::READ |
                                        NIMBLE_PROPERTY::WRITE 
                                        );

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World");

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();


  state = INITIAL_STATE;
  gameTime = 0;
  time_counter = 0;
  player = 0;
  scoring.clear();
  timer.clear();


  
}

void loop() {
    switch (state) {

    case INITIAL_STATE:
      digitalWrite(laserTransmitterPin1, HIGH);  // Turn on the laser transmitter
      digitalWrite(laserTransmitterPin2, HIGH);  // Turn on the laser transmitter

      if (gameTime != 0){
        state = PLAYING_STATE;
        startTime = millis();
        score = 0;
        ///sonic sensor
        targetDistance = sonar.ping_cm() * 0.01;
        Serial.print("Distance: ");
        Serial.print(targetDistance);
        Serial.println(" meters");
        delay(1000); //pause to let things settle

      }
      break;

    case PLAYING_STATE:
        //laser sensor
        digitalWrite(laserTransmitterPin1, HIGH);  // Turn on the laser transmitter
        laserValue1 = digitalRead(laserReceiverPin1);  // Read the laser receiver value
        digitalWrite(laserTransmitterPin2, HIGH);  // Turn on the laser transmitter
        laserValue2 = digitalRead(laserReceiverPin2);  // Read the laser receiver value

        

        //Serial.print(laserValue);
        if ((laserValue1 == HIGH) || (laserValue2 == HIGH)) {
          if (targetDistance < 1) {
            score++;
          } else {
            score++;
            score++;
          }

          Serial.print("Score: ");
          Serial.println(score);
          delay(500);
        }

        time_counter = (millis() - startTime)/1000;
        scoring.push_back(score);
        timer.push_back(time_counter);
        
        // this will set the threshold units
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("D:");
        lcd.print(targetDistance);
        lcd.setCursor(8,0);
        lcd.print("Time:");
        lcd.print(startTime + gameTime - millis());
        lcd.setCursor(7,1);
        lcd.print(score);


        if (millis() > startTime + gameTime){
          state = ENDING_STATE;
          digitalWrite(laserTransmitterPin1, LOW);   // Turn off the laser transmitter
          digitalWrite(laserTransmitterPin2, LOW);
          gameTime = 0; // reset the game time
          player ++;
          delay(1000);  
        }


      break;

    case ENDING_STATE:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Game over!");
        Serial.print("Game over\n");
        //sending the score to AWS
        int err = 0;

        WiFiClient c;
        HttpClient http(c);

        String url = "/?score=" + String(0) + "&time=" + String(0);

          if (c.connect("3.145.1.5", 5000))
          {
          c.print(String("GET ") + url + " HTTP/1.1\r\n" +
                  "Host: " + server + "\r\n" +
                  "Connection: close\r\n\r\n");
          }

        for (int i = 0; i <scoring.size(); ++i)
        {
          String url = "/?score=" + String(scoring[i]) + "&time=" + String(timer[i]);

          if (c.connect("3.145.1.5", 5000))
          {
          c.print(String("GET ") + url + " HTTP/1.1\r\n" +
                  "Host: " + server + "\r\n" +
                  "Connection: close\r\n\r\n");
          }
        }
        c.stop();
        
        delay(10000);

        state = INITIAL_STATE;
        score = 0;    // reset the game score
      break;
    
    
    }



}