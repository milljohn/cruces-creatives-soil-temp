/* DS18B20 1-Wire digital temperature sensor with Arduino example code. More info: https://www.makerguides.com/ds18b20-arduino-tutorial/

NOTE: DS18B20 signal wire must not be on the LED pin. For the Lolin32 Lite, that is pin 22.

FreeRTOS ESP32 post to Goggle Sheets: https://medium.com/@shishir_dey/upload-data-to-google-sheet-with-an-esp32-and-some-scripting-2d8b0ccbc833
AWS Arduino IoT: https://aws.amazon.com/blogs/compute/building-an-aws-iot-core-device-using-aws-serverless-and-an-esp32/
Here is a template: https://github.com/Savjee/arduino-aws-iot-starter-template

Timestamps should be handled by the backend, however conecting to NTP 
is interesting and an example can be found here: https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/


ARCHITECTURE NOTES:

ESP32 => MQTT => IoT Core => RULE => Timestream
                                        ||
                                        ||=> Grafana

Grafana is used to visualize data stored in Timestream.
https://grafana.com/grafana/download?platform=linux
https://grafana.com/grafana/plugins/grafana-timestream-datasource/installation

*/

/*
IoT Soil Temperature Sensor
Authors: John Miller, 

Board: ESP32 -- LOLIN D32 (default settings)

Descriptsion: On boot, read sensor, print value, deep sleep for 10 seconds.

*/
//####################################################################
#include <Arduino.h>
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

// -----------------------------------------------------------------------------------------------------------------
// Libraries required for DS18B20 temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// Define to which pin of the Arduino the 1-Wire bus is connected:
#define ONE_WIRE_BUS 19
// -----------------------------------------------------------------------------------------------------------------


//##################################################################################################################
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;
//##################################################################################################################

// prototypes
void connectAWS();
void publishMessage();
void messageHandler(String &topic, String &payload);


// -----------------------------------------------------------------------------------------------------------------
// Create a new instance of the oneWire class to communicate with any OneWire device:
OneWire oneWire(ONE_WIRE_BUS);

// Pass the oneWire reference to DallasTemperature library:
DallasTemperature sensors(&oneWire);
// -----------------------------------------------------------------------------------------------------------------



//##################################################################################################################
// AWS
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
    sensors.requestTemperatures();

  // Fetch the temperature in degrees Celsius for device index:
  float tempC = sensors.getTempCByIndex(0); // the index 0 refers to the first device
  // Fetch the temperature in degrees Fahrenheit for device index:
  float tempF = sensors.getTempFByIndex(0);

  StaticJsonDocument<200> doc;
  doc["id"] = WiFi.macAddress();
  // doc["time"] = millis();
  doc["temp_c"] = tempC;
  doc["temp_f"] = tempF;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  client.disconnect();
}

void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

//  StaticJsonDocument<200> doc;
//  deserializeJson(doc, payload);
//  const char* message = doc["message"];
}

//##################################################################################################################





void setup() {
  // Begin serial communication at a baud rate of 9600:
  Serial.begin(9600);
  
  // Start the temperature sensor
  sensors.begin();
  
  //##################################################################################################################
  // DEEP SLEEP
  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  
  //##################################################################################################################

  connectAWS();
  
    // -----------------------------------------------------------------------------------------------------------------
  // TEMPERATURE
  // Send the command for all devices on the bus to perform a temperature conversion:
  sensors.requestTemperatures();

  // Fetch the temperature in degrees Celsius for device index:
  float tempC = sensors.getTempCByIndex(0); // the index 0 refers to the first device
  // Fetch the temperature in degrees Fahrenheit for device index:
  float tempF = sensors.getTempFByIndex(0);

  // Print the temperature in Celsius in the Serial Monitor:
  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.print(" \xC2\xB0"); // shows degree symbol
  Serial.print("C  |  ");

  // Print the temperature in Fahrenheit
  Serial.print(tempF);
  Serial.print(" \xC2\xB0"); // shows degree symbol
  Serial.println("F");
  // -----------------------------------------------------------------------------------------------------------------

  publishMessage();
  client.loop();
  
  //##################################################################################################################
  // DEEP SLEEP
  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
  
  //##################################################################################################################

}



void loop() {


}
