/*
 
 Aquarium controler
 
 (C) 2014-2015 Flavius Bindea

Check history and latest versions on https://github.com/flav1972/aquarium

=============================================== 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================

*/

#include <Wire.h> // include the I2C library
#include <RTClib.h> // From: https://github.com/adafruit/RTClib.git 573581794b73dc70bccc659df9d54a9f599f4260
#include <EEPROM.h> // For read and write EEPROM
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include "globals.h"

// Initial setup
void setup() 
{
  Serial.begin(57600);
  Serial.println("Welcome to Aquarium Controler");

  // sets hall sensor for waterflow
  pinMode(hallsensor, INPUT); //initializes digital pin 2 as an input
  NbTopsFan = 0;   //Set NbTops to 0 ready for calculations
  FlowResetMillis = millis(); // gets the time
  attachInterrupt(0, rpm, RISING); //and the interrupt is attached
//  sei();      //Enables interrupts

  // initalise I2C interface  
  Wire.begin(); 
  
  // Configures RTC
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  // Start up DallasTemperature
  sensors.begin();
  sensors.getAddress(tempDeviceAddress, 0);
  sensors.setResolution(tempDeviceAddress, resolution);
  sensors.setWaitForConversion(false);  // request of temperature is non blocking
  sensors.requestTemperatures(); // sends command for all devices on the bus to perform a temperature conversion
  

  // Configures display
  // set up the number of columns and rows on the LCD 
  lcd.begin(20, 4);

  // for liquid cystal the create chars have to be done after the lcd.begin. on some other libraries this has to be done before
  lcd.createChar(ch_up, up_bitmap);
  lcd.createChar(ch_down, down_bitmap);
  lcd.createChar(ch_set, set_bitmap);
  lcd.createChar(ch_deg, deg_bitmap);

  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("Aquarium Controller ");
  // Print the special chars we need
  lcd.write(ch_up);
  lcd.write(ch_down);
  lcd.write(ch_set);
  lcd.write(ch_deg);
  lcd.write(ch_left);
  lcd.write(ch_right);

  // set the cursor to column 0, line 1
  // line 1 is the second row, since counting begins with 0
  lcd.setCursor(0, 1);
  // print to the second line
  lcd.print("RTC DS1307");
  
  // trys to read EEPROM
  if(AQ_SIG1 != EEPROM.read(0) || AQ_SIG2 != EEPROM.read(1)) {
    lcd.print(" NOT SET");
    EEPROM.write(0, AQ_SIG1);
    EEPROM.write(1, AQ_SIG2);
    
    for(int i = 2; i < 2+NBSETS*5; i++) {
      EEPROM.write(i, 0);
    }
  }
  else {
    // reads the EEPROM setup
    read_eeprom(0);
    read_eeprom(1);
    read_eeprom(2);
    read_eeprom(3);
  }
  
  // setout leds
  pinMode(Switch_1, OUTPUT);
  pinMode(Switch_2, OUTPUT);
  pinMode(Light_1, OUTPUT);
  pinMode(Light_2, OUTPUT);
  pinMode(Status_Led, OUTPUT);
  // Set initial state
  digitalWrite(Switch_1, LOW);
  digitalWrite(Switch_2, LOW);
  analogWrite(Light_1, 0); // Turn off light 1
  analogWrite(Light_2, 0); // Turn off light 2
  digitalWrite(Status_Led, HIGH);
  out[0] = Light_1;
  out[1] = Light_2;
  out[2] = Switch_1;
  out[3] = Switch_2;
  for(int i = 0; i < NBSETS; i++) {
    out_m[i] = AUTO;
    current_l[NBSETS] = asked_l[NBSETS] = last_l[NBSETS] = 0;  // last asked level and last level
  }    

  // smooth transition
  transitionSteps = transitionDuration / calculationInterval;
  
  delay(5000);
  lcd.clear();
}

/*
** Main loop
*/
void loop() 
{
  int pressed_bt;

//  Serial.println("loop");
  
  // For interval determination
  unsigned long currentMillis = millis();

  if(currentMillis - previousCalculationMillis > calculationInterval) {
      // save lasted calculation millis
      previousCalculationMillis = currentMillis;  
  
      // does interval calculations
      calculations();
  }
  // only once an interval
  if(currentMillis - previousDisplayMillis > displayInterval) {
    Serial.println("display interval------------------------------");

    // save lasted display millis
    previousDisplayMillis = currentMillis;  

    // getting the voltage reading from the temperature sensor
    if(sensors.isConversionAvailable(tempDeviceAddress)) {
      temperatureC = sensors.getTempC(tempDeviceAddress);
      //Serial.print("read temperature "); Serial.println(temperatureC);
    }
    else {
      Serial.println("no temperature available");
    }

    // display the data on the screen
    display_data();
    sensors.requestTemperatures(); // sends command for all devices on the bus to perform a temperature conversion
  } 

  pressed_bt = read_button();

  switch(pressed_bt) {
    case BT_SET:
      lcd.blink();
      do_menu();
      lcd.noBlink();
      // clean up the screen
      lcd.clear();
      break;
    case BT_LEFT:
      switch_out(0);
      break;
    case BT_RIGHT:
      switch_out(1);
      break;
    case BT_UP:
      switch_out(2);
      break;
    case BT_DOWN:
      switch_out(3);
      break;
  }
    
  // small delay
  delay(50);
}

/*
** does interval calculations
*/
void calculations()
{
  int h, m;
//  Serial.println("calculations");

  // read the date  
  now = RTC.now();
  h = now.hour();
  m = now.minute();
  
  // setting the status of the outputs
  for(int li = 0; li < 4; li++) {
//    Serial.print("Calculation for ");
//    Serial.println(li);
//    Serial.print("Nb of steps:");
//    Serial.println(transitionSteps);

    byte out_s;
    if(out_m[li] == OFF)
      out_s = OFF;
    else if(out_m[li] == ON)
      out_s = ON;
    else if(out_m[li] == MAX)
      out_s = MAX;
    else {
      // checking if we are in the ON time period
      byte order = ((ti[li].h2 > ti[li].h1) || (ti[li].h1 == ti[li].h2 && ti[li].m2 >= ti[li].m1)) ? 1 : 0;
      if( order && (h > ti[li].h1 || (h == ti[li].h1 && m >= ti[li].m1)) && (h < ti[li].h2 || (h == ti[li].h2 && m <= ti[li].m2))
        || ((h > ti[li].h2 || (h == ti[li].h2 && m >= ti[li].m2)) && (h < ti[li].h1 || (h == ti[li].h1 && m <= ti[li].m1))) )
        out_s = ON;
      else
        out_s = OFF;
    }
     
    if(li < 2) {
//      Serial.print("Status = ");
//      Serial.println(out_s);
      switch(out_s) {
        case OFF:
          asked_l[li] = 0;
          break;
        case ON:
          asked_l[li] = ti[li].power*255/99;
          break;
        case MAX:
          asked_l[li] = 255;
          break;
      }
//      Serial.print("Asked Level = ");
//      Serial.print(asked_l[li]);
//      Serial.print(", Last Level = ");
//      Serial.print(last_l[li]);

      if(asked_l[li] != last_l[li]) {
        incr_l[li] = ((long)asked_l[li]*256 - current_l[li])/transitionSteps;
        Serial.print("Set Increment To= ");
        Serial.println(incr_l[li]);
        last_l[li] = asked_l[li];
      }
//      Serial.print(", Increment = ");
//      Serial.print(incr_l[li]);
    
//      Serial.print(", Current Before = ");
//      Serial.println(current_l[li]);
      
      if(current_l[li] != asked_l[li]) {
        current_l[li] += incr_l[li];
        if(abs(current_l[li] - asked_l[li]*256) < abs(incr_l[li])) {
//             Serial.println("Last--------------------------------");
             current_l[li] = (unsigned)asked_l[li]*256;          
             incr_l[li] = 0;
        }
      }
//      Serial.print(", Current After = ");
//      Serial.println(current_l[li]);
      analogWrite(out[li], current_l[li]/256);
    }
    else {
      if(out_s == OFF)
        digitalWrite(out[li], LOW);
      else
        digitalWrite(out[li], HIGH);
    }
  }
}

// this displays the data on the screen: this function has to be rewritten and the call also. Do not need to redisplay everithing each second
void display_data()
{
  // Prints RTC Time on RTC
  now = RTC.now();
  
//  Serial.println("display data");

  // set the cursor to column 0, line 0     
  lcd.setCursor(0, 0);

  // print date
  print2dec(now.day());
  lcd.print('/');
  print2dec(now.month());
  lcd.print('/');
  lcd.print(now.year());

  // move the cursor to the second line
  lcd.setCursor(0, 1);
  // Print time
  print2dec(now.hour());
  lcd.print(':');
  print2dec(now.minute());
  lcd.print(':');
  print2dec(now.second());

  lcd.print(' ');
  // Prints statuses
  for(byte i = 0; i < NBSETS; i++) {
    display_out(i);
  }
  
  // displays temperature
  lcd.setCursor(12,0);

  // Now prints on LCD
  lcd.print((int)temperatureC);
  lcd.print('.');
  lcd.print((int)((temperatureC+0.05-(int)temperatureC)*10.0));

  // flow
  //get_flow();
}

// switch out put mode
void switch_out(byte n)
{
  switch(out_m[n]) {
    case OFF:
      out_m[n] = AUTO;
      break;
    case AUTO:
      out_m[n] = ON;
      break;
    case ON:
      out_m[n] = MAX;
      break;
    case MAX:
      out_m[n] = OFF;
      break;
  }
  display_out(n);
}

void display_out(byte i)
{
  lcd.setCursor(10+i, 1);
  switch(out_m[i]) {
    case OFF:
      lcd.print('0');
      break;
    case AUTO:
      lcd.print('A');
      break;
    case ON:
      lcd.print('1');
      break;
    case MAX:
      lcd.print('M');
      break;
  }        
}

void print2dec(int nb) 
{ //this adds a 0 before single digit numbers
  if (nb >= 0 && nb < 10) {
    lcd.write('0');
  }
  lcd.print(nb);
}


