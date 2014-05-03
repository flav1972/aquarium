/*
 
 Aquarium controler
 
 (C) Flavius Bindea
 This example code is part of the public domain 
 */
#include <Wire.h> // include the I2C library
#include "RTClib.h" // From: https://github.com/adafruit/RTClib.git 573581794b73dc70bccc659df9d54a9f599f4260
#include <EEPROM.h> // Fro read and write EEPROM

// include the library code:
#include <LiquidCrystal.h>

// used for RTC
const int dayspermonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
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
#define aref_voltage 4.91 // real voltage
#define amplifier 3.58    // 3.27 -> amplifier = (R1+R2)/R1 = (220+500)/220, exact=(216+558)/216=3.58

const int sensorPin = A0;
const float baselineTemp = 20.0;

float temperatureC;

// For Averaging
// Define the number of samples to keep track of.  The higher the number,
// the more the readings will be smoothed, but the slower the output will
// respond to the input.  Using a constant rather than a normal variable lets
// use this value to determine the size of the readings array.
#define numReadings 10
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

// screen size
const byte cols = 16, lines = 2;

// menu of status
const int menumin = 0;
const int menumax = 5;

char* menu_entry[] = {
  "1. Set Date/Time",
  "2. Light 1      ",
  "3. Light 2      ",
  "4. Menu entry 4 ",
  "5. Menu entry 5 ",
  "6. Menu entry 6 "
};

// status of programm
#define ST_DISPLAY 0
#define ST_MENU 1
int status = ST_DISPLAY;

/*
 * Define the devices
 */
#define Light_1 11
#define Light_2 10
#define Switch_1 9
#define Switch_2 8

struct AQTIME {
  byte h1;
  byte m1;
  byte h2;
  byte m2;
  byte power;
};

#define NBSETS 2
AQTIME ti[NBSETS];

// EEPROM signature for aquarium: they are stored in 0 and 1
const byte AQ_SIG1 = 45, AQ_SIG2 = 899;

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
  
  // trys to read EEPROM
  if(AQ_SIG1 != EEPROM.read(0) || AQ_SIG2 != EEPROM.read(1)) {
    lcd.print(" NOT SET");
    EEPROM.write(0, AQ_SIG1);
    EEPROM.write(1, AQ_SIG2);
    
    for(int i= 2; i < 2+NBSETS*5; i++) {
      EEPROM.write(i, 0);
    }
  }
  else {
    // reads the EEPROM setup
    read_light(1);
    read_light(2);
  }
  
  // setout leds
  pinMode(Switch_1, OUTPUT);
  pinMode(Switch_2, OUTPUT);
  pinMode(Light_1, OUTPUT);
  pinMode(Light_2, OUTPUT);
  // Set initial state
  digitalWrite(Switch_1, HIGH);
  digitalWrite(Switch_2, LOW);
  analogWrite(Light_1, 0); // Turn off light 1
  analogWrite(Light_2, 0); // Turn off light 2
  delay(1000);
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
  if(status == ST_DISPLAY) {
    // only once an interval
    if(currentMillis - previousMillis > interval) {
      Serial.println("display interval");
      // save the last time you blinked the LED
      previousMillis = currentMillis;  
  
      // does interval calculations
      calculations();
    
      // display the data on the screen
      display_data();
    } 
  }

  pressed_bt = read_button();

  switch(pressed_bt) {
    case BT_SET:
      chg_status();
      do_menu();
      break;
    case BT_LEFT:
      break;
    case BT_RIGHT:
      break;
   case BT_UP:
      break;
   case BT_DOWN:
      break;
   }
      
   // small delay
   delay(100);
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

/*
** return button status if it has changed
*/
int read_button()
{
  int button;
  /*
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
 
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed
    */
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

//  Serial.print("VALUE: "); Serial.println(button);

  if(bstate == 99) {
    Serial.print("ERROR: "); Serial.println(button);
  }
  
  if (blast != bstate) {
    // state has changed
    if(bstate >=1 && bstate <= 5) {
      Serial.print("BUTTON: "); Serial.println(bstate);  
      return(bstate);
    }
  }
  return(0);
}

// read blocking
int read_button_blocking()
{
  int i;

  Serial.println("read button blocking");

  while((i = read_button()) == 0)
    delay(50);
    
  return i;
}

/*
** does interval calculations
*/
void calculations()
{
  int h, m;
  Serial.println("calculations");

  // getting the voltage reading from the temperature sensor
  // subtract the last reading:
  total= total - readings[index];        
  // read from the sensor:  
  delay(100);
  readings[index] = analogRead(sensorPin);
//  delay(100);
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
  
    // converting that reading to voltage
    float voltage = average * aref_voltage/amplifier;
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

  // read the date  
  now = RTC.now();
  h = now.hour();
  m = now.minute();
  
  // checks if we are in the time zone
  for(int li = 0; li < 2; li++) {
    int out = (li == 0) ? Light_1 : Light_2;
    byte order = ((ti[li].h2 > ti[li].h1) || (ti[li].h1 == ti[li].h2 && ti[li].m2 >= ti[li].m1)) ? 1 : 0;
    if( order && (h > ti[li].h1 || (h == ti[li].h1 && m >= ti[li].m1)) && (h < ti[li].h2 || (h == ti[li].h2 && m <= ti[li].m2))
      || ((h > ti[li].h2 || (h == ti[li].h2 && m >= ti[li].m2)) && (h < ti[li].h1 || (h == ti[li].h1 && m <= ti[li].m1))) ) { 
      // sets the leds if they have changed
      analogWrite(out, ti[li].power*255/99);
    }
    else
      analogWrite(out, 0);  
  }
}

// does the menu
void do_menu()
{
  int pressed_bt = -1;
  int menuline = 0;

  Serial.println("do menu---------------------------------");

  start_menu();

  do {
    Serial.println("not set button");
    switch(pressed_bt) {
      case BT_LEFT:
        break;
      case BT_RIGHT:
        do_menu_entry(menuline);
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

    lcd.setCursor(0, 1);
    lcd.write(menu_entry[menuline]);
    lcd.setCursor(15, 1);
  } while((pressed_bt = read_button_blocking()) != BT_SET);

  Serial.println("SET button pressed");
  chg_status();
}

void start_menu()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write("Menu: use ");
  lcd.write(1);
  lcd.write(2);
  lcd.write(126);
  lcd.write("SET");
}

void do_menu_entry(int en)
{
  Serial.print("Do menu entry:");
  Serial.println(en);

  switch(en) {
     case 0:
       set_time();
       break;
     case 1:
       set_light(1);
       break;
     case 2:
       set_light(2);
       break;
  }
}

/*
** Menu entry to setup the time
*/
void set_time()
{
  int pressed_bt = -1;
  int pos = 0, v;
  char val[16];
  int day, month, year, hour, min;
  int i;
  int ok = 0;

  Serial.println("do set time---------------------------------");
  /*
  ** 0123456789012345
  ** 0         1
  ** DD/MM/YYYY HH:MM
  */
  
  now = RTC.now();
  val[0] = now.day()/10+'0';
  val[1] = now.day()%10+'0';
  val[2] = '/';
  val[3] = now.month()/10+'0';
  val[4] = now.month()%10+'0';
  val[5] = '/';
  year = now.year();
  val[6] = '2';
  val[7] = '0';
  year = year-2000;
  val[8] = year/10+'0';
  val[9] = year%10+'0';
  val[10] = ' ';
  val[11] = now.hour()/10+'0';
  val[12] = now.hour()%10+'0';
  val[13] = ':';
  val[14]= now.minute()/10+'0';
  val[15]= now.minute()%10+'0';

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write("Time: use");
  lcd.write(1);
  lcd.write(2);
  lcd.write(127);
  lcd.write(126);
  lcd.write("SET");

  lcd.setCursor(0, 1);
  for(i = 0; i < 16; i++)
    lcd.print(val[i]);

  do {
    do {
      Serial.println("not set button");
      switch(pressed_bt) {
        case BT_LEFT:
          switch(pos) {
              case 1:
                pos = 0;
                break;
              case 3:
                pos = 1;
                break;
              case 4:
                pos = 3;
                break;
              case 8:
                pos = 4;
                break;
              case 9:
                pos = 8;
                break;
              case 11:
                pos = 9;
                break;
              case 12:
                pos = 11;
                break;
              case 14:
                pos = 12;
                break;
              case 15:
                pos = 14;
                break;
          }
          break;
        case BT_RIGHT:
          switch(pos) {
              case 0:
                pos = 1;
                break;
              case 1:
                pos = 3;
                break;
              case 3:
                pos = 4;
                break;
              case 4:
                pos = 8;
                break;
              case 8:
                pos = 9;
                break;
              case 9:
                pos = 11;
                break;
              case 11:
                pos = 12;
                break;
              case 12:
                pos = 14;
                break;
              case 14:
                pos = 15;
                break;
          }
          break;
       case BT_UP:
          val[pos]++;
          break;
       case BT_DOWN:
          val[pos]--;
          break;
       }
  
      if(val[pos] < '0')
        val[pos] = '0';
      else if (val[pos] > '9')
        val[pos] = '9'; 
        
      lcd.setCursor(pos, 1);
      lcd.print(val[pos]);
      lcd.setCursor(pos, 1);
    } while((pressed_bt = read_button_blocking()) != BT_SET);
    day = (val[0] - '0')*10+val[1]-'0';
    month = (val[3]-'0')*10+val[4]-'0';
    year = (val[8]-'0')*10+val[9]-'0';
    hour = (val[11]-'0')*10+val[12]-'0';
    min = (val[14]-'0')*10+val[15]-'0';
    Serial.print("day:");
    Serial.println(day);
    Serial.print("month:");
    Serial.println(month);
    Serial.print("year:");
    Serial.println(year);
    Serial.print("hour:");
    Serial.println(hour);
    Serial.print("min:");
    Serial.println(min);

    if(min >= 0 && min < 60
      && hour >= 0 && hour < 24
      && month >= 1 && month <= 12
      && year >= 0 && year <= 99 
      && day >= 0 && day <= dayspermonth[month-1])
              ok = 1;
  } while(!ok);  
  RTC.adjust(DateTime(year, month, day, hour, min, 0));
}

/*
** setting light
*/
void set_light(byte lnb)
{
  int place, eelocate;
  int pressed_bt = -1;
  int pos = 0, v;
  char val[16];
  byte h1, m1, h2, m2, power;
  int i;
  int ok = 0;
  place = lnb - 1;
  eelocate = 2+place*5;

  Serial.println("do set light 1---------------------------");
  
  // make sure we are up tu date from EEPROM
  read_light(lnb);
  h1 = ti[place].h1;   
  m1 = ti[place].m2;   
  h2 = ti[place].h2;   
  m2 = ti[place].m2;   
  power = ti[place].power;   

  /*
  ** 0123456789012345
  ** 0         1
  ** HH:MM HH:MM XX
  */
  
  val[0] = h1/10+'0';
  val[1] = h1%10+'0';
  val[2] = ':';
  val[3] = m1/10+'0';
  val[4] = m1%10+'0';
  val[5] = ' ';
  val[6] = h2/10+'0';
  val[7] = h2%10+'0';
  val[8] = ':';
  val[9] = m2/10+'0';
  val[10] = m2%10+'0';
  val[11] = ' ';
  val[12] = power/10+'0';
  val[13] = power%10+'0';
  val[14] = ' ';
  val[15] = ' ';
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write("Start Stop  Power");

  lcd.setCursor(0, 1);
  for(i = 0; i < 16; i++)
    lcd.print(val[i]);

  do {
    do {
      Serial.println("not set button");
      switch(pressed_bt) {
        case BT_LEFT:
          switch(pos) {
              case 1:
                pos = 0;
                break;
              case 3:
                pos = 1;
                break;
              case 4:
                pos = 3;
                break;
              case 6:
                pos = 4;
                break;
              case 7:
                pos = 6;
                break;
              case 9:
                pos = 7;
                break;
              case 10:
                pos = 9;
                break;
              case 12:
                pos = 0;
                break;
              case 13:
                pos = 12;
                break;
          }
          break;
        case BT_RIGHT:
          switch(pos) {
              case 0:
                pos = 1;
                break;
              case 1:
                pos = 3;
                break;
              case 3:
                pos = 4;
                break;
              case 4:
                pos = 6;
                break;
              case 6:
                pos = 7;
                break;
              case 7:
                pos = 9;
                break;
              case 9:
                pos = 10;
                break;
              case 10:
                pos = 12;
                break;
              case 12:
                pos = 13;
                break;
          }
          break;
       case BT_UP:
          val[pos]++;
          break;
       case BT_DOWN:
          val[pos]--;
          break;
       }
  
      if(val[pos] < '0')
        val[pos] = '0';
      else if (val[pos] > '9')
        val[pos] = '9'; 
        
      lcd.setCursor(pos, 1);
      lcd.print(val[pos]);
      lcd.setCursor(pos, 1);
    } while((pressed_bt = read_button_blocking()) != BT_SET);
    h1 = (val[0]-'0')*10+val[1]-'0';
    m1 = (val[3]-'0')*10+val[4]-'0';
    h2 = (val[6]-'0')*10+val[7]-'0';
    m2 = (val[9]-'0')*10+val[10]-'0';
    power = (val[12]-'0')*10+val[13]-'0';

    if(h1 >= 0 && h1 < 24
      && m1 >= 0 && m1 < 60
      && h2 >= 0 && h2 < 24
      && m2 >= 0 && m2 < 60
      && power >= 0 && power <= 99)
              ok = 1;
  } while(!ok);  
  ti[place].h1 = h1;   
  ti[place].m1 = m1;   
  ti[place].h2 = h2;   
  ti[place].m2 = m2;   
  ti[place].power = power;   

  EEPROM.write(eelocate++, h1); // H1  
  EEPROM.write(eelocate++, m1); // M1  
  EEPROM.write(eelocate++, h2); // H2  
  EEPROM.write(eelocate++, m2); // M2  
  EEPROM.write(eelocate, power); // P1  
}

// reads data from EEPROM
void read_light(byte nb)
{
  int place, eelocate;
  place = nb - 1;
  eelocate = 2+place*5;
  ti[place].h1 = EEPROM.read(eelocate++);   
  ti[place].m1 = EEPROM.read(eelocate++);   
  ti[place].h2 = EEPROM.read(eelocate++);   
  ti[place].m2 = EEPROM.read(eelocate++);   
  ti[place].power = EEPROM.read(eelocate);   

}

// this displays the data on the screen
void display_data()
{
  // Prints RTC Time on RTC
  now = RTC.now();
  
  Serial.println("display data");
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
