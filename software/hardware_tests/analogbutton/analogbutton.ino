/*
 
 Analog button test
 
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

Based on http://linksprite.com/wiki/index.php5?title=16_X_2_LCD_Keypad_Shield_for_Arduino
and http://arduino-info.wikispaces.com/LCD-Pushbuttons

*/

// For buttons
const int buttonsPin = A1;
int bstate = 1024, blast = 1024;  // button state and button last state

// change value depending on your measurements
const int button1max = 75;    // reading should be 0, 75 threshold
const int button2min = 175;   // reading should be 258, from 175 to 325
const int button2max = 325;
const int button3min = 420;   // reading should be 516, from 420 to 580
const int button3max = 580;
const int button4min = 610;   // reading should be 684, from 610 to 740
const int button4max = 740;
const int button5min = 750;   // reading should be 828, from 750 to 900
const int button5max = 900;
const int buttonNONE = 920;   // reading should be 1023

// button types
#define BT_NONE 0
#define BT_SET 1
#define BT_LEFT 2
#define BT_RIGHT 3
#define BT_UP 4
#define BT_DOWN 5

// Initial setup
void setup() 
{
  Serial.begin(9600);
  Serial.println("Button test");
}

/*
** Main loop
*/
void loop() 
{
  bstate = read_button();
  Serial.print("pressed : ");
  Serial.println(bstate);
  Serial.print("last    : ");
  Serial.println(blast);

  if(bstate != blast) {
    blast = bstate;
    switch(bstate) {
      case BT_SET:
        Serial.println("Button: SET");
        break;
      case BT_LEFT:
        Serial.println("Button: LEFT");
        break;
      case BT_RIGHT:
        Serial.println("Button: RIGHT");
        break;
     case BT_UP:
        Serial.println("Button: UP");
        break;
     case BT_DOWN:
        Serial.println("Button: DOWN");
        break;
     case BT_NONE:
        Serial.println("Button: NONE");
        break;
     }
  }
  delay(100);
}

/*
** return button status if it has changed
*/
int read_button()
{
  int button;

  // read the buttons
  button = analogRead(buttonsPin);

  Serial.print("VALUE: "); Serial.println(button);
  
  if (button < button1max)
    bstate = BT_SET;
  else if (button >= button2min && button <= button2max)
    bstate = BT_LEFT;
  else if (button >= button3min && button <= button3max)
    bstate = BT_RIGHT;
  else if (button >= button4min && button <= button4max)
    bstate = BT_UP;
  else if (button >= button5min && button <= button5max)
    bstate = BT_DOWN;
  else if (button >= buttonNONE)
    bstate = BT_NONE;
  else
    bstate = 99; // we should never arrive here

  if(bstate == 99) {
    Serial.print("ERROR: "); Serial.println(button);
  }
  
  return(bstate);
}


