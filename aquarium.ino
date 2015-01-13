/*
 
 Aquarium controler
 
 (C) 2014-2015 Flavius Bindea

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
#include "RTClib.h" // From: https://github.com/adafruit/RTClib.git 573581794b73dc70bccc659df9d54a9f599f4260
#include <EEPROM.h> // Fro read and write EEPROM
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

// used for RTC
const int dayspermonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
RTC_DS1307 RTC;
DateTime now;

// I2C LCD setups
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

// temperatu reader based on DS18B20
// Data wire is plugged into pin 4 on the Arduino
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

DeviceAddress tempDeviceAddress;
int  resolution = 10; // 10 bits means 0.25°C increments conversion duration is 187.5ms

const float baselineTemp = 20.0;

float temperatureC;


// For buttons
const int buttonsPin = A1;
int bstate = 1024, blast = 1024;  // button state and button last state

// change value depending on your measurements
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

// For looping display by interval
unsigned long previousDisplayMillis = 0; 
unsigned long displayInterval = 1000;
// For looping calculation by interval
unsigned long previousCalculationMillis = 0; 
unsigned long calculationInterval = 250;

// screen size
const byte cols = 16, lines = 2;

// menu of status
const int menumin = 0;
const int menumax = 5;

char* menu_entry[] = {
  "1. Set Date/Time",
  "2. Light 1 setup",
  "3. Light 2 setup",
  "4. Switch 1 set ",
  "5. Switch 2 set ",
  "6. Menu entry 6 "
};

// status of programm
#define ST_DISPLAY 0
#define ST_MENU 1
int status = ST_DISPLAY;

/*
 * function prototypes
 */
void set_function(byte lnb, byte wpower=1);

/*
 * Define the devices
 */
#define Light_1 11
#define Light_2 10
#define Switch_1 9
#define Switch_2 8
#define Status_Led 7

struct AQTIME {
  byte h1;
  byte m1;
  byte h2;
  byte m2;
  byte power;
};

// number of setups in memory
#define NBSETS 4
AQTIME ti[NBSETS];
byte out[NBSETS];

// statuses of outputs
#define OFF 0
#define AUTO 1
#define ON 2
#define MAX 3
byte out_m[NBSETS];

// for nice transition
const unsigned long transitionDuration = 10000;
unsigned int transitionSteps;
byte asked_l[NBSETS]; // new asked level
byte last_l[NBSETS];  // last asked level
unsigned int current_l[NBSETS]; // current level multiplied by 256 in order to avoid floating calculations
int incr_l[NBSETS];   // step increment level multiplied by 256 in order to avoid floating calcultations

#define LightSet 0
#define SwitchSet 2

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

  // Start up the OneWire library for DallasTemperature
  sensors.begin();
  sensors.getAddress(tempDeviceAddress, 0);
  sensors.setResolution(tempDeviceAddress, resolution);

  // Configures display
  // set up the number of columns and rows on the LCD 
  lcd.createChar(1, up);
  lcd.createChar(2, down);

  lcd.begin(cols, lines);
  
  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);

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

  if(currentMillis - previousCalculationMillis > calculationInterval) {
      // save lasted calculation millis
      previousCalculationMillis = currentMillis;  
  
      // does interval calculations
      calculations();
  }
  if(status == ST_DISPLAY) {
    // only once an interval
    if(currentMillis - previousDisplayMillis > displayInterval) {
      Serial.println("display interval");

      // save lasted display millis
      previousDisplayMillis = currentMillis;  
  
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

// switch the menu status
void chg_status()
{
  if(status == ST_DISPLAY) {
    lcd.blink();
    status = ST_MENU;
  }
  else {
    lcd.noBlink();
    status = ST_DISPLAY;
  }
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

  Serial.print("ANALOG READ: "); Serial.println(button);  

  
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
//  Serial.println("calculations");

  // getting the voltage reading from the temperature sensor
  temperatureC = sensors.getTempCByIndex(0); // this takes some time (about 200ms)  

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

// does the menu
void do_menu()
{
  int pressed_bt = -1;
  int menuline = 0;

  Serial.println("do menu---------------------------------");

  start_menu();

  do {
    Serial.print("not set button");
    Serial.print("button = ");
    Serial.print(pressed_bt);
    Serial.print(",  menuline = ");
    Serial.println(menuline);

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
       set_function(1);
       break;
     case 2:
       set_function(2);
       break;
     case 3:
       set_function(3, 0);
       break;
     case 4:
       set_function(4, 0);
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
** setting a entry in the menu
*/
void set_function(byte lnb, byte wpower)
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

  Serial.print("do set light---------------- Number: ");
  Serial.print(lnb);
  Serial.print(" --- with power: ");
  Serial.print(wpower);
  Serial.println("---");
  
  // make sure we are up tu date from EEPROM
  read_eeprom(place);
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
  val[12] = (wpower) ? power/10+'0' : ' ';
  val[13] = (wpower) ? power%10+'0' : ' ';
  val[14] = ' ';
  val[15] = ' ';
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write("Start Stop");
  if(wpower)
    lcd.write(" POW");

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
                pos = 10;
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
                pos = (wpower) ? 12 : 10;
                break;
              case 12:
                pos = (wpower) ? 13 : 10;
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
    power = (wpower) ? (val[12]-'0')*10+val[13]-'0' : 0;

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
void read_eeprom(byte place)
{
  int eelocate;
  eelocate = 2+place*5;
  ti[place].h1 = EEPROM.read(eelocate++);   
  ti[place].m1 = EEPROM.read(eelocate++);   
  ti[place].h2 = EEPROM.read(eelocate++);   
  ti[place].m2 = EEPROM.read(eelocate++);   
  ti[place].power = EEPROM.read(eelocate);   

}

// this displays the data on the screen: this function has to be rewritten and the call also. Do not need to redisplay everithing each second
void display_data()
{
  // Prints RTC Time on RTC
  now = RTC.now();
  
//  Serial.println("display data");

  // clean up the screen before printing
  lcd.clear();
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

void print2dec(int nb) { //this adds a 0 before single digit numbers
  if (nb >= 0 && nb < 10) {
    lcd.write('0');
  }
  lcd.print(nb);
}
