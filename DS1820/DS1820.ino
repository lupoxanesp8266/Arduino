#include <OneWire.h> 
#include <DallasTemperature.h>
 
#define ONE_WIRE_BUS 2 
#define MAXTEMP 28.00
#define MINTEMP 20.00

OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
/********************************************************************/ 
void setup(void) 
{ 
 // start serial port 
 Serial.begin(115200); 
 sensors.begin();
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
} 
void loop(void) 
{
 sensors.requestTemperatures();
 float temp = sensors.getTempCByIndex(0);
 Serial.println(sensors.getTempCByIndex(0));
 int brillo = map(temp,MINTEMP,MAXTEMP,255,0);
 if(temp >= MAXTEMP){
  brillo = 0;
  digitalWrite(4,HIGH);
 }
 if(temp > MINTEMP && temp < MAXTEMP){
  digitalWrite(4,LOW);
  digitalWrite(5,LOW);
 }
 if(temp <= MINTEMP){
  brillo = 255;
  digitalWrite(5,HIGH);
 }
 analogWrite(3,brillo);
   delay(100); 
} 
