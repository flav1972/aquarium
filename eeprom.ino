// reads data from EEPROM
void read_eeprom(byte place)
{
  int eelocate;
  eelocate = 2+place*5;
  ti[place].h1 = EEPROM.read(eelocate++);
  ti[place].m1 = EEPROM.read(eelocate++);
  ti[place].h2 = EEPROM.read(eelocate++);
  ti[place].m2 = EEPROM.read(eelocate++);
  ti[place].power = EEPROM.read(eelocate);
}

// write data to EEPROM
void write_eeprom(byte place)
{
  int eelocate;
  eelocate = 2+place*5;
  EEPROM.write(eelocate++, ti[place].h1);
  EEPROM.write(eelocate++, ti[place].m1);
  EEPROM.write(eelocate++, ti[place].h2);
  EEPROM.write(eelocate++, ti[place].m2);
  EEPROM.write(eelocate++, ti[place].power);
}

