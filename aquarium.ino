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

byte second, minute, hour, dayweek, day, month, year; // create bytes for storing values read from DS1307
// SS[0-60], MM[0-60], HH[0-24], D[1-7], DD[1-31], MM[1-12], YY[00-99]

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(13, 12, 5, 4, 3, 2);

// a variable to choose which reply from the crystal ball
int reply;

void setup() {
  Serial.begin(57600);

  // Configures RTC
  Wire.begin(); // initalise I2C interface  

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  // Configures display
  // set up the number of columns and rows on the LCD 
  lcd.begin(16, 2);

  // Print a message to the LCD.
  lcd.print("Ask the");
  // set the cursor to column 0, line 1
  // line 1 is the second row, since counting begins with 0
  lcd.setCursor(0, 1);
  // print to the second line
  lcd.print("RTC DS1307");

  delay(5000);
}

void loop() {
  // Prints RTC Time
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
  
  // clean up the screen before printing a new reply
  lcd.clear();
  // set the cursor to column 0, line 0     
  lcd.setCursor(0, 0);
  // print some text
  lcd.print("Date: ");
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

  
  delay(1000);
}


void print2dec(int nb) { //this adds a 0 before single digit numbers
  if (nb >= 0 && nb < 10) {
    lcd.write('0');
  }
  lcd.print(nb);
}
