/*
  I2C LCD tests 

  I2C LCD library from:
  http://www.sainsmart.com/sainsmart-iic-i2c-twi-serial-2004-20x4-lcd-module-shield-for-arduino-uno-mega-r3.html
  https://bitbucket.org/fmalpartida/new-liquidcrystal
  
*/
#include <Wire.h> 
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27  // Define I2C Address where the PCF8574A is
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

LiquidCrystal_I2C	lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

// counter for serial printing
unsigned int i = 0;

// setup
void setup() {
  // Setup serial monitor
  Serial.begin(9600);
  
  lcd.begin (20,4);
  
  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();                   // go home

  lcd.print("Hello Test");  
  lcd.setCursor ( 0, 1 );        // go to the 2nd line
  lcd.print("(C)Flavius");

  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);

  // writes a messages on serial
  Serial.println("Setup done!!!"); 
}
  

// loop forever
void loop() {
  Serial.println(i);    // write on serial
  i++;                  // increment
  lcd.setCursor ( 0, 2 );        // go to the 3rd line
  lcd.print(i);
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);               // wait for half a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(500);               // wait for half a second
}


