// script take from: http://www.seeedstudio.com/wiki/index.php?title=G1/2_Water_Flow_sensor
// reading liquid flow rate using Seeeduino and Water Flow Sensor from Seeedstudio.com
// Code adapted by Charles Gantt from PC Fan RPM code written by Crenn @thebestcasescenario.com
// http:/themakersworkbench.com http://thebestcasescenario.com http://seeedstudio.com

/* For YF-S401 the values are:
The inner diameter 1.5mm. (Black) Frequency: F = 98 * Q (Q is flow L / min) error: ± 5 stream End liters of water output 5880 pulse

Frequency calculation = constant * unit flow (L / min) * time (seconds)

Flow Range :0.3-6L / min  

In order to avoid the instantaneous current is too large, burned flow sensor,please use DC3.5V-12V voltage.
*/


 
volatile int NbTopsFan; //measuring the rising edges of the signal
int Calc;                               
int hallsensor = 2;    //The pin location of the sensor
 
void rpm ()     //This is the function that the interupt calls 
{ 
  NbTopsFan++;  //This function measures the rising and falling edge of the hall effect sensors signal
} 

// The setup() method runs once, when the sketch starts
void setup() //
{ 
  pinMode(hallsensor, INPUT); //initializes digital pin 2 as an input
  Serial.begin(9600); //This is the setup function where the serial port is initialised,
  attachInterrupt(0, rpm, RISING); //and the interrupt is attached
} 

// the loop() method runs over and over again,
// as long as the Arduino has power
void loop ()    
{
  NbTopsFan = 0;   //Set NbTops to 0 ready for calculations
  sei();      //Enables interrupts
  delay (1000);   //Wait 1 second
  cli();      //Disable interrupts
  // Calc = (NbTopsFan * 60 / 5.5);    // POW110D3B: (Pulse frequency x 60) / 5.5Q, = flow rate in L/hour 
  Calc = (NbTopsFan * 60 / 98);  // YF-S401: F = 98 * Q in L/min converted to L/h
  Serial.print (Calc, DEC); //Prints the number calculated above
  Serial.print (" L/hour\r\n"); //Prints "L/hour" and returns a  new line
}
