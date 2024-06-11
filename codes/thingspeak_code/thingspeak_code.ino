String myAPIkey = "9M6HXVMIKI7FKCII";  
 
long writingTimer = 17; 
long startTime = 0;
long waitTime = 0;
 
unsigned char check_connection=0;
unsigned char times_check=0;
boolean error;

#include "DHT.h"
#define DHTPIN 11     
#define DHTTYPE DHT11   
DHT dht(DHTPIN, DHTTYPE);
float t;

int measurePin = 0; 
int ledPower = 7;   
int samplingTime = 280; 
int deltaTime = 40; 
int sleepTime = 9680;
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

//ccs_811 library
#include "Adafruit_CCS811.h"
#include <Adafruit_I2CDevice.h>
Adafruit_CCS811 ccs;
int co2;

//*************************************************************************************//

void setup()
{ 
  Serial.begin(115200);  
  startTime = millis(); 
  Serial.println("AT+RST");
  delay(2000);
   while(check_connection==0)
  {Serial.print("AT+CWJAP=\"menna\",\"menna156\"\r\n");
   Serial.setTimeout(5000);
 if(Serial.find("WIFI CONNECTED\r\n")==1){break;}
 times_check++;
 if(times_check>3){times_check=0;}
  }
  
  ccs.begin();
  pinMode(ledPower,OUTPUT);
  dht.begin();
  
}

void loop()
{
  waitTime = millis()-startTime;   
  if (waitTime > (writingTimer*1000)) 
  {
    readSensors();
    writeThingSpeak();
    startTime = millis();   
  }
}


void readSensors(void)
{
   t = dht.readTemperature();
   if(ccs.available()){ccs.readData();co2=ccs.geteCO2();}

  digitalWrite(ledPower,LOW); // power on the LED
  delayMicroseconds(samplingTime);
  voMeasured = analogRead(measurePin); // read the dust value
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH); // turn the LED off
  delayMicroseconds(sleepTime);
  calcVoltage = voMeasured * (5.0 / 1024.0);
  dustDensity = 170 * calcVoltage - 0.1;
}


/////////////////////////////////////////////////////////////////////////

void writeThingSpeak(void)
{
  startThingSpeakCmd();
  String getStr = "GET /update?api_key=";
  getStr += myAPIkey;
  getStr +="&field1=";
  getStr += String(t);
  getStr +="&field2=";
  getStr += String(co2);
  getStr +="&field3=";
  getStr += String(dustDensity);
  getStr += "\r\n\r\n";
  GetThingspeakcmd(getStr); 
}

//////////////////////////////////////////////////////////////////////////

void startThingSpeakCmd(void)
{
  Serial.flush();
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com IP address
  cmd += "\",80";
  Serial.println(cmd);
  if(Serial.find("Error")) {return;}
}

////////////////////////////////////////////////////////////////////////////////

String GetThingspeakcmd(String getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  Serial.println(cmd);
  if(Serial.find(">"))
   {Serial.print(getStr);
    delay(500);
    String messageBody = "";
    while (Serial.available()) 
    {String line = Serial.readStringUntil('\n');
     if (line.length() == 1) {messageBody = Serial.readStringUntil('\n');}
    }
    return messageBody;
  }
  else{Serial.println("AT+CIPCLOSE");} 
}
