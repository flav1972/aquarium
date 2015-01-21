// used for RTC
const int dayspermonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
RTC_DS1307 RTC;
DateTime now;

// I2C LCD setups
#define I2C_ADDR    0x27  // Define I2C Address where the PCF8574A is. Use i2c_scanner.ino to find it.
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
LiquidCrystal_I2C	lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

/*****************************************************************************
 * characters bitmaps
 */
// 126: -> 127: <-
const char ch_right = 126;
const char ch_left = 127;

const char ch_up = 1;
byte up_bitmap[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
};

const char ch_down = 2;
byte down_bitmap[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
};

const char ch_set = 3;
byte set_bitmap[8] = {
  B10001,
  B01110,
  B01000,
  B01110,
  B00010,
  B01110,
  B10001,
};

const char ch_deg = 4;
byte deg_bitmap[8] = {
  B00000,
  B01110,
  B01010,
  B01110,
  B00000,
  B00000,
  B00000,
};

// temperature reader based on DS18B20
// Data wire is plugged into pin 4 on the Arduino
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

DeviceAddress tempDeviceAddress; // address of the Temperature sensor
int  resolution = 10; // 10 bits means 0.25°C increments conversion duration is 187.5ms

const float baselineTemp = 20.0;

float temperatureC;

//////////////////////////////////////////////////////////
// Waterflow using hall sensor
volatile int NbTopsFan; //measuring the rising edges of the signal
unsigned long FlowResetMillis; // last reset
int hallsensor = 2;    //The pin location of the sensor

/*****************************************************************************
 * Buttons reading constants and variables
 */
const int buttonsPin = A1;	  // Pin on which the buttons are connected
unsigned long debounceDelay = 10;   // debouncing delay
unsigned long repeatDelay = 300;    // auto button repeat delay

// change value depending on your measurements
const int button1max = 75;		// reading should be 0, 75 threshold
const int button2max = 250;   // reading should be 151, from 76 to 250
const int button3max = 430;   // reading should be 347, from 251 to 430
const int button4max = 606;   // reading should be 515, from 431 to 606
const int button5max = 850;   // reading should be 700, from 607 to 850

// button values
#define BT_NONE 0
#define BT_SET 1
#define BT_LEFT 2
#define BT_RIGHT 3
#define BT_UP 4
#define BT_DOWN 5

/*****************************************************************************
 * Defines for Loops
 */
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

const unsigned long menuTimeout = 15000; // exit from menu after XXX ms

/*
 * function prototypes
 */
void set_function(byte lnb, byte wpower=1);

/*
 * Define the devices
 */
#define Light_1 10
#define Light_2 11
#define Switch_1 9
#define Switch_2 8
#define Switch_3 7
#define Switch_4 6

#define Status_Led 13

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

