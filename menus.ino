// does the menu
void do_menu()
{
  int pressed_bt;
  int menuline = 0;
  unsigned long lastEntry = millis();

  Serial.println(F("do menu---------------------------------"));

  start_menu();

  while(true) {
    pressed_bt = read_button();
    Serial.println(F("looping in menu"));
    Serial.print(F("button = "));
    Serial.print(pressed_bt);
    Serial.print(F(",  menuline = "));
    Serial.println(menuline);

    // if a button is pressed we get the time
    if(pressed_bt != 0) {
      lastEntry = millis();
    }
    // check if no button pressed for a while
    else if((millis() - lastEntry) > menuTimeout)
      break;

    // we exit the loop if we press set
    if(pressed_bt == BT_SET)
      break;
      
    switch(pressed_bt) {
      case BT_RIGHT:
        Serial.println(F("RIGHT button pressed calling do_menu_entry"));
        do_menu_entry(menuline);
        Serial.println(F("Return from do_menu_entry"));        
        start_menu();
        lastEntry = millis();
        break;
      case BT_UP:
        menuline--;
        break;
      case BT_DOWN:
        menuline++;
        break;
    }

    Serial.println(F("in do_menu after switch"));
    
    if(menuline < menumin)
      menuline = menumin;
    else if(menuline > menumax)
      menuline = menumax;

    lcd.setCursor(0, 1);
    lcd.write(menu_entry[menuline]);
    lcd.setCursor(15, 1); 
    Serial.println(F("in do_menu short pause"));
    delay(50);
  }

  Serial.println(F("SET button pressed or timeout"));
}

/*
 * Display static menu data
 */
void start_menu()
{
  Serial.println(F("->in start_menu"));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Menu: use "));
  lcd.write(ch_up);
  lcd.write(ch_down);
  lcd.write(ch_right);
  lcd.write(ch_set);
}

void do_menu_entry(int en)
{
  Serial.print(F("Do menu entry: start"));
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
      set_temperature();
      break;
    case 4:
      set_function(3, 0);
      break;
    case 5:
      set_function(4, 0);
      break;
    case 6:
      set_function(5, 0);
      break;
    case 7:
      set_function(6, 0);
      break;
  }
  Serial.println(F("Do menu entry: end"));
}

/*
** Menu entry to setup the time
*/
void set_time()
{
  int pressed_bt;
  unsigned long lastEntry = millis();
  int pos = 0, v;
  char val[16];
  int day, month, year, hour, min;
  int i;
  int ok = 0;
  // table for cursor move
  // initial position                0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
  const byte move_right[] PROGMEM = {1, 3, 3, 4, 8, 8, 8, 8, 9,11, 8,12,14,15,15,15};
  const byte move_left[] PROGMEM =  {0, 0, 0, 1, 3, 3, 3, 3, 4, 8, 8, 9,11,11,12,14};

  Serial.println(F("do set time---------------------------------"));
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
  lcd.print(F("Time: use"));
  lcd.write(1);
  lcd.write(2);
  lcd.write(127);
  lcd.write(126);
  lcd.print(F("SET"));

  lcd.setCursor(0, 1);
  for(i = 0; i < 16; i++)
    lcd.print(val[i]);

  do {
    while(true) {
      pressed_bt = read_button();
      Serial.println(F("looping in set_time"));
      Serial.print(F("button = "));
      Serial.println(pressed_bt);
      
      // if a button is pressed we get the time
      if(pressed_bt != 0) {
        lastEntry = millis();
      }
      // check if no button pressed for a while
      else if((millis() - lastEntry) > menuTimeout) {
        Serial.println(F("set_time timed out"));
        return;
      }
      
      // we exit the loop if we press set
      if(pressed_bt == BT_SET)
        break;

      Serial.println(F("not set button"));
      switch(pressed_bt) {
        case BT_LEFT:
          pos = move_left[pos];
          break;
        case BT_RIGHT:
          pos = move_right[pos];
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
      Serial.println(F("set_time loop: short delay"));
      delay(50);
    }
    Serial.println(F("set_time end of readkeyloop: checking time is valid"));
    day = (val[0] - '0')*10+val[1]-'0';
    month = (val[3]-'0')*10+val[4]-'0';
    year = (val[8]-'0')*10+val[9]-'0';
    hour = (val[11]-'0')*10+val[12]-'0';
    min = (val[14]-'0')*10+val[15]-'0';
    Serial.print(F("day:"));
    Serial.println(day);
    Serial.print(F("month:"));
    Serial.println(month);
    Serial.print(F("year:"));
    Serial.println(year);
    Serial.print(F("hour:"));
    Serial.println(hour);
    Serial.print(F("min:"));
    Serial.println(min);

    if(min >= 0 && min < 60
      && hour >= 0 && hour < 24
      && month >= 1 && month <= 12
      && year >= 0 && year <= 99 
      && day >= 0 && day <= dayspermonth[month-1])
              ok = 1;
  } while(!ok);  
  Serial.println(F("set_time: saving new time end exit"));
  RTC.adjust(DateTime(year, month, day, hour, min, 0));
}

/*
** menu entry to settup a time schedule 
*/
void set_function(byte lnb, byte wpower)
{
  int place, eelocate;
  int pressed_bt;
  unsigned long lastEntry = millis();
  int pos = 0, v;
  char val[16];
  byte h1, m1, h2, m2, power;
  int i;
  int ok = 0;
  // table for cursor move
  // initial position                0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
  const byte move_right[] PROGMEM = {1, 3, 3, 4, 6, 6, 7, 9, 9,10,12,12,13,13,13,13};
  const byte move_left[] PROGMEM =  {0, 0, 1, 1, 3, 4, 4, 6, 7, 7, 9,10,10,12,12,12};

  Serial.print(F("do set light---------------- Number: "));
  Serial.print(lnb);
  Serial.print(F(" --- with power: "));
  Serial.print(wpower);
  Serial.println(F("---"));
  
  // calculates positions in EEPROM
  place = lnb - 1;
  eelocate = 2+place*5;
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
  ** HH:MM HH:MM XXX
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
  lcd.print(F("Start Stop"));
  if(wpower)
    lcd.print(F(" POW"));

  lcd.setCursor(0, 1);
  for(i = 0; i < 16; i++)
    lcd.print(val[i]);

  do {
    while(true) {
      pressed_bt = read_button();
      Serial.println(F("looping in set_function"));
      Serial.print(F("button = "));
      Serial.println(pressed_bt);
      
      // if a button is pressed we get the time
      if(pressed_bt != 0) {
        lastEntry = millis();
      }
      // check if no button pressed for a while
      else if((millis() - lastEntry) > menuTimeout) {
        Serial.println(F("set_function timed out"));
        return;
      }
      
      // we exit the loop if we press set
      if(pressed_bt == BT_SET)
        break;

      Serial.println(F("not set button"));
      switch(pressed_bt) {
        case BT_LEFT:
          pos = move_left[pos];
          break;
        case BT_RIGHT:
          pos = move_right[pos];
          break;
        case BT_UP:
          val[pos]++;
          break;
        case BT_DOWN:
          val[pos]--;
          break;
      }

      // if we do not display power max position is 10
      if(!wpower && pos>10)
        pos = 10;
        
      if(val[pos] < '0')
        val[pos] = '0';
      else if (val[pos] > '9')
        val[pos] = '9'; 
        
      lcd.setCursor(pos, 1);
      lcd.print(val[pos]);
      lcd.setCursor(pos, 1);
      Serial.println(F("set_function loop: short delay"));
      delay(50);
    }
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
  Serial.println(F("set_function: saving new timings end exit"));
}

/*
** menu entry to settup the temperature
*/
void set_temperature()
{
  //int place, eelocate;
  int pressed_bt;
  unsigned long lastEntry = millis();
  int pos = 0, v;
  char val[16];
  byte TempI, TempD, TresI, TresD, Swi;
  int i;
  int ok = 0;
  //place = lnb - 1;
  //eelocate = 2+place*5;

  // table for cursor move
  // initial position        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13
  const byte move_right[] = {1, 3, 3, 7, 7, 7, 7, 9, 9,13,13,13,13,13};
  const byte move_left[] =  {0, 0, 1, 1, 3, 3, 3, 3, 7, 7, 9, 9, 9, 9};

  Serial.println(F("do set temperature----------------"));
  Debug_RAM("set_temperature start");

  // make sure we are up tu date from EEPROM
//Todo
// using globals for starting
//float tempSetpoint = 24.5;		// the temperature we need
//float tempTreshold = 1.2;   // the treshold
//byte tempOutput = -1;  // the switch on which the temperature is controled
  TempI = tempSetpoint;
  TempD = ((int)(tempSetpoint*10))%10;
  TresI = tempTreshold;
  TresD = ((int)(tempTreshold*10))%10;
  Swi = (tempOutput == -1) ? 0 : tempOutput-SwitchSet+1;
  val[0] = TempI/10+'0';
  val[1] = TempI%10+'0';
  val[2] = '.';
  val[3] = TempD+'0';
  val[4] = ch_deg;
  val[5] = 'C';
  val[6] = ' ';
  val[7] = TresI%10+'0';
  val[8] = '.';
  val[9] = TresD+'0';
  val[10] = ch_deg;
  val[11] = 'C';
  val[12] = ' ';
  val[13] = Swi+'0';
  val[14] = ' ';
  val[15] = ' ';
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Temp   Tresh Switch"));

  lcd.setCursor(0, 1);
  for(i = 0; i < 16; i++)
    lcd.write(val[i]);

  do {
    while(true) {
      pressed_bt = read_button();
      Serial.println(F("looping in set_temperature"));
      Serial.print(F("button = "));
      Serial.println(pressed_bt);
      
      // if a button is pressed we get the time
      if(pressed_bt != 0) {
        lastEntry = millis();
      }
      // check if no button pressed for a while
      else if((millis() - lastEntry) > menuTimeout) {
        Serial.println(F("set_temperature timed out"));
        return;
      }
      
      // we exit the loop if we press set
      if(pressed_bt == BT_SET)
        break;

      Serial.println(F("not set button"));
      switch(pressed_bt) {
        case BT_LEFT:
          pos = move_left[pos];
          break;
        case BT_RIGHT:
          pos = move_right[pos];
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
      Serial.println(F("set_temperature loop: short delay"));
      delay(50);
    }
    TempI = (val[0]-'0')*10+val[1]-'0';
    TempD = (val[3]-'0');
    TresI = (val[7]-'0');
    TresD = (val[9]-'0');
    Swi = val[13]-'0';

    // check if values are correct
    if(Swi <= NBSETS-SwitchSet)
      ok = 1;
  } while(!ok);  
  tempSetpoint = TempI + TempD/10.0;
  tempTreshold = TresI + TresD/10.0;
  tempOutput = (Swi == 0) ? -1 : Swi+SwitchSet-1;
  Serial.print(F("Temp values: tempSetpoint="));
  Serial.print(tempSetpoint);
  Serial.print(F(" tempTreshold="));
  Serial.print(tempTreshold);
  Serial.print(F(" tempOutput="));
  Serial.println(tempOutput);  
  Serial.println(F("set_temperature: saving new timings end exit"));
}
