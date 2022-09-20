// Case_Cart_Boiler_1.v0

#include <SD.h>
#include <ModbusMaster.h>

File myFile;

const int low1 = 5;
const int low2 = 6;
const int alarmPin = 8;
const int MAX485_DE = 3;
const int MAX485_RE_NEG = 2;
const int SIMpin = A3;
const int debounceInterval = 3000;
int primaryCutoff;
int counter1;
int secondaryCutoff;
int counter2;
int alarm;
int counter3;
int hlpcIN = 14;
int hlpcOUT = 15;
int hlpcCOMMON;
int hlpcNC;
int counter4;

//char urlHeaderArray[] = "AT+HTTPPARA=\"URL\",\"http://relay-post-8447.twil.io/secondary-low-water?";
//char contactFromArray1[] = "From=%2b19049808059&";
//char contactToArray1[] = "To=%2b17065755866&";

char SetCombody[] = "Body=SquawkBox%20Setup%20Complete\"\r";
char LWbody[] = "Body=Primary%20Low%20Water\"\r";
char LW2body[] = "Body=Secondary%20Low%20Water\"\r";
char REPbody[] = "Body=CaseCart%20Routine%20Timer\"\r";
char HLPCbody[] = "Body=High%20Pressure%20Alarm\"\r";
char CHECKbody[] = "Body=Good%20Check\"\r";
char BCbody[] = "Body=Boiler%20Down\"\r";

String URLheader = "";
String conFrom1 = "";
String conTo1 = "";
String conTo2 = "";
String conTo3 = "";
String conTo4 = "";

char contactFromArray1[25];
char contactToArray1[25];
char contactToArray2[25];
char contactToArray3[25];
char contactToArray4[25];
char urlHeaderArray[100];
unsigned char data = 0;
char incomingChar = "";

unsigned long currentMillis = 0;
unsigned long difference = 0;
unsigned long difference2 = 0;
unsigned long difference3 = 0;
unsigned long difference4 = 0;
unsigned long difference5 = 0;
unsigned long dailytimer = 43200000;
unsigned long msgtimer1 = 0;
unsigned long alarmTime = 0;
unsigned long alarmTime2 = 0;
unsigned long alarmTime3 = 0;
unsigned long alarmTime4 = 0;

bool alarmSwitch = false;
bool alarmSwitch2 = false;
bool alarmSwitch3 = false;
bool alarmSwitch4 = false;
bool msgswitch = false;

ModbusMaster node;

void setup() 
{
  Serial.begin(9600);
  Serial1.begin(19200);
  Serial.println(F("This is Case_Cart_Boiler_1.v0 sketch."));

  pinMode(low1, INPUT);
  pinMode(low2, INPUT);
  pinMode(alarmPin, INPUT);
  pinMode(hlpcIN, INPUT);
  pinMode(hlpcOUT, INPUT);
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  pinMode(SIMpin, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  node.begin(1, Serial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  SIMboot();
  // Give time to your GSM shield log on to network
  delay(15000);
  loadContacts();
  Serial.println(F("Contacts Loaded.  Booting SIM module.  Initiating wakeup sequence..."));
  delay(2000);
  //PUT SIM MODULE WAKEUP HERE
  Serial.println("Hey!  Wake up!");
  Serial1.print("AT\r"); //Manufacturer identification
  delay(50);
  Serial1.print("AT\r"); //Manufacturer identification
  delay(50);
  Serial1.print("AT\r"); //Manufacturer identification
  delay(50);
  Serial1.print("AT\r"); //Manufacturer identification
  delay(50);
  Serial1.print("AT\r"); //Manufacturer identification
  delay(50);
  //SIM MODULE SETUP---
  Serial1.print("AT+CGDCONT=1,\"IP\",\"super\"\r");
  delay(500);
  Serial1.print("AT+COPS=1,2,\"310410\"\r");
  delay(5000);
  Serial1.print("AT+SAPBR=3,1,\"APN\",\"super\"\r");
  delay(3000);
  Serial1.print("AT+SAPBR=1,1\r");
  delay(2000);
  Serial1.print("AT+CMGD=0,4\r");
  delay(100);
  Serial1.print("AT+CMGF=1\r");
  //PUT TEST MESSAGE HERE
  delay(100);
  Serial1.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);
  sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, SetCombody);
  //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, SetCombody);
  //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, SetCombody);
  //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, SetCombody);
  Serial.println(F("Setup complete. Entering main loop"));
}

void loop()
{
  primaryCutoff = digitalRead(low1);
  secondaryCutoff = digitalRead(low2);
  alarm = digitalRead(alarmPin);
  hlpcCOMMON = digitalRead(hlpcIN);
  hlpcNC = digitalRead(hlpcOUT);
  currentMillis = millis();

  //resetCounters();
  primary_LW();
  secondary_LW();
  Honeywell_alarm();
  HLPC();
  timedmsg();
  SMSRequest();
}

// void resetCounters()
// {
//   if (primaryCutoff == LOW)
//   {
//     alarmSwitch = false;
//     difference = 0;
//     alarmTime = 0;
//     counter1 = 0;
//   }
//   if (secondaryCutoff == LOW)
//   {
//     alarmSwitch2 = false;
//     difference2 = 0;
//     alarmTime2 = 0;
//     counter2 = 0;
//   }
//   if (alarm == LOW)
//   {
//     alarmSwitch3 = false;
//     counter3 = 0;
//     difference3 = 0;
//     alarmTime3 = 0;
//   }
//   if ((hlpcCOMMON == HIGH) && (hlpcNC == HIGH))
//   {
//     alarmSwitch4 = false;
//     difference4 = 0;
//     alarmTime4 = 0;
//     counter4 = 0;
//   }
//   //this next line may not be necessary, but I think it will help prevent against false alarms on HLPC
//   if (hlpcCOMMON == LOW)
//   {
//     counter4 = 1;
//   }
// }

void primary_LW()
{
  if ((primaryCutoff == HIGH) && (counter1 == 0))
  {
    if (alarmSwitch == false)
    {
      alarmTime = currentMillis;
      alarmSwitch = true;
      Serial.println("alarmSwitch is true");
    }
    difference = currentMillis - alarmTime;

    if ( difference >= debounceInterval)
    {
      Serial.println(F("Primary low water.  Sending message"));
      sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, LWbody);
      //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, LWbody);
      //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, LWbody);
      //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, LWbody);

      Serial.println(F("message sent or simulated"));
      counter1 = 1;
      difference = 0;
      alarmSwitch = false;
      alarmTime = 0;
    }
    if (difference < debounceInterval)
    {
      Serial.println(difference);
      return;
    }
  }
  else
  {
    if ((primaryCutoff == LOW) && (counter1 == 1))
    {
      alarmSwitch = false;
      difference = 0;
      alarmTime = 0;
      counter1 = 0;
      return;
    }
  }
}

void secondary_LW()
{
  if ((secondaryCutoff == HIGH) && (counter2 == 0))
  {
    if (alarmSwitch2 == false)
    {
      alarmTime2 = currentMillis;
      alarmSwitch2 = true;
      Serial.println(F("alarmSwitch2 is true"));
    }
    difference2 = currentMillis - alarmTime2;

    if ( difference2 >= debounceInterval)
    {
      Serial.println(F("Secondary low water.  Sending message."));
      sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, LW2body);
      //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, LW2body);
      //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, LW2body);
      //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, LW2body);
      Serial.println(F("message sent or simulated"));
      counter2 = 1;
      difference2 = 0;
      alarmSwitch2 = false;
      alarmTime2 = 0;
    }
    if (difference2 < debounceInterval)
    {
      Serial.println(difference2);
      return;
    }
  }
  else
  {
    if ((secondaryCutoff == LOW) && (counter2 == 1))
    {
      alarmSwitch2 = false;
      difference2 = 0;
      alarmTime2 = 0;
      counter2 = 0;
      return;
    }
  }
}

void Honeywell_alarm()
{
  if ((alarm == HIGH) && (counter3 == 0))
  {
    if (alarmSwitch3 == false)
    {
      alarmTime3 = currentMillis;
      alarmSwitch3 = true;
      Serial.println("alarmSwitch is true");
    }
    difference3 = currentMillis - alarmTime3;

    if ( difference3 >= debounceInterval)
    {
      Serial.println(F("sending alarm message"));
      sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, BCbody);
      //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, BCbody);
      //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, BCbody);
      //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, BCbody);
      Serial.println(F("about to enter modbus reading function..."));
      readModbus();
      Serial.println(F("message sent or simulated"));
      counter3 = 1;
      difference3 = 0;
      alarmSwitch3 = false;
      alarmTime3 = 0;
    }
    if (difference3 < debounceInterval)
    {
      Serial.println(difference3);
      return;
    }
  }
  else
  {
    if ((alarm == LOW) && (counter3 == 1))
    {
      alarmSwitch3 = false;
      difference3 = 0;
      alarmTime3 = 0;
      counter3 = 0;
      return;
    }
  }
}

void HLPC()
{
  if ((hlpcCOMMON == HIGH) && (hlpcNC == LOW) && (counter4 == 0))
  {
    if (alarmSwitch4 == false)
    {
      alarmTime4 = currentMillis;
      alarmSwitch4 = true;
      Serial.println("alarmSwitch is true");
    }
    difference4 = currentMillis - alarmTime4;

    if ( difference4 >= debounceInterval)
    {
      Serial.println("Sending HLPC alarm message");
      sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, HLPCbody);
      //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, HLPCbody);
      //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, HLPCbody);
      //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, HLPCbody);
      Serial.println(F("message sent or simulated"));
      counter4 = 1;
      difference4 = 0;
      alarmSwitch4 = false;
      alarmTime4 = 0;
    }
    if (difference4 < debounceInterval)
    {
      Serial.println(difference4);
      return;
    }
  }
  else
  {
    if ((hlpcCOMMON == HIGH) && (hlpcNC == HIGH) && (counter4 == 1))
    {
      alarmSwitch4 = false;
      difference4 = 0;
      alarmTime4 = 0;
      counter4 = 0;
      return;
    }
  }
}

void sendSMS(char pt1[], char pt2[], char pt3[], char pt4[])
{
  char finalURL[250] = "";
  strcpy(finalURL, pt1);
  strcat(finalURL, pt2);
  strcat(finalURL, pt3);
  strcat(finalURL, pt4);
  delay(500);
  Serial.println(finalURL);
  delay(20);
  Serial1.print("AT+HTTPTERM\r");
  delay(1000);
  Serial1.print("AT+SAPBR=3,1,\"APN\",\"super\"\r");
  delay(300);
  Serial1.print("AT+SAPBR=1,1\r");
  delay(1000);
  Serial1.print("AT+HTTPINIT\r");
  delay(100);
  Serial1.print("AT+HTTPPARA=\"CID\",1\r");
  delay(100);
  Serial1.println(finalURL);
  delay(100);
  Serial1.print("AT+HTTPACTION=1\r");
  delay(5000);
}

// void getResponse()
// {
//   if (Serial1.available())
//   {
//     while (Serial1.available())
//     {
//       data = Serial1.read();
//       Serial.write(data);
//     }
//     data = 0;
//   }
//   delay(500);
// }

void timedmsg()
{
  if (msgswitch == false)
  {
    msgtimer1 = currentMillis;
    msgswitch = true;
  }
  difference5 = currentMillis - msgtimer1;

  if (difference5 >= dailytimer)
  {
    sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, REPbody);
    difference5 = 0;
    msgswitch = false;
    msgtimer1 = 0;
  }
}


void SMSRequest()
{
  if (Serial1.available() > 0) {
    incomingChar = Serial1.read();
    Serial.print(incomingChar);
    if (incomingChar == 'C') {
      delay(100);
      Serial.print(incomingChar);
      incomingChar = Serial1.read();
      if (incomingChar == 'H') {
        delay(100);
        Serial.print(incomingChar);
        incomingChar = Serial1.read();
        if (incomingChar == 'E') {
          delay(100);
          Serial.print(incomingChar);
          incomingChar = Serial1.read();
          if (incomingChar == 'C') {
            delay(100);
            Serial.print(incomingChar);
            incomingChar = Serial1.read();
            if (incomingChar == 'K') {
              delay(100);
              Serial.print(incomingChar);
              incomingChar = "";
              Serial.println(F("GOOD CHECK. SMS SYSTEMS ONLINE"));
              Serial.println(F("SENDING CHECK VERIFICATION MESSAGE")) ;
              sendSMS(urlHeaderArray, contactToArray1, contactFromArray2, CHECKbody);
              Serial.println("verification message sent");
              Serial1.print("AT+CMGD=0,4\r");
              delay(100);
              return;
            }
          }
        }
      }
    }
  }
  incomingChar = "";
  return;
}

void loadContacts()
{
  if (!SD.begin(10)) 
  {
    Serial.println(F("initialization failed!"));
    sendSMS(urlHeaderArray, "To=%2b16158122833&", "From=%2b19049808059&", "Body=SD%20Module%20Failed%20CaseCartBoiler1\"\r");
    while (1);
  }
  Serial.println(F("initialization done."));

  //------------------load "from" number.  This is the number alert messages will apear to be from-------------//

  myFile = SD.open("from1.txt");
  if (myFile) 
  {
    Serial.println("phone number command 1");
    // read from the file until there's nothing else in it:
    while (myFile.available()) 
    {
      char c = myFile.read();  //gets one byte from serial buffer
      conFrom1 += c;
    }
    myFile.close();
  } 
  else 
  {
    Serial.println("error opening from1.txt");
  }
  //convert the String into a character array
  conFrom1.toCharArray(contactFromArray1, 25);
  Serial.print(F("The first phone number FROM String is "));
  Serial.println(conFrom1);
  Serial.print(F("The first phone number FROM char array is "));
  Serial.println(contactFromArray1);

  //------------------load first contact number-------------//

  myFile = SD.open("to1.txt");
  if (myFile) 
  {
    Serial.println("phone number 1 command");
    while (myFile.available()) 
    {
      char c = myFile.read();
      conTo1 += c;
    }
    myFile.close();
  } 
  else 
  {
    Serial.println("error opening to1.txt");
  }
  conTo1.toCharArray(contactToArray1, 25);
  Serial.print(F("The first phone number TO String is "));
  Serial.println(conTo1);
  Serial.print(F("The first phone number TO char array is "));
  Serial.println(contactToArray1);

  //------------------load second contact number-------------//

  myFile = SD.open("to2.txt");
  if (myFile) 
  {
    Serial.println(F("phone number 2 command"));
    while (myFile.available()) 
    {
      char c = myFile.read();
      conTo2 += c;
    }
    myFile.close();
  } 
  else 
  {
    Serial.println(F("error opening to2.txt"));
  }
  conTo2.toCharArray(contactToArray2, 25);
  Serial.print(F("The second phone number TO String is "));
  Serial.println(conTo2);
  Serial.print(F("The second phone number TO char array is "));
  Serial.println(contactToArray2);

  //------------------load third contact number-------------//

  myFile = SD.open("to3.txt");
  if (myFile)
  {
    Serial.println("phone number 3 command");
    while (myFile.available()) 
    {
      char c = myFile.read();
      conTo3 += c;
    }
    myFile.close();
  } 
  else 
  {
    Serial.println("error opening to3.txt");
  }
  conTo3.toCharArray(contactToArray3, 25);
  Serial.print(F("The third phone number TO String is "));
  Serial.println(conTo3);
  Serial.print(F("The third phone number TO char array is "));
  Serial.println(contactToArray3);

  //------------------load fourth contact number-------------//

  myFile = SD.open("to4.txt");
  if (myFile) 
  {
    Serial.println("phone number 4 command");
    while (myFile.available()) 
    {
      char c = myFile.read();
      conTo4 += c;
    }
    myFile.close();
  } 
  else 
  {
    Serial.println("error opening to4.txt");
  }
  conTo4.toCharArray(contactToArray4, 25);
  Serial.print(F("The fourth phone number TO String is "));
  Serial.println(conTo4);
  Serial.print(F("The fourth phone number TO char array is "));
  Serial.println(contactToArray4);

  //------------------load URL header-------------//

  myFile = SD.open("URL.txt");
  if (myFile) 
  {
    Serial.println(F("loading URL header"));
    while (myFile.available()) 
    {
      char c = myFile.read();
      URLheader += c;
    }
    myFile.close();
  } 
  else 
  {
    Serial.println("error opening URL.txt");
  }
  URLheader.toCharArray(urlHeaderArray, 100);
  Serial.print(F("The URL header is "));
  Serial.println(URLheader);
  Serial.print(F("The URL header array is  "));
  Serial.println(urlHeaderArray);
}

void preTransmission() // user designated action required by the MODBUS library
{                      // writing these terminals to HIGH / LOW tells the RS-485 board to send or recieve
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}
void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void readModbus()
{
  Serial.println("In the readModbus() function now");
  uint16_t result;

  result = node.readHoldingRegisters (0x0000, 1);
  if (result == node.ku8MBSuccess)
  {
    int alarmRegister = node.getResponseBuffer(result);
    Serial.print("Register response:  ");
    Serial.println(alarmRegister);

    switch (alarmRegister)
    {
      case  1: sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Fault%20Code1%20No%20Purge%20Card\"\r");
               //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, "Body=Fault%20Code1%20No%20Purge%20Card\"\r");
               //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, "Body=Fault%20Code1%20No%20Purge%20Card\"\r");
               //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, "Body=Fault%20Code1%20No%20Purge%20Card\"\r");
               break;
      case 10: sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Fault%20Code10%20PreIgnition%20Interlock%20Standby\"\r");
               //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, "Body=Fault%20Code10%20PreIgnition%20Interlock%20Standby\"\r");
               //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, "Body=Fault%20Code10%20PreIgnition%20Interlock%20Standby\"\r");
               //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, "Body=Fault%20Code10%20PreIgnition%20Interlock%20Standby\"\r");
               break;
      case 14: sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Fault%20Code14%20High%20Fire%20Interlock%20Switch\"\r");
               //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, "Body=Fault%20Code14%20High%20Fire%20Interlock%20Switch\"\r");
               //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, "Body=Fault%20Code14%20High%20Fire%20Interlock%20Switch\"\r");
               //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, "Body=Fault%20Code14%20High%20Fire%20Interlock%20Switch\"\r");
               break;
      case 15: sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Fault%20Code15%20Unexpected%20Flame\"\r");
               //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, "Body=Fault%20Code15%20Unexpected%20Flame\"\r");
               //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, "Body=Fault%20Code15%20Unexpected%20Flame\"\r");
               //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, "Body=Fault%20Code15%20Unexpected%20Flame\"\r");
               break;
      case 17: sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Fault%20Code17%20Main%20Flame%20Failure%20RUN\"\r");
               //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, "Body=Fault%20Code17%20Main%20Flame%20Failure%20RUN\"\r");
               //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, "Body=Fault%20Code17%20Main%20Flame%20Failure%20RUN\"\r");
               //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, "Body=Fault%20Code17%20Main%20Flame%20Failure%20RUN\"\r");
              break;
      case 19: sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Fault%20Code19%20Main%20Flame%20Ignition%20Failure\"\r");
               //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, "Body=Fault%20Code19%20Main%20Flame%20Ignition%20Failure\"\r");
               //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, "Body=Fault%20Code19%20Main%20Flame%20Ignition%20Failure\"\r");
               //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, "Body=Fault%20Code19%20Main%20Flame%20Ignition%20Failure\"\r");
               break;
      case 20: sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Fault%20Code20%20Low%20Fire%20Interlock%20Switch\"\r");
               //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, "Body=Fault%20Code20%20Low%20Fire%20Interlock%20Switch\"\r");
               //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, "Body=Fault%20Code20%20Low%20Fire%20Interlock%20Switch\"\r");
               //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, "Body=Fault%20Code20%20Low%20Fire%20Interlock%20Switch\"\r");
              break;
      case 28: sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Fault%20Code28%20Pilot%20Flame%20Failure\"\r");
               //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, "Body=Fault%20Code28%20Pilot%20Flame%20Failure\"\r");
               //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, "Body=Fault%20Code28%20Pilot%20Flame%20Failure\"\r");
               //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, "Body=Fault%20Code28%20Pilot%20Flame%20Failure\"\r");
               break;
      case 29: sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Fault%20Code29%20Lockout%20Interlock\"\r");
               //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, "Body=Fault%20Code29%20Lockout%20Interlock\"\r");
               //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, "Body=Fault%20Code29%20Lockout%20Interlock\"\r");
               //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, "Body=Fault%20Code29%20Lockout%20Interlock\"\r");
               break;
      case 33: sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Fault%20Code33%20PreIgnition%20Interlock\"\r");
               //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, "Body=Fault%20Code33%20PreIgnition%20Interlock\"\r");
               //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, "Body=Fault%20Code33%20PreIgnition%20Interlock\"\r");
               //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, "Body=Fault%20Code33%20PreIgnition%20Interlock\"\r");
              break;
      case 47: sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Fault%20Code47%20Jumpers%20Changed\"\r");
               //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, "Body=Fault%20Code47%20Jumpers%20Changed\"\r");
               //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, "Body=Fault%20Code47%20Jumpers%20Changed\"\r");
               //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, "Body=Fault%20Code47%20Jumpers%20Changed\"\r");
               break;
      default: sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Fault%20Check%20Fault%20Code\"\r");
               //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, "Body=Fault%20Check%20Fault%20Code\"\r");
               //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, "Body=Fault%20Check%20Fault%20Code\"\r");
               //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, "Body=Fault%20Check%20Fault%20Code\"\r");
              break;
    }
  }
  else
  {
    sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Modbus%20Com%20Fail\"\r");
    //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, "Body=Modbus%20Com%20Fail\"\r");
    //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, "Body=Modbus%20Com%20Fail\"\r");
    //sendSMS(urlHeaderArray, contactToArray4, contactFromArray1, "Body=Modbus%20Com%20Fail\"\r");
  }
}

void SIMboot()
{
//This function only boots the SIM module if it needs to be booted
//This prevents nuisance power-downs upon startup
  unsigned char sim_buffer {};
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      Serial1.print("AT\r"); //blank AT command elicits a response of OK
      delay(50);
    }
    if (Serial1.available())
    {
      while (Serial1.available())
      {
        sim_buffer = Serial1.read();
        Serial.write(sim_buffer);
      }
      printf("SIM module appears to be on.  No need to boot\n");
      return;
    }
    else 
    {
      printf("SIM module appears to be off.  Attempting boot...\n");
      digitalWrite(SIMpin, HIGH);
      delay(3000);
      digitalWrite(SIMpin, LOW);
      printf("boot attempted.  wait 10 seconds\n");
      delay(10000);
    }
  }
}
