// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// Bases on demo of http://milesburton.com/index.php?title=Dallas_Temperature_Control_Library
// Updates by Flavius Bindea

#include <OneWire.h>
#include <DallasTemperature.h>
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;
boolean led_status = true; // used to toggle led status

// Data wire is plugged into pin 4 on the Arduino
#define ONE_WIRE_BUS 4
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
 
void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");

  // Start up the library
  sensors.begin();
}
 
 
void loop(void)
{
  digitalWrite(led, led_status);   // turn on/off the LED 
  led_status = !led_status;

  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print(" Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.print("DONE.  ");

  Serial.print("Temperature for Device 1 is: ");
  Serial.println(sensors.getTempCByIndex(0)); // Why "byIndex"? 
    // You can have more than one IC on the same bus. 
    // 0 refers to the first IC on the wire
  
  // pause for 1 second
  delay(1000);
}

