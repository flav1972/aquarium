/*
** return button status if it has changed
*/
int read_button()
{
  int button; // analog value
  int read_state; // reading translated

  static int buttonState = 0; // last button state
  
  // values for debouncing
  static int lastDebounceButtonState = 0;    // last button state for debouncing
  static unsigned long lastDebounceTime = 0; // last state time

  // values for auto repeat
  static unsigned long lastOutputTime = 0;  // last time button sent
  
  // read the buttons
  button = analogRead(buttonsPin);

//  Serial.print(F("ANALOG READ: ")); Serial.println(button);  
  
  if (button < button1max)
    read_state = BT_SET;	// Set button is pressed
  else if (button <= button2max)
    read_state = BT_LEFT;
  else if (button <= button3max)
    read_state = BT_RIGHT;
  else if (button <= button4max)
    read_state = BT_UP;
  else if (button <= button5max)
    read_state = BT_DOWN;
  else
    read_state = BT_NONE;	// No button is pressed

//  Serial.print(F("BUTTON VALUE: ")); Serial.println(read_state);

  // check to see if you just pressed the button
  // and you've waited
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (read_state != lastDebounceButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
    lastDebounceButtonState = read_state;
  }
  
  if ((millis() - lastDebounceTime) >= debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    
    // if button press has changed or if ti was pressed for long enough
    if(read_state != buttonState ||
      (millis() - lastOutputTime) >= repeatDelay) {
      buttonState = read_state;
      if(read_state <= 5) {
        lastOutputTime = millis();
//        Serial.print(F("BUTTON: status = ")); Serial.println(read_state);  
        if(read_state>0) {
          Serial.print(F("BUTTON: status = ")); Serial.println(read_state); 
        }
        return(read_state);
      }
    }
  }
  return(0);
}

// read blocking
int read_button_blocking()
{
  int i;

  Serial.println(F("read button blocking"));

  while((i = read_button()) == 0)
    delay(50);
    
  return i;
}


