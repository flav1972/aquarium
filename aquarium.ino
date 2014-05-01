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

// 126: -> 127: <-
byte up[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
};

byte down[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
};

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

// button types
#define BT_NONE 0
#define BT_SET 1
#define BT_LEFT 2
#define BT_RIGHT 3
#define BT_UP 4
#define BT_DOWN 5

// For looping by interval
long previousMillis = 0; 
// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 1000;           // interval at which to blink (milliseconds)

// keeping cursor position
char cx = 0, cy = 0;

// screen size
const byte cols = 16, lines = 2;

// menu of status
int menuline = 0;
const int menumin = 0;
const int menumax = 5;

// status of programm
#define ST_DISPLAY 0
#define ST_MENU 1
int status = ST_DISPLAY;

// Initial setup
void setup() 
{
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
  lcd.createChar(1, up);
  lcd.createChar(2, down);

  lcd.begin(cols, lines);
  
  // Print a message to the LCD.
  lcd.print("Hello you!!!");
  // set the cursor to column 0, line 1
  // line 1 is the second row, since counting begins with 0
  lcd.setCursor(0, 1);
  // print to the second line
  lcd.print("RTC DS1307");

  // setout leds
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  digitalWrite(8, HIGH);
  analogWrite(9, 20);
  analogWrite(10, 70);
  analogWrite(11, 105); // max 255
  delay(1000);
}

// Main loop
void loop() 
{
  int pressed_bt;
  
  // For interval determination
  unsigned long currentMillis = millis();
  
  if(status == ST_DISPLAY) {
    // only once an interval
    if(currentMillis - previousMillis > interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;  
  
      // does interval calculations
      calculations();
    
      // display the data on the screen
      display_data();
    } 
  }

  lcd.setCursor(cx, cy);

  pressed_bt = read_button();

  switch(pressed_bt) {
    case BT_SET:
      chg_status();
      do_menu();
      break;
    case BT_LEFT:
      cx--;
      //lcd.scrollDisplayLeft();
      break;
    case BT_RIGHT:
      cx++;
      //lcd.scrollDisplayRight(); 
      break;
   case BT_UP:
      cy--;
      menuline--;
      display_menu();
      break;
   case BT_DOWN:
      cy++;
      menuline++;
      display_menu();
      break;
   }
      
   // don't go out of screen
   if(cx < 0)
     cx = 0;
   else if(cx >= cols)
     cx = cols-1;
   if(cy < 0)
     cy = 0;
   else if(cy >= lines)
     cy = lines-1;
     
   // small delay
   delay(5);
}

// switch the status
void chg_status()
{
  if(status == ST_DISPLAY) {
    //lcd.cursor();
    lcd.blink();
    status = ST_MENU;
  }
  else {
    lcd.noBlink();
    status = ST_DISPLAY;
  }
}

// return button status if it has changed
int read_button()
{
  int button;
  // read the buttons
  button = analogRead(buttonsPin);

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
    if(bstate >=1 && bstate <= 5) {
      return(bstate);
    }
  }
  return(0);
}

// read blocking
int read_button_blocking()
{
  int i;
  while((i = read_button()) == 0)
    ;
    
  return i;
}

// does interval calculations
void calculations()
{
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
  }
  else {
    //Serial.print(index); Serial.println(" averaging");
  }  
}

// does the menu
void do_menu()
{
  int pressed_bt;

  display_menu();
  lcd.setCursor(0, 0);
  while((pressed_bt = read_button_blocking()) != BT_SET) {
    switch(pressed_bt) {
      case BT_LEFT:
        break;
      case BT_RIGHT:
        break;
     case BT_UP:
        menuline--;
        break;
     case BT_DOWN:
        menuline++;
        break;
     }
     if(menuline < menumin)
       menuline = menumin;
     else if(menuline > menumax)
       menuline = menumax;
     display_menu();
     lcd.setCursor(0, 0);
  }

  chg_status();
}

// displays the menu
void display_menu()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(1);
  print_entry(menuline);
  lcd.setCursor(15, 0);
  lcd.write(126);
  lcd.setCursor(0, 1);
  lcd.write(2);
  print_entry(menuline+1);
  lcd.setCursor(15, 1);
  lcd.write(126);
}

void print_entry(int i)
{
  switch(i) {
    case 0:
      lcd.print("Setup Date");
      break;
    case 1:
      lcd.print("Setup Time");
      break;
    case 2:
      lcd.print("Menu Entry 3");
      break;
    case 3:
      lcd.print("Menu Entry 4");
      break;
    case 4:
      lcd.print("Menu Entry 5");
      break;
    case 5:
      lcd.print("Menu Entry 6");
      break;
  }
}

// this displays the data on the screen
void display_data()
{
  // Prints RTC Time on RTC
  now = RTC.now();
  
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

  // displays temperature
  lcd.setCursor(12,0);

  // Now prints on LCD
  if(full) {
    lcd.print((int)temperatureC);
    lcd.print('.');
    lcd.print((int)((temperatureC+0.05-(int)temperatureC)*10.0));
  }
  else {
    lcd.print(index); lcd.print("Avr");
  }

}

void print2dec(int nb) { //this adds a 0 before single digit numbers
  if (nb >= 0 && nb < 10) {
    lcd.write('0');
  }
  lcd.print(nb);
}
