// function to return flow
void get_flow() 
{
  int Calc;
  unsigned long duration;
  
  detachInterrupt(0);
  Serial.print(F("NbTopFan="));Serial.println(NbTopsFan);
  duration = millis()-FlowResetMillis;
  Serial.print(F("Duration="));Serial.println(duration);
  // Calc = (NbTopsFan * 60 / 5.5);    // POW110D3B: (Pulse frequency x 60) / 5.5Q, = flow rate in L/hour 
  Calc = 612.2 * NbTopsFan/duration;  // YF-S401: F = 98 * Q in L/min converted to L/h => 60 * tops/(98*millis/1000) = Q in L/h => Q(in L/h) =  612.2 * tops/millis
  NbTopsFan = 0;   //Set NbTops to 0 ready for calculations
  FlowResetMillis = millis();
  attachInterrupt(0, rpm, RISING); //and the interrupt is attached
  Serial.print(F("Calc="));Serial.println(Calc);
}

// interrupt function
void rpm ()     //This is the function that the interupt calls 
{ 
  NbTopsFan++;  //This function measures the rising and falling edge of the hall effect sensors signal
} 


