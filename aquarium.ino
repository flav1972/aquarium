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
#include <avr/pgmspace.h> // to put some strings in Flash
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
  int h, m;
  Serial.begin(57600);
  Serial.println(F("Welcome to Aquarium Controler"));

#ifdef DEBUG
  Debug_RAM("setup start");
#endif

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
    Serial.println(F("RTC is NOT running!"));
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
  lcd.begin(cols, lines);

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
  lcd.print(F("Aquarium Controller"));

  // set the cursor to column 0, line 1
  // line 1 is the second row, since counting begins with 0
  lcd.setCursor(0, 1);
  // print to the second line
  lcd.print(F("RTC DS1307"));
  
  // if old EEPROM
  if(AQ_SIG1 == EEPROM.read(0) && AQ_SIG2_old == EEPROM.read(1)) {
    lcd.print(F(" OLD MEMORY"));
    EEPROM.write(1, AQ_SIG2);
    for(int i = 2+NBSETS_old*5; i < 2+NBSETS*5; i++)
      EEPROM.write(i, 0);
    write_eeprom_temp();
    write_eeprom_fading();
  }
  else if (AQ_SIG1 != EEPROM.read(0) || AQ_SIG2 != EEPROM.read(1)) {
    lcd.print(F(" NOT SET"));
    EEPROM.write(0, AQ_SIG1);
    EEPROM.write(1, AQ_SIG2);

    for(int i = 2; i < 2+NBSETS*5; i++)
      EEPROM.write(i, 0);
    write_eeprom_temp();
    write_eeprom_fading();
  }
  // reads the EEPROM setup
  for(int i = 0; i < NBSETS; i++)
    read_eeprom(i);
  read_eeprom_temp();
  read_eeprom_fading();

  // set output status modes
  pinMode(Status_Led, OUTPUT);

  for(int i = 0; i < NBSETS; i++)
    pinMode(out[i], OUTPUT);
    
  // Set initial state
  for(int i = 0; i < NBSETS; i++) {
    digitalWrite(out[i], 0); // Turn off 
  }
  digitalWrite(Status_Led, HIGH);

  now = RTC.now();
  h = now.hour();
  m = now.minute();
  // setup requested status
  for(int i = 0; i < NBSETS; i++) {
    out_m[i] = AUTO;
    if(i < SWITCHSET) {
      // last asked level and last level
      current_l[i] = asked_l[i]
                    = (unsigned int)(ti[i].power*255.0/99.0)*256 * in_on_timerange(i, h, m);
    }
  }    

  // smooth transition
  transitionSteps = transitionDuration / calculationInterval;
  incr_l = 255*256/transitionSteps;

  delay(1000);
  lcd.clear();
#ifdef DEBUG
  Debug_RAM("setup end");
#endif
}

/*
** Main loop
*/
void loop() 
{
  int pressed_bt;

#ifdef DEBUG
  Debug_RAM("loop start");
#endif

  // For interval determination
  unsigned long currentMillis = millis();

#ifdef DEBUG
  Serial.print(F("currentMillis = "));
  Serial.println(currentMillis);
#endif

  if(currentMillis - previousCalculationMillis > calculationInterval) {
#ifdef DEBUG
    Serial.println(F("calculations interval------------------------------"));
#endif
    // save lasted calculation millis
    previousCalculationMillis = currentMillis;  

    // does interval calculations
    calculations();
  }
  // only once an interval
  if(currentMillis - previousDisplayMillis > displayInterval) {
#ifdef DEBUG
    Serial.println(F("display interval------------------------------"));
#endif

    // save lasted display millis
    previousDisplayMillis = currentMillis;  

    // Serial output hour
    print_hour();
    Serial.println();

    // getting the voltage reading from the temperature sensor
    if(sensors.isConversionAvailable(tempDeviceAddress)) {
      temperatureC = sensors.getTempC(tempDeviceAddress);
      Serial.print(F("TEMPERATURE: ")); Serial.println(temperatureC);
      if(temperatureC < -55.0) {
        Serial.println(F("ERROR: temperature out of range"));
        tempOk = 0;
      }
      else {
        tempTimeLastRead = currentMillis;
        tempOk = 1;
      }
    }
    else {
      Serial.println(F("ERROR: no temperature available"));
    }
    if(currentMillis - tempTimeLastRead > tempTimeMax) {
      Serial.println(F("ERROR: temperature expired"));
      tempOk = 0;
    }
    sensors.requestTemperatures(); // sends command for all devices on the bus to perform a temperature conversion

    // display the data on the screen
    display_data();
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
#ifdef DEBUG
  Debug_RAM("calculations start");
#endif

  // read the date  
  now = RTC.now();
  h = now.hour();
  m = now.minute();
  
//  Serial.print(F("Nb of steps:"));
//  Serial.println(transitionSteps);
  
  // puts temperature output to TMP(erature) status
  out_m[tempOutput] = TMP;
  // setting the status of the outputs
  for(int li = 0; li < NBSETS; li++) {
//    Serial.print(F("Calculation for "));
//    Serial.println(li);

//    Serial.print(F("Setup: "));
//    Serial.print(ti[li].h1);
//    Serial.print(':');
//    Serial.print(ti[li].m1);
//    Serial.print(F(" to "));
//    Serial.print(ti[li].h2);
//    Serial.print(':');
//    Serial.print(ti[li].m2);
//    Serial.print(F(" pow:"));
//    Serial.println(ti[li].power);

    byte out_s;
    if(out_m[li] == OFF)
      out_s = OFF;
    else if(out_m[li] == ON)
      out_s = ON;
    else if(out_m[li] == MAX)
      out_s = MAX;
    else if(out_m[li] == TMP) {
      // first check if temperature is corectly read
      if(tempOk) {
        // basic temperature regulation
        if (temperatureC > (tempSetpoint+tempTreshold)) {
          out_s = OFF;
          tempStatus = 0;
        }
        else if(temperatureC < (tempSetpoint-tempTreshold)) {
          out_s = ON;
          tempStatus = 1;      
        }
        else {
          out_s = (tempStatus == 1) ? ON : OFF;
        }
      }
      else {
        // temperature is not ok, turning of
        out_s = OFF;
        tempStatus = 0;
      }
    }
    else {
      // Auto time schedule
      
      // checking if we are in the ON time period
      if(in_on_timerange(li, h, m))
        out_s = ON;
      else
        out_s = OFF;
    }
    
//    Serial.print(F("calculation gives (0=OFF,1=AUTO,2=ON,3=MAX): "));
//    Serial.println(out_s);
     
    // if it is a light
    if(li < SWITCHSET) {
//      Serial.print(F("Calculation for "));
//      Serial.println(li);
//      Serial.print(F("Increment = "));
//      Serial.println(incr_l);
      switch(out_s) {
        case OFF:
          asked_l[li] = 0;
          break;
        case ON:
          asked_l[li] = (unsigned int)(ti[li].power*255.0/99.0)*256;
          break;
        case MAX:
          asked_l[li] = 255*256;
          break;
      }
//      Serial.print(F("Asked Level = "));
//      Serial.print(asked_l[li]);

//      Serial.print(F(", Current Before = "));
//      Serial.println(current_l[li]);
      
      if(current_l[li] < asked_l[li]) {
        if((asked_l[li] - current_l[li] ) < incr_l) {
//          Serial.println(F("==============Last  increasing"));
          current_l[li] = asked_l[li];          
        }
        else
          current_l[li] += incr_l;
      }
      else if(asked_l[li] < current_l[li]) {
        if((current_l[li] - asked_l[li]) < incr_l) {
//          Serial.println(F("==============Last  decreasing"));
          current_l[li] = asked_l[li];          
        }
        else
          current_l[li] -= incr_l;
      }
//      Serial.print(F("Current After = "));
//      Serial.println(current_l[li]);
//      Serial.print(F("Writing = "));
//      Serial.println(current_l[li]/256);
      // usign logarithmic perception table
      analogWrite(out[li], pgm_read_byte(&perception[current_l[li]/256]));
    }
    else {
      if(out_s == OFF) {
        digitalWrite(out[li], LOW);
//        Serial.println(F("putting OFF"));
      }
      else {
        digitalWrite(out[li], HIGH);
//        Serial.println(F("putting ON"));
      }
    }
  }
}

/**
 * calculates if we are in on period
 * @param li the number of the output
 * @param h hour
 * @param m minute
 * @return true if on, false if off
 */
boolean in_on_timerange(int li, int h, int m)
{
  // checking if we are in the ON time period
  // first we check if start time is earlier than end time
  byte order = ((ti[li].h2 > ti[li].h1) || (ti[li].h1 == ti[li].h2 && ti[li].m2 >= ti[li].m1)) ? 1 : 0;
  // order = 1 if time end is higher than time start

  // checks if current hour is in the ON interval
  if( (order && (h > ti[li].h1 || (h == ti[li].h1 && m >= ti[li].m1)) && (h < ti[li].h2 || (h == ti[li].h2 && m <= ti[li].m2))) 
            // start time <= current time and current time <= end time
    || (!order && ((h > ti[li].h1 || (h == ti[li].h1 && m >= ti[li].m1)) || (h < ti[li].h2 || (h == ti[li].h2 && m <= ti[li].m2))))
            // current time >= start time or current time <= end time
    )
    return true;
  return false;
}

/*
 * this displays the data on the screen 
 */
void display_data()
{
  int flow;

#ifdef DEBUG 
  Debug_RAM("display data");
#endif 

  // Gets RTC Time
  now = RTC.now();

  // set the cursor to column 0, line 0     
  lcd.setCursor(0, 0);

  // Print time
  print2dec(now.hour());
  lcd.print(':');
  print2dec(now.minute());
  lcd.print(':');
  print2dec(now.second());

  // Prints statuses, display_out moves the cursor
  for(byte i = 0; i < NBSETS; i++) {
    display_out(i);
  }
  
  // set the cursor to column 0, line 1
  lcd.setCursor(0, 1);

  // Now prints on LCD
#ifdef BIGSCREEN
  lcd.print(F("T:"));
#endif
  if(temperatureC > 0 && temperatureC < 100) {
    lcd.print((int)temperatureC);
    lcd.print('.');
    lcd.print((int)((temperatureC-(int)temperatureC)*10.0+0.5));
    lcd.write(ch_deg);
    lcd.write('C');
    lcd.write(' ');
  }
  else
    lcd.print(F("ErrorT "));

  // prints temperature output status
  lcd.print(tempStatus);
    
  // set the cursor to column 10, line 1
#ifdef BIGSCREEN
  lcd.setCursor(11, 1);
  lcd.print(F("F:"));
#else
  lcd.setCursor(10, 1);
#endif
  // displays the flow
  flow = get_flow();
  Serial.print(F("FLOW: ")); Serial.println(flow);
  print3dec(flow);
  lcd.print(F("L/H"));
  
#ifdef BIGSCREEN
  lcd.setCursor(0, 3);
  // print date
  print2dec(now.day());
  lcd.print('/');
  print2dec(now.month());
  lcd.print('/');
  lcd.print(now.year());
#endif
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
  lcd.setCursor(10+i, 0);
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
    case TMP:
      lcd.print('T');
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

void print3dec(int nb) 
{ //this adds a 0 before single digit numbers
  if(nb >= 0 && nb < 1000) {
    if (nb < 100) {
      lcd.write('0');
    }
    if (nb < 10) {
      lcd.write('0');
    }
    lcd.print(nb);
  }
  else
    lcd.print(F("Err"));
}

/**
 * Prints the hour on Serial
 */
void print_hour()
{
  Serial.print(now.hour());
  Serial.print(':');
  Serial.print(now.minute());
  Serial.print(':');
  Serial.print(now.second());
}

/**
 * this funtction returns free RAM
 * @return free RAM
 */
int freeRAM()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
