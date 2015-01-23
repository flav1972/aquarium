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

/**
 * read temperature setups to EEPROM
 */
void read_eeprom_temp()
{
  int eelocate;
  eelocate = 2+NBSETS*5;
  
  eelocate = read_eeprom_float(eelocate, &tempSetpoint);
  eelocate = read_eeprom_float(eelocate, &tempTreshold);
  read_eeprom_int(eelocate, &tempOutput);
}

/**
 * write temperature setups to EEPROM
 */
void write_eeprom_temp()
{
  int eelocate;
  eelocate = 2+NBSETS*5;
  
  eelocate = write_eeprom_float(eelocate, tempSetpoint);
  eelocate = write_eeprom_float(eelocate, tempTreshold);
  write_eeprom_int(eelocate, tempOutput);
}

/**
 * read transitionDuration setups to EEPROM
 */
void read_eeprom_fading()
{
  int eelocate;
  eelocate = 2+NBSETS*5+2*sizeof(float)+sizeof(int);
  
  read_eeprom_ulong(eelocate, &transitionDuration);
}

/**
 * write transitionDuration setups to EEPROM
 */
void write_eeprom_fading()
{
  int eelocate;
  eelocate = 2+NBSETS*5+2*sizeof(float)+sizeof(int);
  
  write_eeprom_ulong(eelocate, transitionDuration);
}

/**
 * Reads float from eeprom
 * @param[in] eelocate position in eeprom where the var is stored
 * @param[out] f pointer to where the float has to be stored
 * @return the position of next value in eeprom
 */
int read_eeprom_float(int eelocate, float *value)
{
  byte* p = (byte*)(void*)value;
  int i;
  
  for (i = 0; i < sizeof(float); i++)
    *p++ = EEPROM.read(eelocate++);
    
  return eelocate;
 }   
    
/**
 * Write float to eeprom
 * @param eelocate position in eeprom where the float has to be stored
 * @param f pointer to the float to be read
 * @return the position of next value in eeprom
 */
int write_eeprom_float(int eelocate, float value)
{
  const byte* p = (const byte*)(const void*)&value;
  int i;
 
  for (i = 0; i < sizeof(float); i++)
    EEPROM.write(eelocate++, *p++);
  
  return eelocate;
}

/**
 * Reads int from eeprom
 * @param[in] eelocate position in eeprom where the var is stored
 * @param[out] value pointer to where the int has to be stored
 * @return the position of next value in eeprom
 */
int read_eeprom_int(int eelocate, int *value)
{
  byte* p = (byte*)(void*)value;
  int i;
  
  for (i = 0; i < sizeof(int); i++)
    *p++ = EEPROM.read(eelocate++);
    
  return eelocate;
 }   
    
/**
 * Write int to eeprom
 * @param[in] eelocate position in eeprom where the int has to be stored
 * @param[in] value pointer to the int to be read
 * @return the position of next value in eeprom
 */
int write_eeprom_int(int eelocate, int value)
{
  const byte* p = (const byte*)(const void*)&value;
  int i;
 
  for (i = 0; i < sizeof(int); i++)
    EEPROM.write(eelocate++, *p++);
  
  return eelocate;
}

/**
 * Reads unsigned long from eeprom
 * @param[in] eelocate position in eeprom where the var is stored
 * @param[out] value pointer to where the unsigned long has to be stored
 * @return the position of next value in eeprom
 */
int read_eeprom_ulong(int eelocate, unsigned long *value)
{
  byte* p = (byte*)(void*)value;
  int i;
  
  for (i = 0; i < sizeof(unsigned long); i++)
    *p++ = EEPROM.read(eelocate++);
    
  return eelocate;
 }   
    
/**
 * Write unsigned long to eeprom
 * @param[in] eelocate position in eeprom where the unsigned long has to be stored
 * @param[in] value pointer to the unsigned long to be read
 * @return the position of next value in eeprom
 */
int write_eeprom_ulong(int eelocate, unsigned long value)
{
  const byte* p = (const byte*)(const void*)&value;
  int i;
 
  for (i = 0; i < sizeof(unsigned long); i++)
    EEPROM.write(eelocate++, *p++);
  
  return eelocate;
}
