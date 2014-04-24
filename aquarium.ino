/*
 
 Aquarium controler
 
 (C) Flavius Bindea
 This example code is part of the public domain 
 */
#include <Wire.h> // include the I2C library
#include "RTClib.h"

// include the library code:
#include <LiquidCrystal.h>

// used for RTC
RTC_DS1307 RTC;
DateTime now;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(13, 12, 5, 4, 3, 2);


// For Temperature sensor TMP36 on A0
#define aref_voltage 5 // we tie 3.3V to ARef and measure it with a multimeter!
const int sensorPin = A0;
const float baselineTemp = 20.0;

float temperatureC;

// For Averaging
// Define the number of samples to keep track of.  The higher the number,
// the more the readings will be smoothed, but the slower the output will
// respond to the input.  Using a constant rather than a normal variable lets
// use this value to determine the size of the readings array.
const int numReadings = 10;
int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
int full = 0;                   // boolean in order to know if we have enoungh measurements

// For buttons
const int buttonsPin = A1;
int bstate = 1024, blast = 1024;  // button state and button last state
int button;

const int button1max = 75;    // reading should be 0, 75 threshold
const int button2min = 76;   // reading should be 151, from 76 to 250
const int button2max = 250;
const int button3min = 251;   // reading should be 347, from 251 to 430
const int button3max = 430;
const int button4min = 431;   // reading should be 515, from 431 to 606
const int button4max = 606;
const int button5min = 607;   // reading should be 700, from 607 to 850
const int button5max = 850;
const int buttonNONE = 900;   // reading should be 1023

// For looping by interval
long previousMillis = 0; 
// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 1000;           // interval at which to blink (milliseconds)
// Initial setup

void setup() {
  Serial.begin(57600);
  Serial.println("Welcome to Aquarium Controler");

  // Configures RTC
  Wire.begin(); // initalise I2C interface  

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  // If you want to set the aref to something other than 5v
  analogReference(EXTERNAL);

  // Configures display
  // set up the number of columns and rows on the LCD 
  lcd.begin(16, 2);

  // Print a message to the LCD.
  lcd.print("Hello you!!!");
  // set the cursor to column 0, line 1
  // line 1 is the second row, since counting begins with 0
  lcd.setCursor(0, 1);
  // print to the second line
  lcd.print("RTC DS1307");

  lcd.cursor();
  
  delay(1000);
}

void loop() {
  // For looping
  unsigned long currentMillis = millis();
  
  // Prints RTC Time on RTC
  now = RTC.now();
  
  // only once an interval
  if(currentMillis - previousMillis > interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;  
    // display the data on the screen
    display_data();
  } 

  // getting the voltage reading from the temperature sensor
  // subtract the last reading:
  total= total - readings[index];        
  // read from the sensor:  
  readings[index] = analogRead(sensorPin);
  //Serial.print(readings[index]); Serial.println(" reading");
  // add the reading to the total:
  total= total + readings[index];      
  // advance to the next position in the array:  
  index = index + 1;                    

  if (full == 0 && index == numReadings)
     full = 1;
     
  // if we're at the end of the array...
  if (index >= numReadings)              
     // ...wrap around to the beginning:
     index = 0;                          

  // displays temperature
  lcd.setCursor(12,0);

  if(full) {
    // calculate the average:
    average = total / numReadings;        
    //Serial.print(average); Serial.println(" average");

  
    // converting that reading to voltage, for 3.3v arduino use 3.3
    float voltage = average * aref_voltage;
    voltage /= 1024.0;
    // print out the voltage
    //Serial.print(voltage, 4); Serial.println(" volts");
    // now print out the temperature
    temperatureC = (voltage - 0.5) * 100 ; //converting from 10 mv per degree wit 500 mV offset
    //Serial.print(temperatureC); Serial.println(" degrees C");

    // Now prints on LCD
    lcd.print((int)temperatureC);
    lcd.print('.');
    lcd.print((int)((temperatureC+0.05-(int)temperatureC)*10.0));
  }
  else {
    //Serial.print(index); Serial.println(" averaging");
    lcd.print(index); lcd.print("Avr");
  }

  // read the buttons
  button = analogRead(buttonsPin);
  //Serial.print("Buttons : "); Serial.println(button);

  blast = bstate;

  if (button < button1max)
    bstate = 1;
  else if (button >= button2min && button <= button2max)
    bstate = 2;
  else if (button >= button3min && button <= button3max)
    bstate = 3;
  else if (button >= button4min && button <= button4max)
    bstate = 4;
  else if (button >= button5min && button <= button5max)
    bstate = 5;
  else if (button >= buttonNONE)
    bstate = 0;
  else
    bstate = 99; // we should never arrive here
  
  if(bstate == 99) {
    Serial.print("ERROR: "); Serial.println(button);
  }
  
  if (blast != bstate) {
    // state has changed
  //  Serial.print("Buttons State: "); Serial.println(bstate);
  //  Serial.print("Buttons Last: "); Serial.println(blast);
    if(bstate >=1 && bstate <= 5) {
      Serial.print("Pressed: "); Serial.println(bstate);
    }
    Serial.println(' ');
  }
}


// this displays the data on the screen
void display_data()
{
  /*
  // prints data on serial
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  Serial.print(" since 1970 = ");
  Serial.print(now.unixtime());
  Serial.print("s = ");
  Serial.print(now.unixtime() / 86400L);
  Serial.println("d");

  // calculate a date which is 7 days and 30 seconds into the future

  DateTime future (now.unixtime() + 7 * 86400L + 30);
  Serial.print(" now + 7d + 30s: ");
  Serial.print(future.year(), DEC);
  Serial.print('/');
  Serial.print(future.month(), DEC);
  Serial.print('/');
  Serial.print(future.day(), DEC);
  Serial.print(' ');
  Serial.print(future.hour(), DEC);
  Serial.print(':');
  Serial.print(future.minute(), DEC);
  Serial.print(':');
  Serial.print(future.second(), DEC);
  Serial.println();
  Serial.println();
*/
  // clean up the screen before printing
  lcd.clear();
  // set the cursor to column 0, line 0     
  lcd.setCursor(0, 0);
  // print some text
  //lcd.print("Date: ");
  print2dec(now.day());
  lcd.print('/');
  print2dec(now.month());
  lcd.print('/');
  lcd.print(now.year());

  // move the cursor to the second line
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  print2dec(now.hour());
  lcd.print(':');
  print2dec(now.minute());
  lcd.print(':');
  print2dec(now.second());
}

void print2dec(int nb) { //this adds a 0 before single digit numbers
  if (nb >= 0 && nb < 10) {
    lcd.write('0');
  }
  lcd.print(nb);
}
