/* DS18B20 1-Wire digital temperature sensor with Arduino example code. More info: https://www.makerguides.com 

FreeRTOS ESP32 post to Goggle Sheets: https://medium.com/@shishir_dey/upload-data-to-google-sheet-with-an-esp32-and-some-scripting-2d8b0ccbc833
AWS Arduino IoT: https://aws.amazon.com/blogs/compute/building-an-aws-iot-core-device-using-aws-serverless-and-an-esp32/
*/

/*
IoT Soil Temperature Sensor
Authors: John Miller, 

Board: ESP32 -- LOLIN D32 (default settings)

Descriptsion: On boot, read sensor, print value, deep sleep for 10 seconds.

*/

// -----------------------------------------------------------------------------------------------------------------
// Libraries required for DS18B20 temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// Define to which pin of the Arduino the 1-Wire bus is connected:
#define ONE_WIRE_BUS 4
// -----------------------------------------------------------------------------------------------------------------


//##################################################################################################################
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;
//##################################################################################################################


// -----------------------------------------------------------------------------------------------------------------
// Create a new instance of the oneWire class to communicate with any OneWire device:
OneWire oneWire(ONE_WIRE_BUS);

// Pass the oneWire reference to DallasTemperature library:
DallasTemperature sensors(&oneWire);
// -----------------------------------------------------------------------------------------------------------------


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

  
}

void loop() {
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

  
  //##################################################################################################################
  // DEEP SLEEP
  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
  
  //##################################################################################################################


}
