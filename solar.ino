#include <stdint.h>

#include "ThingSpeak.h"

#include <WiFi.h>

#include <stdio.h>

#include <stdlib.h>

#define WLAN_SSID   "satish"

#define WLAN_PASS   "satish123"

#define NUMSAMPLES 5

int curr_samples[NUMSAMPLES];

int volt_samples[NUMSAMPLES];

int temp_samples[NUMSAMPLES];

WiFiClient client;


#define THERMISTORNOMINAL 10000         



#define TEMPERATURENOMINAL 25   



#define BCOEFFICIENT 3950



#define SERIESRESISTOR 10000    

const int curr_an_pin = 35;

const int volt_an_pin = 34;

const int ntc_temp_an_pin = 33;

int count = 0;

#define thingSpeakAddress "xxxxxxxxx"   

#define channelID xxxxx                     

#define writeFeedAPIKey "xxxxxxx"                 

#define readFeedAPIKey "xxxxxxx"                  

#define readFieldAPIKey "xxxxxxxx"                

#define readStatusAPIKey "xxxxxxx"                

void setup() {

  
  Serial.begin(115200);  //Initialize serial

  delay(1000);

  WiFi.mode(WIFI_STA);

  ThingSpeak.begin(client);  

  xTaskCreate(

                wifi_task,           

                "wifi_task",             

                1024 * 2,               

                NULL,                   
                5,           

                NULL);                  
  Serial.print("Data Reading.");

}

void loop() {

 
  int i=0;

  float solar_curr_adc_val = 0;

  float solar_volt_adc_val = 0;

  for (i = 0; i < NUMSAMPLES; i++) {

                curr_samples[i] = analogRead(curr_an_pin);

                volt_samples[i] = analogRead(volt_an_pin);

                temp_samples[i] = analogRead(ntc_temp_an_pin);

                delay(10);

  }

  
  float curr_avg = 0;

  float volt_avg = 0;

  float temp_avg = 0;

  for (i = 0; i < NUMSAMPLES; i++) {

                curr_avg += curr_samples[i];

                volt_avg += volt_samples[i];

                temp_avg += temp_samples[i];

  }

  curr_avg /= NUMSAMPLES;

  volt_avg /= NUMSAMPLES;

  temp_avg /= NUMSAMPLES;

   float solar_curr = (curr_avg * 3.3 ) / (4095);

  float solar_volt = (volt_avg * 3.3 ) / (4095);
  
  solar_volt *= 6;

  temp_avg = 4095 / temp_avg - 1;

  temp_avg = SERIESRESISTOR / temp_avg;
 
  float steinhart;

  steinhart = temp_avg / THERMISTORNOMINAL;       

  steinhart = log(steinhart);               

  steinhart /= BCOEFFICIENT;                           // 1/B * ln(R/Ro)

  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)

  steinhart = 1.0 / steinhart;                 

  steinhart -= 273.15;                          

  delay(1000);

  count++;

  Serial.print(".");

  if (count >= 15 ) {

                count = 0;

                Serial.println("=========================================================================");

                Serial.print("Solar Voltage = ");

                Serial.println(solar_volt);

                Serial.print("Solar Current = ");

                Serial.println(solar_curr);

                float solar_watt = solar_volt * solar_curr;

                Serial.print("Solar Watt  = ");

                Serial.println(solar_watt);

                Serial.print("Solar Temperature  = ");

                Serial.println(steinhart);

                Serial.println("=========================================================================");

                if (WiFi.status() == WL_CONNECTED) {

                ThingSpeak.setField(1, solar_volt);

                ThingSpeak.setField(2, solar_curr);

                ThingSpeak.setField(3, solar_watt);

                ThingSpeak.setField(4, steinhart);

                // write to the ThingSpeak channel

                int x = ThingSpeak.writeFields(channelID, writeFeedAPIKey);

                if (x == 200) {

                Serial.println("Channels update successful.");

                }

                else {

                Serial.println("Problem updating channel. HTTP error code " + String(x));

                }

                } else {

                Serial.println("\r\n############################################################");

                Serial.println("Failed to update Data to thingSpeak Server. ");

                Serial.println("WiFi not connected...");

                Serial.println("############################################################\r\n");

                }

  Serial.print("Data Reading.");

  }

}

void wifi_task( void * parameter ) {

  while (1) {

                if (WiFi.status() != WL_CONNECTED) {

                Serial.print("Attempting to connect to SSID: ");

                Serial.println(WLAN_SSID);

                while (WiFi.status() != WL_CONNECTED) {

                WiFi.begin(WLAN_SSID, WLAN_PASS);  

                Serial.print(".");

                delay(5000);

                }

                Serial.println("\nConnected.");

                Serial.println();

                Serial.println("WiFi connected");

                Serial.println("IP address: ");

                Serial.println(WiFi.localIP());

                }

                vTaskDelay( 1000 / portTICK_PERIOD_MS);

  }

  vTaskDelete(NULL);

}