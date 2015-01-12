/*
 
 Output Tests
 
 (C) 1/2015 Flavius Bindea

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

const int Light_1=10;
const int Light_2=11;
const int Switch_1=9;
const int Switch_2=8;
const int Status_Led=7;
const int Internal_Led=13;

// Initial setup
void setup() 
{
  // setout leds
  pinMode(Light_1, OUTPUT);
  pinMode(Light_2, OUTPUT);
  pinMode(Switch_1, OUTPUT);
  pinMode(Switch_2, OUTPUT);
  pinMode(Status_Led, OUTPUT);
  pinMode(Internal_Led, OUTPUT);
}

/*
** Main loop
*/
void loop() 
{ 
  brightLed(Light_1);
  brightLed(Light_2);
  blinkLed(Switch_1);
  blinkLed(Switch_2);
  blinkLed(Status_Led);
}


void blinkLed(int led)
{
  toggleLed(Internal_Led);
  digitalWrite(led, true);  
  delay(1000);
  digitalWrite(led, false);  
}

void brightLed(int led)
{
  int brightness = 0;
  int d = 5;
  
  toggleLed(Internal_Led);
  while(brightness <= 255) {
    analogWrite(led, brightness++);
    delay(d);
  }
  toggleLed(Internal_Led);
  while(brightness > 0) {
    analogWrite(led, --brightness);
    delay(d);
  } 
}

void toggleLed(int led)
{
  static boolean status = true;
  digitalWrite(led, status);
  status = !status;
}

