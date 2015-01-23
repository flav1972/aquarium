/*
 * Debugging prototype
 */
#define Debug_RAM(X)   Serial.print(F(X " free RAM: ")); Serial.println(freeRAM())

// used for RTC
const int dayspermonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
RTC_DS1307 RTC;
DateTime now;

/****************************************************************************
 * I2C LCD setups
 */
// Define I2C Address where the PCF8574A is. Use i2c_scanner.ino to find it.
#define I2C_ADDR    0x27  // my big screen's address
//#define I2C_ADDR    0x3F  // my little screen's address

// if big screen (20x4) uncoment following line
#define BIGSCREEN

// pins as connected on the PCF8574A
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
  B00000,
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
  B00000
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
  B00000
};

const char ch_deg = 4;
byte deg_bitmap[8] = {
  B01110,
  B01010,
  B01110,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};

/*****************************************************************************
 * temperature reader based on DS18B20
 */
// Data wire is plugged into pin 4 on the Arduino
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

DeviceAddress tempDeviceAddress; // address of the Temperature sensor
int  resolution = 10;	// 10 bits means 0.25°C increments conversion duration
                      // is 187.5ms

float temperatureC;   // the temperature from the sensor
byte tempOk = 0;            // is the temperature read ok ?
unsigned long tempTimeLastRead = 0;   // last time temperature was read
const unsigned long tempTimeMax = 60*1000;  // temperature read is valid for this duration (ms)
float tempSetpoint = 22;		// the temperature we need
float tempTreshold = 0.5;   // the treshold
int tempOutput = -1;  // the switch on which the temperature is controled
byte tempStatus = 0;  // status of the temperature switch

//////////////////////////////////////////////////////////
// Waterflow using hall sensor
volatile int NbTopsFan; //measuring the rising edges of the signal
unsigned long FlowResetMillis; // last reset
int hallsensor = 2;    //The pin location of the sensor

/*****************************************************************************
 * Buttons reading constants and variables
 */
const int buttonsPin = A1;	  // Pin on which the buttons are connected
const unsigned long debounceDelay = 10;   // debouncing delay
const unsigned long repeatDelay = 300;    // auto button repeat delay

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
const unsigned long displayInterval = 1000;
// For looping calculation by interval
unsigned long previousCalculationMillis = 0; 
const unsigned long calculationInterval = 250;

/*****************************************************************************
 *  display settings
 */
// screen size
#ifdef BIGSCREEN
const byte cols = 20, lines = 4;
#else
const byte cols = 16, lines = 2;
#endif

// menu of status
const int menumin = 0;
const int menumax = 7;

const char* menu_entry[] = {
  "1.Set Date/Time ",
  "2.Light 1 setup ",
  "3.Light 2 setup ",
  "4.Temp setup    ",
  "5.Switch 1 setup",
  "6.Switch 2 setup",
  "7.Switch 3 setup",
  "8.Switch 4 setup"
};

const unsigned long menuTimeout = 15000; // exit from menu after XXX ms

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

// this struct contains the time schedule of a position
struct AQTIME {
  byte h1;
  byte m1;
  byte h2;
  byte m2;
  byte power;
};

// number of setups in memory
#define NBSETS 6
#define LIGHTSET 0  // position of first light
#define SWITCHSET 2 // position of first switch = NB of lights
AQTIME ti[NBSETS];  // for each setup (output) we have a timeschedule
const byte out[] = { Light_1, Light_2, Switch_1, Switch_2, Switch_3, Switch_4 };

#define NBSETS_old 4	// old value for backwards compability

// statuses of outputs
#define OFF 0
#define AUTO 1
#define ON 2
#define MAX 3
#define TMP 4
byte out_m[NBSETS];

// for nice transition
const unsigned long transitionDuration = 10000;
unsigned int transitionSteps;
unsigned int asked_l[SWITCHSET]; // new asked level (*256)
unsigned int current_l[SWITCHSET];  // current level multiplied by 
                                    //256 in order to avoid floating calculations
int incr_l; // step increment level multiplied by 256 in order to avoid floating calcultations
            // we are using only one increment for all transitions.
            // It is calculated to go from 0 to MAX in transitionDuration

// EEPROM signature for aquarium: they are stored in 0 and 1
const byte AQ_SIG1 = 45, AQ_SIG2 = 898;
const byte AQ_SIG2_old = 899; // old value of signature with only 4 sets
