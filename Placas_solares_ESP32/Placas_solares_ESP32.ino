/*** BIBLIOTECAS ***/
#include <WiFi.h>//Para conectarse a una red WiFi
#include <ThingerESP32.h>//Para conectarse a Thinger.io
#include <SD.h>//Para la SD
#include <RTClib.h>//Para RTC
#include <Wire.h>//Para protocolo I2C
#include <LiquidCrystal_I2C.h>//Para LCD
#include <driver/adc.h>//Para controlar las entradas analógicas
#include "FS.h"
#include "SD.h"
#include "SPI.h"
/*** CONSTANTES ***/
#define USERNAME "lupoxan"//Nombre de usuario de Thinger.io
#define DEVICE_ID "Esp_32"//Nombre del dispositivo Thinger
#define DEVICE_CREDENTIAL "7#G9BVTY&tqZ"//Credenciales del dispositivo
#define SSID "MOVISTAR_9DBC"//SSID Wifi
#define SSID_PASSWORD "RY64GqbfCCqeKmGUpJxN"//Password Wifi
#define MAX 500//Máximo para media de valores
#define SECONDS 1//Segundos que quieres tardar en mostrar valores
#define TIME 1000*SECONDS//Tiempo de espera para mostrar valores
#define MAXCORRIENTE 20.00//Corriente máxima antes de dar alarma
#define MAXTENSION 15.00//Tension máxima antes de dar alarma
#define SENSORES 6//Número de pines analógicos
#define Chip_Select_SD 5//Pin al que se conectará la SD
#define DECIMAL 100.00//Para quitar los decimales
#define NOMBRE_ARCHIVO "valores.txt"//Nombre del archivo en la SD
#define MAXVAL 60//Maximo de valores que guarda
#define MAXSENS 6//Maximo de sensores que hay
/*** VARIABLES NECESARIAS ***/
ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);//Variable para Thinger.io
RTC_DS3231 RTC;//Variable de RTC
//RTC_DS1307 RTC;//Variable de RTC
LiquidCrystal_I2C lcd(0x27, 16, 2);//Variable para lcd
File dataFile;//Variable para SD
//Current: A0 --> Vp, A3 --> Vn, A6 --> D34
//Voltage: A7 --> D35, A4 --> D32, A5 --> D33
int sensores[MAXSENS] = {A0, A3, A6, A7, A4, A5}; //Vector con las entradas
typedef struct {
  char tension = 'v';
  char corriente = 'A';
} modo;
typedef struct {
  float valores[MAXVAL];
  String fechora[MAXVAL];
  modo tp;
} mediciones;
/*** VARIABLES A CONDICIÓN ***/
mediciones vec[MAXSENS];
mediciones value;
String fecHora;
char algo;
unsigned long presentime;//Para contar el tiempo
unsigned long previoustime = 0;//Para contar el  tiempo
int indice;//Para saber qué sensor es el que está leyendo
char buffer[128];//Añadir a un String
String strValues1;
String strValues;//String que guarda valores
int enteroVal;//Para definir entero 'valores'
int decimalVal;//Para definir decimal 'valores'
float valor;//Para la tension y para la corriente
boolean tipo;//Para saber cuando mide tension (false) y cuando corriente (true)
boolean condicion;//La condicion de si salta la alarma (true)o no (false)
int j = 0, i = 0;//Indice para mostrar por pantalla, grabar SD y mostrar Serial
/*** FUNCIONES ***/
float medicion(boolean, int);//Para hacer las mediciones
void pantalla(boolean, int, float);//Para LCD
void mostrar_por_serial(boolean, int, float);//Para PC
void leer_tarjeta();//Para leer SD
void escribir_tarjeta(boolean, int, float);//Para escribir SD
void elementos_clave();//Para definir "resources"
/*** VOID SETUP() ***/
void setup() {
  thing.add_wifi(SSID, SSID_PASSWORD);//Conectar a la red WiFi
  lcd.begin();//Encender la pantalla lcd
  output_input_pins();//Iniciar los pines a OUTPUT
  Wire.begin();//Para el protocolo I2C
  Serial.begin(115200);//Comenzar comunicación Serial
  RTC.begin();//Arrancar la RTC
  elementos_clave();//Add Resources
  pinMode(Chip_Select_SD , OUTPUT);
  leer_tarjeta();//Función que lee la tarjeta SD
  //RTC.adjust(DateTime(__DATE__, __TIME__));//Para ajustar fecha y hora
  escribir_tarjeta(SD, "/valores.txt", "ISFV IES CIUDAD JARDIN\n");
}
/*** VOID LOOP() ***/
void loop() {
  thing.handle();
  presentime = millis();
  if (presentime - previoustime >= TIME) {
    previoustime = presentime;
    if (j > SENSORES - 1) {
      j = 0;
    }
    if (j >= 0 && j <= 2) {
      tipo = true;
    }
    if (j >= 3 && j <= SENSORES - 1) {
      tipo = false;
    }
    valor = medicion(tipo, j);
    mostrar_por_serial(tipo, j, valor);
    pantalla(tipo, j, valor);
    appendFile(SD, "/valores.txt", valor, j);
    j++;
  }
}

float medicion(boolean tipo, int indice) {
  float value;
  float media = 0.00;
  if (tipo == true) { //Corriente
    for (int i = 0; i <= MAX - 1; i++) {
      value = analogRead(sensores[indice]);
      value = ((3.3 * value / 4095) - 0.8) / 0.100;//0.185 for 5 A
      media = media + value;
    }
  }
  else { //Tensión
    for (int i = 0; i <= MAX - 1; i++) {
      value = analogRead(sensores[indice]);
      value = 3.3 * value * 7.27 / 4095;
      media = media + value;
    }
  }
  media = media / MAX;
  return media;

}

void pantalla(boolean tipo, int indice, float valor) {
  DateTime now = RTC.now();
  if (tipo == true) { //Corriente
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("I");
    lcd.print(indice + 1);
    lcd.print(" = ");
    lcd.setCursor(6, 0);
    lcd.print(valor);
    lcd.setCursor(12, 0);
    lcd.print(" A");
  }
  else { //tensión
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("V");
    lcd.print(indice - 2);
    lcd.print(" = ");
    lcd.setCursor(6, 0);
    lcd.print(valor);
    lcd.setCursor(12, 0);
    lcd.print("v");
  }
  lcd.setCursor(0, 1);
  sprintf(buffer, "%02d/%02d/%04d %02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute());
  strValues1 = buffer;
  lcd.print(strValues1);
}

void mostrar_por_serial(boolean tipo, int indice, float valor) {
  DateTime now = RTC.now();
  if (indice == 0) {
    sprintf(buffer, "%04d/%02d/%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    strValues = buffer;
  }
  if (tipo == true) { //Corriente
    enteroVal = valor;
    decimalVal = (valor - (float)enteroVal) * DECIMAL;
    sprintf(buffer, " -- I%d = %d.%d A", indice + 1, enteroVal, abs(decimalVal));
    strValues += buffer;
  }
  else {
    enteroVal = valor;
    decimalVal = (valor - (float)enteroVal) * DECIMAL;
    sprintf(buffer, " -- V%d = %d.%d v", indice - 2, enteroVal, abs(decimalVal));
    strValues += buffer;
    if (indice == SENSORES - 1) {
      Serial.print(strValues);
      strValues = "\n";
      Serial.print(strValues);
    }
  }
}

void leer_tarjeta() {
  Serial.print(F("Iniciando SD ... "));
  if (!SD.begin(Chip_Select_SD)) {
    Serial.println(F("Error al iniciar"));
    return;
  }
  Serial.println(F("Correcto"));
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
}

void escribir_tarjeta(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, float valor, int indice) {
  /*DateTime now = RTC.now();

  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (indice == 0) {
    sprintf(buffer, "%02d/%02d/%04d %02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute());
    strValues1 = buffer;
    file.print(strValues1);
  }
  file.print(",");
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  if(indice == MAXSENS-1){
    file.println();
  }
  file.close();*/
  File logFile;
  DateTime now = RTC.now();
  logFile = SD.open(path, FILE_APPEND);
  if (logFile) {
    if (indice == 0) {
      sprintf(buffer, "%04d/%02d/%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
      strValues1 = buffer;
    }
    if (tipo == true) { //Corriente
      enteroVal = valor;
      decimalVal = (valor - (float)enteroVal) * DECIMAL;
      sprintf(buffer, ",%d.%d", indice + 1, enteroVal, abs(decimalVal));
      strValues1 += buffer;
      logFile.print(",");
      logFile.print(valor);
    }
    else {
      enteroVal = valor;
      decimalVal = (valor - (float)enteroVal) * DECIMAL;
      sprintf(buffer, ",%d.%d", indice + 1, enteroVal, abs(decimalVal));
      strValues1 += buffer;
      if (indice == SENSORES - 1) {
        logFile.print(strValues1);
        strValues1 = "\n";
        logFile.print(strValues1);
      }
    }
    logFile.close();
  }
  else {
    Serial.println("Error al abrir el archivo");
  }
}
void elementos_clave() {
  thing["LED_On_Board"] << digitalPin(2);
  thing["Corrientes_tensiones"] >> [](pson & out) {
    out["I1"] = medicion(true, 0);
    out["I2"] = medicion(true, 1);
    out["I3"] = medicion(true, 2);
    out["V1"] = medicion(false, 3);
    out["V2"] = medicion(false, 4);
    out["V3"] = medicion(false, 5);
    out["P1"] = medicion(true, 0) * medicion(false, 3);
    out["P2"] = medicion(true, 1) * medicion(false, 4);
    out["P3"] = medicion(true, 2) * medicion(false, 5);
  };
}

void output_input_pins() {
  /*** OUTPUT ***/
  //int output_pins[]={2,13,12,14,27,26,25,15,4};
  pinMode(2, OUTPUT);//Led On_Board
  digitalWrite(2, LOW);
  pinMode(13, OUTPUT);//C1 Iluminacion
  digitalWrite(13, LOW);
  pinMode(12, OUTPUT);//C2 Uso general
  digitalWrite(12, LOW);
  pinMode(14, OUTPUT);//C3 Cocina y horno
  digitalWrite(14, LOW);
  pinMode(27, OUTPUT);//C4 Lavadora, lavavajillas y termo
  digitalWrite(27, LOW);
  pinMode(26, OUTPUT);//C5 Uso general auxiliar
  digitalWrite(26, LOW);
  /*pinMode(, OUTPUT);//Rele C6 Iluminacion (+30 puntos)
    digitalWrite(, LOW);
    pinMode(, OUTPUT);//Rele C7 Uso general (+20 puntos)
    digitalWrite(, LOW);*/
  pinMode(25, OUTPUT);//C8 Calefaccion
  digitalWrite(25, LOW);
  pinMode(15, OUTPUT);//C9 Aire acondicionado
  digitalWrite(15, LOW);
  pinMode(4, OUTPUT);//C10 Secadora
  digitalWrite(4, LOW);
  /*** INPUT ***/
  //int sensores[] = {A0, A3, A6, A7, A4, A5};
}

