#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <RTClib.h>//Para RTC
#include "IRremote.h"

#define ONE_WIRE_BUS 25
#define TIME 10000//Tiempo

int i = 0;
const int rs = 27, en = 26, d4 = 31, d5 = 30, d6 = 29, d7 = 28;
char buffer[128];
String strValues;
unsigned long presentime;//Para contar el tiempo
unsigned long previoustime = 0;//Para contar el  tiempo
int receiver = 22; // Signal Pin of IR receiver to Arduino Digital Pin 11

IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'
RTC_DS3231 RTC;//Variable de RTC tipo 3231
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  sensors.begin();
  RTC.begin();
  if (!RTC.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }
  //RTC.adjust(DateTime(__DATE__, __TIME__));//Para ajustar fecha y hora
  pinMode(23, OUTPUT);
  digitalWrite(23,HIGH);
  pinMode(24, INPUT);
  pinMode(6, OUTPUT);
  irrecv.enableIRIn(); // Start the receiver
}
void loop() {
  DateTime now = RTC.now();//Variable para RTC
  sprintf(buffer, "%02d/%02d/%04d %02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute());
  strValues = buffer;
  lcd.setCursor(0, 0);
  lcd.print(strValues);
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(i);
  int pir = digitalRead(24);
  presentime = millis();
  if (presentime - previoustime >= TIME) {
    previoustime = presentime;
    switch (i) {
      case 0:
        Serial.print("Temp1: ");
        Serial.print(temp);
        Serial.print(" C");
        lcd.setCursor(0, 1);
        lcd.print("Temp1: ");
        lcd.print(temp);
        lcd.print(" C");
        if (pir) {
          digitalWrite(23, LOW);
        } else {
          digitalWrite(23, HIGH);
        }
        i++;
        break;
      case 1:
        Serial.print(" --- ");
        Serial.print("Temp2: ");
        Serial.print(temp);
        Serial.println(" C");
        lcd.setCursor(0, 1);
        lcd.print("Temp2: ");
        lcd.print(temp);
        lcd.print(" C");
        i = 0;
        break;
    }
  }
  if (irrecv.decode(&results)) {
    translateIR();
    irrecv.resume(); // receive the next value
  }
}
void setAnalog(int value){
  analogWrite(6,value);
}

void translateIR() {

  switch (results.value) {
    case 0xFFA25D:
      setAnalog(0);
      break;
    case 0xFFE21D:
      Serial.println("FUNC/STOP");
      break;
    case 0xFF629D:
      Serial.println("VOL+");
      break;
    case 0xFF22DD:
      Serial.println("FAST BACK");
      break;
    case 0xFF02FD:
      Serial.println("PAUSE");
      break;
    case 0xFFC23D:
      Serial.println("FAST FORWARD");
      break;
    case 0xFFE01F:
      Serial.println("DOWN");
      break;
    case 0xFFA857:
      Serial.println("VOL-");
      break;
    case 0xFF906F:
      Serial.println("UP");
      break;
    case 0xFF9867:
      Serial.println("EQ");
      break;
    case 0xFFB04F:
      Serial.println("ST/REPT");
      break;
    case 0xFF6897:
      analogWrite(6,25);
      break;
    case 0xFF30CF:
      analogWrite(6,50);
      break;
    case 0xFF18E7:
      analogWrite(6,75);
      break;
    case 0xFF7A85:
      analogWrite(6,100);
      break;
    case 0xFF10EF:
      analogWrite(6,125);
      break;
    case 0xFF38C7:
      analogWrite(6,150);
      break;
    case 0xFF5AA5:
      analogWrite(6,175);
      break;
    case 0xFF42BD:
      analogWrite(6,200);
      break;
    case 0xFF4AB5:
      analogWrite(6,225);
      break;
    case 0xFF52AD:
      analogWrite(6,255);
      break;
    case 0xFFFFFFFF:
      Serial.println(" REPEAT");
      break;
    default:
      Serial.println(" other button   ");

  }
  delay(500);
}
