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
          switch(pos) {
              case 1:
                pos = 0;
                break;
              case 3:
                pos = 1;
                break;
              case 4:
                pos = 3;
                break;
              case 8:
                pos = 4;
                break;
              case 9:
                pos = 8;
                break;
              case 11:
                pos = 9;
                break;
              case 12:
                pos = 11;
                break;
              case 14:
                pos = 12;
                break;
              case 15:
                pos = 14;
                break;
          }
          break;
        case BT_RIGHT:
          switch(pos) {
              case 0:
                pos = 1;
                break;
              case 1:
                pos = 3;
                break;
              case 3:
                pos = 4;
                break;
              case 4:
                pos = 8;
                break;
              case 8:
                pos = 9;
                break;
              case 9:
                pos = 11;
                break;
              case 11:
                pos = 12;
                break;
              case 12:
                pos = 14;
                break;
              case 14:
                pos = 15;
                break;
          }
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
  place = lnb - 1;
  eelocate = 2+place*5;

  Serial.print(F("do set light---------------- Number: "));
  Serial.print(lnb);
  Serial.print(F(" --- with power: "));
  Serial.print(wpower);
  Serial.println(F("---"));
  
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
  ** HH:MM HH:MM XX
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
          switch(pos) {
              case 1:
                pos = 0;
                break;
              case 3:
                pos = 1;
                break;
              case 4:
                pos = 3;
                break;
              case 6:
                pos = 4;
                break;
              case 7:
                pos = 6;
                break;
              case 9:
                pos = 7;
                break;
              case 10:
                pos = 9;
                break;
              case 12:
                pos = 10;
                break;
              case 13:
                pos = 12;
                break;
          }
          break;
        case BT_RIGHT:
          switch(pos) {
              case 0:
                pos = 1;
                break;
              case 1:
                pos = 3;
                break;
              case 3:
                pos = 4;
                break;
              case 4:
                pos = 6;
                break;
              case 6:
                pos = 7;
                break;
              case 7:
                pos = 9;
                break;
              case 9:
                pos = 10;
                break;
              case 10:
                pos = (wpower) ? 12 : 10;
                break;
              case 12:
                pos = (wpower) ? 13 : 10;
                break;
          }
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
** menu entry to settup a temperature
*/
/*
void set_temperature(byte lnb, byte wpower)
{
  int place, eelocate;
  int pressed_bt;
  unsigned long lastEntry = millis();
  int pos = 0, v;
  char val[16];
  byte h1, m1, h2, m2, power;
  int i;
  int ok = 0;
  place = lnb - 1;
  eelocate = 2+place*5;

  Serial.print(F("do set light---------------- Number: "));
  Serial.print(lnb);
  Serial.print(F(" --- with power: "));
  Serial.print(wpower);
  Serial.println(F("---"));
  
  // make sure we are up tu date from EEPROM
  read_eeprom(place);
  h1 = ti[place].h1;   
  m1 = ti[place].m2;   
  h2 = ti[place].h2;   
  m2 = ti[place].m2;   
  power = ti[place].power;   

  
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
  lcd.write(F("Start Stop"));
  if(wpower)
    lcd.write(F(" POW"));

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
          switch(pos) {
              case 1:
                pos = 0;
                break;
              case 3:
                pos = 1;
                break;
              case 4:
                pos = 3;
                break;
              case 6:
                pos = 4;
                break;
              case 7:
                pos = 6;
                break;
              case 9:
                pos = 7;
                break;
              case 10:
                pos = 9;
                break;
              case 12:
                pos = 10;
                break;
              case 13:
                pos = 12;
                break;
          }
          break;
        case BT_RIGHT:
          switch(pos) {
              case 0:
                pos = 1;
                break;
              case 1:
                pos = 3;
                break;
              case 3:
                pos = 4;
                break;
              case 4:
                pos = 6;
                break;
              case 6:
                pos = 7;
                break;
              case 7:
                pos = 9;
                break;
              case 9:
                pos = 10;
                break;
              case 10:
                pos = (wpower) ? 12 : 10;
                break;
              case 12:
                pos = (wpower) ? 13 : 10;
                break;
          }
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

*/