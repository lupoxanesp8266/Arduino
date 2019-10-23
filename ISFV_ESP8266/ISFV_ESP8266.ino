#include <LiquidCrystal_I2C.h>

//#include <LiquidCrystal_I2C.h>//Para LCD
#include <Wire.h>//Para protocolo I2C
#include <RTClib.h>//Para RTC
#include <ESP8266WiFi.h>//Biblioteca de conexión WiFi del módulo ESP8266
#include <ThingerESP8266.h>//Biblioteca de la plataforma thinger.io
#include <SD.h>//Para la SD

#define SSID1 "MIWIFI_jZTH"//SSID Wifi
#define PASS1 "PIso3Wazowski2018"//Password Wifi
#define usuario "lupoxan"//Nombre de usuario de thinger
#define device_Id "ESP_8266"//Nombre del dispositivo al que nos conectaremos
#define device_credentials "#gHQ3ARec6Rh"//Credenciales generadas por Thinger
#define Chip_Select_SD D8//Pin al que se conectará la SD
#define NOMBRE_ARCHIVO "valores.txt"//Nombre del archivo en la SD

ThingerESP8266 thing(usuario, device_Id, device_credentials);//Variable para acceder a Thinger
LiquidCrystal_I2C lcd(0x27, 16, 2);//Variable para lcd0x3F
RTC_DS3231 RTC;//Variable de RTC tipo 3231
File dataFile;//Variable para SD

void setup() {
  Serial.begin(115200);//Iniciar comunicacion serial
  lcd.begin();//Iniciar pantalla lcd
  lcd.backlight();
  RTC.begin();//Iniciar RTC
  //RTC.adjust(DateTime(F(__DATE__),F( __TIME__)));//Para configrar fecha y hora
  Serial.print(F("Iniciando SD ... ")); 
  if (!SD.begin(Chip_Select_SD)){ 
    Serial.println(F("Error al iniciar")); 
    return; 
    } 
  Serial.println(F("Correcto"));
  if (dataFile = SD.open("valores.txt")){ 
    while (dataFile.available()){
      Serial.write(dataFile.read());
    } 
    dataFile.close();
    } 
  else{ 
    Serial.println(F("Error al abrir el archivo"));
  }
  thing.add_wifi(SSID1, PASS1);//Conectar a la red WiFi
  lcd.setCursor(0,0);
  lcd.print("Hello world");
  thing["Value"] >> outputValue(analogRead(A0));
}

void loop() {
  File logFile;
  thing.handle();
  DateTime now = RTC.now();//Variable para la RTC
  lcd.setCursor(0,1);
  lcd.print(now.day(), DEC); // Dia
  lcd.print('/');
  lcd.print(now.month(), DEC); // Mes
  lcd.print('/');
  lcd.print(now.year(), DEC); // Año
  lcd.print(' ');
  lcd.print(now.hour(), DEC); // Horas
  lcd.print(':');
  lcd.print(now.minute(), DEC); // Minutos
  delay(1000);
  lcd.setCursor(0,0);
  lcd.print("Segundos: ");
  lcd.print(now.second());
  logFile = SD.open("valores.txt", FILE_WRITE);
  if (logFile){
    logFile.print(now.year(), DEC); // Año
    logFile.print('/');
    logFile.print(now.month(), DEC); // Mes
    logFile.print('/');
    logFile.print(now.day(), DEC); // Dia
    logFile.print(",");
    logFile.print(now.hour(), DEC); // Horas
    logFile.print(':');
    logFile.print(now.minute(), DEC); // Minutos
    logFile.print(':');
    logFile.print(now.second(), DEC); // Segundos
    logFile.println();
    logFile.close();
    }
  else{
    Serial.println("Error al abrir el archivo");
    }
}
