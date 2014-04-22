/*
 
 Aquarium controler
 
 (C) Flavius Bindea
 This example code is part of the public domain 
 */
#include <Wire.h> // include the I2C library
#include "RTClib.h"

// include the library code:
#include <LiquidCrystal.h>

RTC_DS1307 RTC;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(13, 12, 5, 4, 3, 2);


// For Temperature sensor TMP36 on A0
#define aref_voltage 3.3 // we tie 3.3V to ARef and measure it with a multimeter!
const int sensorPin = A0;
const float baselineTemp = 20.0;

void setup() {
  Serial.begin(57600);

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

  delay(5000);
}

void loop() {
  // Prints RTC Time on RTC
  DateTime now = RTC.now();
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

   //getting the voltage reading from the temperature sensor
  int reading = analogRead(sensorPin);
  // converting that reading to voltage, for 3.3v arduino use 3.3
  float voltage = reading * aref_voltage;
  voltage /= 1024.0;
  // print out the voltage
  Serial.print(voltage, 4); Serial.println(" volts");
  // now print out the temperature
  float temperatureC = (voltage - 0.5) * 100 ; //converting from 10 mv per degree wit 500 mV offset
  Serial.print(temperatureC); Serial.println(" degrees C");

  // Now prints on LCD
  lcd.setCursor(12,0);
  lcd.print((int)temperatureC);
  lcd.print('.');
  lcd.print((int)((temperatureC-(int)temperatureC)*10.0));
  
  delay(1000);
}


void print2dec(int nb) { //this adds a 0 before single digit numbers
  if (nb >= 0 && nb < 10) {
    lcd.write('0');
  }
  lcd.print(nb);
}
