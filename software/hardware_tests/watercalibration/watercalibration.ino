/*
 * This scripts can be used to mesure the constants for a waterflow sensor
 * it will display on the serial the number of ticks and the duration since the first tick
 */
 
volatile int NbTopsFan = 0; //measuring the rising edges of the signal
int last_t=0;
int new_t=0;
int hallsensor = 2;    //The pin location of the sensor
unsigned long start=0;

void rpm ()     //This is the function that the interupt calls 
{ 
  NbTopsFan++;  //This function measures the rising and falling edge of the hall effect sensors signal
} 

// The setup() method runs once, when the sketch starts
void setup() //
{ 
  pinMode(hallsensor, INPUT); //initializes digital pin 2 as an input
  Serial.begin(57600); //This is the setup function where the serial port is initialised,
  attachInterrupt(0, rpm, RISING); //and the interrupt is attached
  Serial.println("Waiting flow...");
} 

// the loop() method runs over and over again,
// as long as the Arduino has power
void loop ()    
{
  cli();      //Disable interrupts 
  new_t = NbTopsFan;
  sei(); // Enable interrupts 
  if(start == 0 && new_t > 0) {
    start = millis();
  }
  if(new_t > last_t) {
    last_t = new_t;
    Serial.print(millis()-start); // Prints ms since start
    Serial.print(" ms: ");
    Serial.print(new_t, DEC);
    Serial.println(" ticks");
  }
  delay(100);   //Wait 100 milis
}
