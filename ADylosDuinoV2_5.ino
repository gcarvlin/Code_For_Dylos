/*

Software for uploading Dylos PM counts to kairosDB through an Arduino Yun.
Version 2.5
Date 12-04-14
Seto Lab
UW DEOHS

*/

#include <Wire.h>
#include <Bridge.h>
#include <Process.h>
#include <SoftwareSerial.h>
#include <FileIO.h>
#define rxPin 10  //pin 10 on the arduino is connected to the TX pin on the converter
#define txPin 11  //pin 11 on the arduino is connected to the RX pin on the converter
SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

// HIH6130 variables
unsigned int H_dat, T_dat;
byte _status;
float RH, T_C;

//String variables
String inputString,string1,string2,string3,string4,timeString,dylosTime;
String seconds, minute, hour, day, month, year;
String nameString = "Dylos8";  //Name of unit

//Integer variables
int intDaysinMonth;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Serial open..");
  FileSystem.begin();
  Serial.println("File system open..");
  Bridge.begin();
  Serial.println("Bridge open..");
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  Serial.println("Pinmode set..");
  mySerial.begin(9600);
  Serial.println("Software serial setup..");
  Serial.println("DylosDuino v2.5 booted up!");
}

void loop() {

   //Read in data, if at end of string analyze input
   char bchar = mySerial.read();
   if (bchar != -1) {
   inputString += bchar;
   if (bchar == 10) {
     
     //Analyze input string recieved from Dylos
     dataAnalysis(inputString);
     
     //Output to serial console for debugging
     Serial.print(inputString);
     Serial.print("Bin1 = ");
     Serial.print(string1);
     Serial.print(", Bin2 = ");
     Serial.print(string2);
     Serial.print(", Bin3 = ");
     Serial.print(string3);
     Serial.print(", Bin4 = ");
     Serial.println(string4);
     
     //Get temp and RH data
     getRHTHIH6130();
     RH = (float) H_dat * 6.104e-3;
     T_C = (float) T_dat * 1.007e-2 - 40.0;
     Serial.print(_status);  
     //  case 0: "Normal"   case 1: "Stale Data"   case 2: "In command mode"  default: "Diagnostic" 
     Serial.print(",");
     Serial.print(RH, 1);
     Serial.print(",");
     Serial.println(T_C, 2);
     
     //Send data
     sendData();
     
     //Output to SD card
     File dataFile = FileSystem.open("/mnt/sda1/datalog.txt", FILE_APPEND);
     if (dataFile) {
     dataFile.print(timeString);
     dataFile.print("000, ");
     dataFile.print(dylosTime);
     dataFile.print("000, ");
     dataFile.print(string1);
     dataFile.print(", ");
     dataFile.print(string2);
     dataFile.print(", ");
     dataFile.print(string3);
     dataFile.print(", ");
     dataFile.println(string4);
     dataFile.close();
     Serial.println("Data saved locally!");
     }  
     //If the file isn't open, pop up an error:
     else {
     Serial.println("Error opening file");
     }
     Serial.println("");
     
     //Reset variables
     inputString = "";
     string1 ="";
     string2 ="";
     string3 ="";
     string4 ="";
     timeString = "";
     }
   }
   delay(10);
}

//Send data to kairos
void sendData() {
  Serial.print("Sending data...");
  
  //Get current time
  Process t;
  t.begin("date");
  t.addParameter("+%s");
  t.run();

  while (t.available() > 0) {
    char c = t.read();
    if ((c>47) && (c<58)) {
      timeString += c;
      }  
    }
  Serial.flush();

  //Call a python script which uploads the data to kairos
  Process p;
  p.begin("python");
  p.addParameter("/root/upload.py");
  p.addParameter(nameString);
  p.addParameter(timeString);
  p.addParameter(string1);
  p.addParameter(string2);
  p.addParameter(string3);
  p.addParameter(string4);
  p.addParameter(String(int(T_C)));
  p.addParameter(String(-((int(T_C)*1000)-int(T_C*1000))));
  p.addParameter(String(int(RH)));
  p.addParameter(String(-((int(RH)*1000)-int(RH*1000))));
  //p.addParameter("&2>1"); // Pipe error output to stdout, uncomment for debugging
  p.run();

  while(p.available()>0) {
    Console.print(p.read());
  }

  Serial.println("  Data Sent!");
}

//Parse the data from the Dylos
void dataAnalysis(String aString) {
  int start = aString.indexOf(',');
  int second = aString.indexOf(',', start+1);
  int third = aString.indexOf(',', second+1);
  int last = aString.indexOf(',', third+1);  
  int final = aString.indexOf('-');
  int slash1 = aString.indexOf('/');
  int slash2 = aString.indexOf('/', slash1+1);
  int space = aString.indexOf(' ');
  int colon1 = aString.indexOf(':');
  int colon2 = aString.indexOf(':', colon1+1);

  if (final = -1) {final = inputString.length()-2;}
  
  month = ""; day = ""; year = ""; hour = ""; minute = ""; seconds = ""; dylosTime = "";
  
  month += (inputString.charAt(0));
  month += (inputString.charAt(1));
  
  day += (inputString.charAt(3));
  day += (inputString.charAt(4));
  
  year += (inputString.charAt(6));
  year += (inputString.charAt(7));
  
  hour += (inputString.charAt(9));
  hour += (inputString.charAt(10));
 
  minute += (inputString.charAt(12));
  minute += (inputString.charAt(13));
 
  seconds += (inputString.charAt(15));
  seconds += (inputString.charAt(16));
  
  //Convert to Unix Time
  
  //Find out how many days into the year it is
  switch (month.toInt()) {
    case 1:
    intDaysinMonth = 0;
    break; 
    case 2:
    intDaysinMonth = 31;           //30 = sept, april, june, nov   28 = Feb
    break;
    case 3:
    intDaysinMonth = 31+28; //Feb - Doesn't check if this is a leap year
    break;
    case 4:
    intDaysinMonth = 31+28+31;  
    break;
    case 5:
    intDaysinMonth = 31+28+31+30; //Apr
    break;
    case 6:
    intDaysinMonth = 31+28+31+30+31;  
    break;
    case 7:
    intDaysinMonth = 31+28+31+30+31+30; //Jun
    break;
    case 8:
    intDaysinMonth = 31+28+31+30+31+30+31; 
    break;
    case 9:
    intDaysinMonth = 31+28+31+30+31+30+31+31; 
    break;
    case 10:
    intDaysinMonth = 31+28+31+30+31+30+31+31+30; //Sep
    break;
    case 11:
    intDaysinMonth = 31+28+31+30+31+30+31+31+30+31; 
    break;
    case 12:
    intDaysinMonth = 31+28+31+30+31+30+31+31+30+31+30; //Nov
    break;
    default:
    Serial.println("Error in month calculation!");    
  }
  
  Serial.print("Unix timestamp = ");
  dylosTime += (((((365 * (year.toInt()+30)) + 10 +intDaysinMonth+day.toInt())*24) + hour.toInt() + 8)*60*60) +(minute.toInt()*60)+seconds.toInt();
  Serial.println(dylosTime); //this does not count leap seconds
  
  for(int i=0; i<(second-start-1); i++) {
    string1 += (inputString.charAt(start+i+1)-48);
  }
  
  for(int i=0; i<(third-second-1); i++) {
    string2 += (inputString.charAt(second+i+1)-48);
  }
  
  for(int i=0; i<(last-third-1); i++) {
    string3 += (inputString.charAt(third+i+1)-48);
  }
  
  for(int i=0; i<(final-last-1); i++) {
    string4 += (inputString.charAt(last+i+1)-48);
  }
}


// Get the RH and temp from the HIH-6130
void getRHTHIH6130() {
  byte address, Hum_H, Hum_L, Temp_H, Temp_L;
  address = 0x27;;
  Wire.beginTransmission(address); 
  Wire.endTransmission();
  delay(100);
  
  Wire.requestFrom((int)address, (int) 4);
  Hum_H = Wire.read();
  Hum_L = Wire.read();
  Temp_H = Wire.read();
  Temp_L = Wire.read();
  Wire.endTransmission();
  
  _status = (Hum_H >> 6) & 0x03;
  Hum_H = Hum_H & 0x3f;
  H_dat = (((unsigned int)Hum_H) << 8) | Hum_L;
  T_dat = (((unsigned int)Temp_H) << 8) | Temp_L;
  T_dat = T_dat / 4;
}
