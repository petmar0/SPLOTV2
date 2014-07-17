#include <SoftwareSerial.h>
#include <math.h>

char ID='A';

int gTempCmd  = 0b00000011;
int gHumidCmd = 0b00000101;

int spectrumStrobe=6;
int spectrumReset=7;
int dataPin=10;
int clockPin=11;
int triggerPin=12;
int micPowerPin=13;
int lightPowerPin=12;
int TRHPowerPin=9;


int spectrumPin=0;
int envelopePin=2;
int lightPin=3;

int Bands[7];
int level=0;

int T=0;
int RH=0;
int P=0;
int L=0;

int dly=1000;

int SPOTrx = 2; //Receive from SPOT (TXO pin on breakout) into pin 2 on Arduino
int SPOTtx = 3; //Transmit out to SPOT (RXI pin on breakout) from pin 3 on breakout
//SoftwareSerial(rxPin, txPin)
SoftwareSerial softSPOT(7, SPOTtx); //Dummy soft RX on 7 (not actually used), Soft TX out on SPOTtx

int onButton = 4; //Holding this pin low for more than 5 seconds will power up SPOT in Bluetooth discovery mode
int powerControl = 5; //This controls the voltage regulators on the SPOT breakout board, active high

int numberOfPowerOns = 0;
int unitStatus = 0; //0x00 = GPS/Radio powered off, 0x07 = GPS searching, 0x0F = GPS lock, 0x06 = transmitting to satellite network
int timeToNextCheckin = 0; //Starts at 0. In seconds Once GPS locks, will increase to a large number then count down to next try.
int sentTries = 0; //Starts at 0 and goes to 1, then to 2, then unit shuts down.
int gpsStatus = 0; //Goes to 0x01 once we have good GPS coords. GPS will then be powered down.
int satsInView = 0;

unsigned char SPOT_requestUnitID[] = { 
  0xAA, 0x03, 0x01 };

unsigned char SPOT_requestStatus[] = { 
  0xAA, 0x03, 0x52 };

unsigned char SPOT_requestLastGPS[] = { 
  0xAA, 0x03, 0x25 };

char buffer[200]; //Used for status sprintf things

int messageNumber = 1; //Let's add a # variable to the message we send out


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(spectrumReset, OUTPUT);
  pinMode(spectrumStrobe, OUTPUT);
  pinMode(13, OUTPUT);

  //Init spectrum analyzer
  digitalWrite(spectrumStrobe,LOW);
    delay(1);
  digitalWrite(spectrumReset,HIGH);
    delay(1);
  digitalWrite(spectrumStrobe,HIGH);
    delay(1);
  digitalWrite(spectrumStrobe,LOW);
    delay(1);
  digitalWrite(spectrumReset,LOW);
    delay(5);
  int tries;
  for(tries = 0 ; tries < 3 ; tries++){

    SPOTsetup(); //Init the IO pins for the SPOT
    SPOTcheckStatus(); //Check for a valid response

    if(unitStatus != 255) break; //255 = SPOT didn't respond

    Serial.print("SPOT failed to respond.");
  }

  if(tries == 3) { //Give up!
    Serial.print("Please check connections.");
    
    while(1){ //Infini-blink error
      digitalWrite(8, HIGH);
      delay(1000);
      digitalWrite(8, LOW);
      delay(1000);
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(micPowerPin, HIGH);
  digitalWrite(lightPowerPin, HIGH);
  digitalWrite(TRHPowerPin, HIGH);
  level=analogRead(envelopePin);
  L=analogRead(lightPin);
  delay(dly/2);
  
  for(int i=0; i<7; i++){
    Bands[i]=(analogRead(spectrumPin) + analogRead(spectrumPin) ) >>1; //Read twice and take the average by dividing by 2
    digitalWrite(spectrumStrobe,HIGH);
    digitalWrite(spectrumStrobe,LOW);
  }

  char cmd = 0;
  int ack;
  
  if(SPOTcheckStatus()) {

    Serial.print("[Unit Status:");
    if(unitStatus == 0) Serial.print("Waiting Cmd]");
    else if(unitStatus == 0x07) Serial.print("GPS Search]");
    else if(unitStatus == 0x0F) Serial.print("GPS Lock]");
    else if(unitStatus == 0x06) Serial.print("Xmit to Sats]");
    else Serial.print("No response]");
    if(gpsStatus == 0) Serial.print(" (No location)");
    else if(gpsStatus == 1) Serial.print(" (Location good)");

    sprintf(buffer, " [Sats:%02d] [Next Xmit:%02d] [Try#:%02d] [#ofPowerOns:%02d]", satsInView, timeToNextCheckin, sentTries, numberOfPowerOns);
    Serial.println(buffer);

  }
  else {
    Serial.println("SPOT failed to respond to status check");
    unitStatus = 0xFF; //Error mode
  }
  sendCommandSHT(gTempCmd, dataPin, clockPin);
  waitForResultSHT(dataPin);
  T = getData16SHT(dataPin, clockPin);
  skipCrcSHT(dataPin, clockPin);
  sendCommandSHT(gHumidCmd, dataPin, clockPin);
  waitForResultSHT(dataPin);
  RH = getData16SHT(dataPin, clockPin);
  skipCrcSHT(dataPin, clockPin);
  //Let's send a new message!
  if(unitStatus == 0) {

    String msgToSend =(ID + String(messageNumber) + "," + Bands[0] + "," + Bands[1] + "," + Bands[2] + "," + Bands[3] + "," + Bands[4] + "," + Bands[5] + "," + Bands[6] + "," + level + "," + T + "," + RH + "," + P + "," + L + "\n"); //This is the message to send
    Serial.println(msgToSend);

    int msgLength = 8 + msgToSend.length(); //There are 8 data bytes before the message begins 
    
    byte SPOTmsg[100];
    
    SPOTmsg[0] = 0xAA; //Preamble
    SPOTmsg[1] = msgLength; //Frame length
    SPOTmsg[2] = 0x26; //Transmit
    SPOTmsg[3] = 0x01; //0x01 = I'm ok, 0x04 = Help, 0x40 = Track?
    SPOTmsg[4] = 0x00; //Unknown
    SPOTmsg[5] = 0x01; //Unknown
    SPOTmsg[6] = 0x00; //Unknown
    SPOTmsg[7] = 0x01; //Unknown
    for(int x = 0 ; x < msgToSend.length() ; x++)
      SPOTmsg[8 + x] = msgToSend[x]; //Attach message onto frame
      
    //Serial.print("Message:");
    for(int i = 0 ; i < msgLength ; i++) {
      softSPOT.print(char(SPOTmsg[i])); //Push message request

      //sprintf(buffer, "%02X ", SPOTmsg[i]);
      //Serial.print(buffer);
    }
    //Serial.println();

    Serial.println("Message sent");
    
    messageNumber++; //Advance to the next message number
  }
  digitalWrite(micPowerPin, LOW);
  digitalWrite(lightPowerPin, LOW);
  digitalWrite(TRHPowerPin, LOW);
  delay(dly/2); //Check status every second
}
void SPOTsetup(void) {
  softSPOT.begin(115200); //Setup software TX on pin 3

  digitalWrite(onButton, HIGH);
  pinMode(onButton, OUTPUT);
  
  pinMode(SPOTrx, INPUT);
  digitalWrite(SPOTrx, HIGH);

  //This will power cycle the SPOT
  //Because we want the unit in a known reset state, I don't recommend shortening
  //the power off or on time without lots of testing with your own application
  Serial.println("Regulators off");
  digitalWrite(powerControl, LOW); //Turn off voltage regulators
  pinMode(powerControl, OUTPUT);
  delay(500);
  digitalWrite(powerControl, HIGH); //Turn on voltage regulators
  delay(500);
  Serial.println("Regulators on");

  //Next we hold the On button of the SPOT for 4 seconds
  Serial.println("Turning SPOT on");
  digitalWrite(onButton, LOW);
  //delay(3000); //Normal mode - won't work with this code
  delay(4000); //Bluetooth discovery mode

//  digitalWrite(onButton, HIGH);
  pinMode(onButton, INPUT); //Release on button

  delay(3500); //Wait for the main processor (MSP) to come online
  Serial.println("SPOT is now on");
}

//Check Status of the SPOT unit. This will load the global variables with the latest values for
//unit status, number of power ons, sent tries, sats in view, and time to checkin.
//You can check the status every 5 to 10ms, but normal is 1 to 10 seconds.
int SPOTcheckStatus(void) {
  byte SPOTresponse[50];
  
  //Send request to SPOT
  for(int i = 0 ; i < 3 ; i++)
    softSPOT.print(char(SPOT_requestStatus[i])); //Push message request at 115200bps
  
  cli();
  for(int x = 0 ; x < 32 ; x++) 
    SPOTresponse[x] = getByte_115200();
  sei();

  //For debugging
  Serial.print("Status Array: ");
  for(int x = 0 ; x < 32 ; x++) {
    sprintf(buffer, "%02X ", (byte)SPOTresponse[x]);
    Serial.print(buffer);
  }
  Serial.println();

  if(SPOTresponse[0] == 0xAA) { //We have found the header!

    numberOfPowerOns = SPOTresponse[6];

    unitStatus = SPOTresponse[7]; //0x00 = GPS/Radio powered off, 0x07 = GPS searching, 0x0F = GPS lock, 0x06 = transmitting to satellite network

    timeToNextCheckin = (SPOTresponse[11] << 8) | SPOTresponse[12];

    sentTries = SPOTresponse[19]; //Starts at 0 and goes to 1, then to 2, then unit shuts down.

    gpsStatus = SPOTresponse[26]; //Goes to 0x01 once we have good GPS coords. GPS will then be powered down.

    satsInView = SPOTresponse[31];
    
    return(1); //We got a good response
  }
  else {
    numberOfPowerOns = 0;

    unitStatus = 255; //0x00 = GPS/Radio powered off, 0x07 = GPS searching, 0x0F = GPS lock, 0x06 = transmitting to satellite network

    timeToNextCheckin = 0;

    sentTries = 0; //Starts at 0 and goes to 1, then to 2, then unit shuts down.

    gpsStatus = 0; //Goes to 0x01 once we have good GPS coords. GPS will then be powered down.

    satsInView = 0;

    return(0); //We didn't get a response in time!
  }
}

//This function receives characters at 115200bps
//SoftwareSerial doesn't support full Bandwidth 115200 so we have to bit-bang it
//This code only works at 16MHz. Use a logic analyzer to eval the timing (cut the cycles in half) if 
//you need to work at 8MHz.
//Modifying any part of this code will throw the timing off.
byte getByte_115200() {

  pinMode(8, OUTPUT); //Used to toggle a pin for time measurements

  int timeOut = 0;
  while((PIND & 1<<2)){
    delayMicroseconds(1); //Wait for the start of a new byte
    if(timeOut++ > 10000) return(0);
  }

  delay_us(2);

  byte myByte = 0;
  for(char x = 0 ; x < 8 ; x++) {
    delay_us(3); //On the first loop, this will wait for the middle of bit 1

    myByte >>= 1;

    PORTB &= ~(1<<0); //To measure timing on Pin8
    if(PIND & 1<<2) //SPOTrx on 2 is PD2 on the ATmega
      myByte |= 0b10000000;
    else
      myByte |= 0; //This is here for definitive number of cycles
  
    PORTB |= (1<<0); //To measure timing on Pin8
  }

  return myByte;
}

//This delay routine only works at 16MHz
//This has been carefully trimmed 
void delay_us(char x) {
  while(x--) {
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
  }
}

int shiftIn(int dataPin, int clockPin, int numBits)
{
   int ret = 0;
   int i;

   for (i=0; i<numBits; ++i)
   {
      digitalWrite(clockPin, HIGH);
      delay(10);  // I don't know why I need this, but without it I don't get my 8 lsb of temp
      ret = ret*2 + digitalRead(dataPin);
      digitalWrite(clockPin, LOW);
   }

   return(ret);
}

void sendCommandSHT(int command, int dataPin, int clockPin)
{
  int ack;

  // Transmission Start
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  digitalWrite(dataPin, HIGH);
  digitalWrite(clockPin, HIGH);
  digitalWrite(dataPin, LOW);
  digitalWrite(clockPin, LOW);
  digitalWrite(clockPin, HIGH);
  digitalWrite(dataPin, HIGH);
  digitalWrite(clockPin, LOW);
           
  // The command (3 msb are address and must be 000, and last 5 bits are command)
  shiftOut(dataPin, clockPin, MSBFIRST, command);

  // Verify we get the coorect ack
  digitalWrite(clockPin, HIGH);
  pinMode(dataPin, INPUT);
  ack = digitalRead(dataPin);
  if (ack != LOW)
    Serial.println("Ack Error 0");
  digitalWrite(clockPin, LOW);
  ack = digitalRead(dataPin);
  if (ack != HIGH)
     Serial.println("Ack Error 1");
}

void waitForResultSHT(int dataPin)
{
  int i;
  int ack;
  
  pinMode(dataPin, INPUT);
  
  for(i= 0; i < 100; ++i)
  {
    delay(10);
    ack = digitalRead(dataPin);

    if (ack == LOW)
      break;
  }
  
  if (ack == HIGH)
    Serial.println("Ack Error 2");
}

int getData16SHT(int dataPin, int clockPin)
{
  int val;
  
  // Get the most significant bits
  pinMode(dataPin, INPUT);
  pinMode(clockPin, OUTPUT);
  val = shiftIn(dataPin, clockPin, 8);
  val *= 256;

  // Send the required ack
  pinMode(dataPin, OUTPUT);
  digitalWrite(dataPin, HIGH);
  digitalWrite(dataPin, LOW);
  digitalWrite(clockPin, HIGH);
  digitalWrite(clockPin, LOW);
           
  // Get the lest significant bits
  pinMode(dataPin, INPUT);
  val |= shiftIn(dataPin, clockPin, 8);

  return val;
}

void skipCrcSHT(int dataPin, int clockPin)
{
  // Skip acknowledge to end trans (no CRC)
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  digitalWrite(dataPin, HIGH);
  digitalWrite(clockPin, HIGH);
  digitalWrite(clockPin, LOW);
}
