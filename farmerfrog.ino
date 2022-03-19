/* Greenhouse Heating Capstone Project - Winter 2022
 * Sponsored by Farmer Frog
 * Academic Advisor: Dr. Rick Cordray
 * By: Terry Tran, William Chau, Ibrahim Hashim, and Abdulrazig Alawad
 * 
 * For this greenhouse heating project, 
 * This is the microcontroller code that is used for the heating system. This includes all of the code used for the entire system.
 * Keypad implementation, relay integration, OLED display code, and temperature test cases are included down below. 
 */

#include "DHT.h"
#define DHT11Pin 2
#define DHTType DHT11

//OLED
#include <Wire.h>
#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//DS18b20
#include<OneWire.h>
#include<DallasTemperature.h>

#define ONE_WIRE_BUS 5

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int Celcius = 0;
int wTempF=0; 

DHT HT(DHT11Pin,DHTType);
int humi;
float tempC;
int tempF;

//OLED define
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//Keypad
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte colPins[COLS] = {10,11,12,13}; //connect to the row pinouts of the keypad
byte rowPins[ROWS] = {6,7,8,9}; //connect to the column pinouts of the keypad

// Relay Pins
const int pump = 3;
const int heat = 4;

// Timer
unsigned long prevMillis = 0;
unsigned long interval = 2000UL;
int autoMode = 1;

// Possible Interval Water Heater and Pump Option
//unsigned long tenMin = 600000UL; // Turn on for 10 minutes (pump)
//unsigned long oneHour = 3600000UL; // Turn off for an hour (pump and let water heater warm water)
//unsigned long prevMillis2 = 0;
//unsigned long prevMillis3 = 0; 


//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

//Desired Temperature
String input = "";
int desiredTemp = 0;
int minTemp = 0;
boolean done = false;

void setup() {
  sensors.begin();
//outputs
  pinMode(heat, OUTPUT); //Pin 4 - heating element
  pinMode(pump, OUTPUT); //Pin 3 - pump
  
//Display code
  Serial.begin(9600);
  //For DHT11
  HT.begin();
  //For OLED I2C
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display(); //Display logo
  delay(1000); 
  display.clearDisplay();
}

void loop() {
  // put your main code here, to run repeatedly:
  display.clearDisplay();
  oledDisplayHeader();
  display.display();
  
  char key = customKeypad.getKey();  
  if (key == 'A') // Mode A: Set Desired Temperature
  {
    desiredTemperatureMode();
    while (!done){
      key = customKeypad.waitForKey(); 
      if (key != 'A')
      {
        if (key == 'A' || key == 'C' || key == 'D' || key == '*' || key == '#')
        {
          continue;
        }
        display.print(key);
        display.display();
        input += key;
      }
      else
      {
        desiredTemp = input.toInt();
        input = "";
        display.println();
        display.print("Desired Temp is: ");
        display.print(desiredTemp); 
        display.print(" F");
        display.display();
        delay(1000);
        done = true; 
      }
    }
    done = false; // will revert back to false in-case another desired temperature is given
  }
  
  if (key == 'B') // Mode B: Set Minimum Temperature
  {
     minTemperatureMode();
     while (!done){
      key = customKeypad.waitForKey(); 
      if (key != 'B')
      {
        if (key == 'A' || key == 'C' || key == 'D' || key == '*' || key == '#')
        {
          continue;
        }
        display.print(key);
        display.display();
        input += key;
      }
      else
      {
        minTemp = input.toInt();
        input = "";
        display.println();
        display.print("Min Temp is: ");
        display.print(minTemp);
        display.print(" F"); 
        display.display();
        delay(1000);
        done = true; 
      }
    }
    done = false; // will revert back to false in-case another desired temperature is given
  }

  //OLED Alternating Message
  if (millis() - prevMillis > interval)
  {
    prevMillis += interval;
    if (autoMode == 1)
    {
      oledDisplayHeader();
      autoMode = 2;
    }
    if (autoMode == 2)
    {
      autoMode2();
      display.display();
      delay(1000);
    }
  }
}

void oledDisplayHeader(){
//DHT11
  humi = HT.readHumidity();
  tempC = HT.readTemperature();
  tempF = HT.readTemperature(true);
//DS18b20
  sensors.requestTemperatures(); 
  Celcius=sensors.getTempCByIndex(0);
  wTempF=sensors.toFahrenheit(Celcius);

  // Display Air and Water Temperatures
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(0, 0);
   display.print("Frog Heating");

   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(0, 20);
   display.print("Air Temp: ");
   display.print(tempF);
   display.print(" F");

   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(0, 35);
   display.print("Water Temp: ");
   display.print(wTempF);
   display.print(" F");

   
   if (wTempF >= 100) // Turn off Heater to conserve power if temperature is greater than or equal to 100
   {
      digitalWrite(heat, LOW);
   }
   
   if(tempF < desiredTemp) // Turns on pump and heater to circulate warm water if the temperature of the greenhouse is lower than the desired temp.
   {
      digitalWrite(heat, HIGH);
     // if (millis() - prevMillis2 > oneHour) <--- Note: the commented parts for this test case are for the optional possibility for interval implementation
     // {
     //   prevMillis2 += oneHour;
       // Timer = millis();
        digitalWrite(pump, HIGH);
      //}
      //if (Timer%tenMin == 0);
      //{
       // digitalWrite(pump, LOW);
        // Timer = 1;
      //}
   }
   else if(tempF >= desiredTemp) // Turn off the pump and water heater if the greenhouse temperature is greater than or equal to the desired temp.
   {
      digitalWrite(heat, LOW);
      digitalWrite(pump, LOW);
   }

   if (tempF == (minTemp +15)) // Turn on the heater 15 degrees before the greenhouse temp drops to minTemp
   {
      digitalWrite(heat, HIGH);
      if (tempF == minTemp) // Once greenhouse temp reaches the minTemp, turn on the pump and circulate the warm water
      {
        digitalWrite(pump, HIGH);
      }
   }
}

void desiredTemperatureMode() // OLED text for the desired temperature
{
   display.clearDisplay();
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(0, 0);
   display.println("Input desired temp: ");
   display.display();
}

void minTemperatureMode() // OLED text for the minimum temperature
{
   display.clearDisplay();
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(0, 0);
   display.println("Input minimum temp: ");
   display.display();
}

void autoMode2() // OLED text for the alternating information
{
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.print("Frog Heating");
      
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 20);
      display.print("Minimum Temp: ");
      display.print(minTemp);
      display.print(" F");
      
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 35);
      display.print("Desired Temp: ");
      display.print(desiredTemp);
      display.print(" F");
} 
