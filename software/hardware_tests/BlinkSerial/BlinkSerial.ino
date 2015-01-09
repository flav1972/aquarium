/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

// another led put outside 
int outled = 3;

// counter for serial printing
unsigned int i = 0;

// the setup routine runs once when you press reset:
void setup() {         
  // starts serial at 9600 bauds
  Serial.begin(9600);  
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  pinMode(outled, OUTPUT); 
  // writes a messages on serial
  Serial.println("Setup done!!!"); 
}

// the loop routine runs over and over again forever:
void loop() {
  Serial.println(i);    // write on serial
  i++;                  // increment
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(outled, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(500);               // wait for half a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(outled, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);               // wait for half a second
}
