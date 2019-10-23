/************* MONITORIZACION DE UNA INSTALACION SOLAR FOTOVOLTAICA CON UN ESP32 *************/
/************** Fecha de inicio: 10/11/2017 -- Fecha de finalización: 14/12/2017 *************/
/************************************* THINGER.io ********************************************/
/************************* Resultados medianamente aceptables ********************************/
/******************************** MATERIALES NECESARIOS **************************************/

/*    - Pantalla LCD 16X2       x1 //Se alimenta a 5V y no a 3.3V.
      - RTC 3231                x1 //Tambien puede ser RTC 1307, ha de alimentarse a 5V.
      - Zocalo microSD          x1 //Se alimenta a 5V y no a 3.3V.
      - Tarjeta microSD         x1 //Cualquier tipo es posible.
      - Sensores de tension     x3 //Divisor resistivo con multivueltas.
      - Sensores de corriente   x3 //Sensor efecto hall 20A.
      - Fuente de alimentación  x1 //Mínimo 12V, 1A (no más de 5V).                          */

/********************************* Problemas generales ***************************************/

/* 1- El primer problema de este proyecto fue la integración del ESP32 en el IDE de Arduino
    que se solucionó cuando conseguí unir todos los archivos necesarios para el compilador

   2- El segundo problema era que de las 14 entradas analógicas sólo seis estaban disponibles,
    luego leí que no era posible "activar" las otras 8 estando el WiFi encendido y conectado.

   3- El bus de I2C (SDA: D21,SCL: D22) funciona perfectamente, solo que todos los dispositivos
    deben ser alimentados a 5V o se "volverá loco" y no hará bien su función.

   4- El bus SPI para el zocalo de SD es: MISO: D19,MOSI: D23,CLK: D18, CS: D5.

   5- Muestra bien los valores en la lcd, pero aún se boquea:

      5.1- La lcd deja de mostrar los valores y se para.

      5.2- La RTC cuando la lcd se para, da error (165/165/2165 165:165:85).
      
      5.3- Parece ser que es culpa del compilador (esptool.exe) y me rendí (ESP8266 BETTER) */

/************************************ IMPLEMENTACION ****************************************/
/*** BIBLIOTECAS ***/
#include <WiFi.h>//Para conectarse a una red WiFi
#include <ThingerESP32.h>//Para conectarse a Thinger.io
#include <SD.h>//Para la SD
#include <RTClib.h>//Para RTC
#include <Wire.h>//Para protocolo I2C
#include <LiquidCrystal_I2C.h>//Para LCD
#include <driver/adc.h>//Para controlar las entradas analógicas (innecesaria)
#include "FS.h"//No son necesarias
#include "SD.h"//No son necesarias
#include "SPI.h"//No son necesarias
#include "driver/i2c.h"//
/*** CONSTANTES ***/
#define USERNAME "lupoxan"//Nombre de usuario de Thinger.io
#define DEVICE_ID "Esp_32"//Nombre del dispositivo Thinger
#define DEVICE_CREDENTIAL "7#G9BVTY&tqZ"//Credenciales del dispositivo
#define SSID "MIWIFI_jZTH"//SSID Wifi
#define SSID_PASSWORD "PIso3Wazowski2018"//Password Wifi
#define MAX 50//Máximo para media de valores
#define TIME 1000//Tiempo de espera para mostrar valores
#define SENSORES 6//Número de pines analógicos
#define Chip_Select_SD 5//Pin al que se conectará la SD
#define DECIMAL 100.00//Para quitar los decimales
#define NOMBRE_ARCHIVO "/valores.txt"//Nombre del archivo en la SD
#define MAXVAL 60//Maximo de valores que guarda
#define MAXSENS 6//Maximo de sensores que hay
/*** ESTRUCTURAS DE DATOS ***/
typedef struct {
  char tension = 'v';//tipo tension
  char corriente = 'A';//tipo corriente
} tipo1;//Para decidir si es tipo V o I
typedef struct {
  float valores[MAXVAL];//Valor de cada sensor
  String fechora[MAXVAL];//Valor de la fecha y hora a la que lee
  tipo1 tp;//Para guardar I o V
} mediciones;//Para guardar los valores de las mediciones
/*** VARIABLES NECESARIAS ***/
ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);//Variable para Thinger.io
RTC_DS3231 RTC;//Variable de RTC tipo 3231
//RTC_DS1307 RTC;//Variable de RTC tipo 1307
LiquidCrystal_I2C lcd(0x27, 16, 2);//Variable para lcd0x3F
File dataFile;//Variable para SD
//Current: A0 --> Vp, A3 --> Vn, A6 --> D34
//Voltage: A7 --> D35, A4 --> D32, A5 --> D33
int sensores[MAXSENS] = {A0, A3, A6, A7, A4, A5}; //Vector con las entradas
/*** VARIABLES A CONDICIÓN ***/
mediciones vec[MAXSENS];//Vector de registro tipo mediciones
mediciones value;//Vble de registro tipo mediciones
String fecHora;//String para fecha y hora
char algo;//Vble que nos indica si es I o V
unsigned long presentime, ahora;//Para contar el tiempo
unsigned long previoustime = 0, antes = 0;//Para contar el  tiempo
int indice;//Para saber qué sensor es el que está leyendo
char buffer[128];//Añadir a un String
String strValues1;//String que guarda fechora
String strValues;//String que guarda valores
int enteroVal;//Para definir entero 'valores'
int decimalVal;//Para definir decimal 'valores'
float valor;//Para la tension y para la corriente
boolean tipo;//Para saber cuando mide tension (false) y cuando corriente (true)
int j = 0, i = 0, ind = 0;//Indice para mostrar por pantalla, grabar SD y mostrar Serial
boolean isFile = true;//Si hay un archivo llamado "valores.txt"
boolean isDetected = true;//Si lo ha leido ya
void setup() {
  Serial.begin(115200);//Iniciar comunicacion serial
  thing.add_wifi(SSID, SSID_PASSWORD);//Conectar a la red WiFi
  elementos_clave();//Add Resources
  output_input_pins();//Iniciar los pines a OUTPUT
  lcd.begin();//Iniciar pantalla lcd
  lcd.backlight();
  RTC.begin();//Iniciar RTC
  //RTC.adjust(DateTime(__DATE__, __TIME__));//Para configrar fecha y hora
  iniciar_tarjeta();//Inicializar la microSD.
  delay(1000);//Esperar 1s para estabilizar
}

void loop() {
  thing.handle();//Función para manejar Thinger.io (no entrar en bucle)
  DateTime now = RTC.now();//Variable para la RTC
  presentime = millis();//Millis() para añadir cadena
  ahora = millis();//Millis() para mostrar por lcd
  if (presentime - previoustime >= TIME) {
    previoustime = presentime;
    if (i > MAXVAL - 1 ) {
      i = 0;
    }
    sprintf(buffer, "%02d/%02d/%04d,%02d:%02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());
    value.fechora[i] = buffer;
    strValues = buffer;
    for (int j = 0; j <= MAXSENS - 1 ; j++) {
      if (j >= 0 && j <= 3 - 1) {
        algo = value.tp.tension;
        vec[j].valores[i] = medicion(false, j);
        sprintf(buffer, ",%.2f", vec[j].valores[i]);
        strValues += buffer;
      }
      else {
        algo = value.tp.corriente;
        vec[j].valores[i] = medicion(true, j);
        sprintf(buffer, ",%.2f", vec[j].valores[i]);
        strValues += buffer;
      }
    }
    i++;
    Serial.println(strValues);//Mostrar por Serial
    tarjeta(SD, NOMBRE_ARCHIVO, strValues);//Añadir a la tarjeta SD
  }
  /* Mostrar por pantalla lcd */
  if (ahora - antes >= TIME ) {
    antes = ahora;
    pantalla(vec, value);
  }
}

/* Hacer cálculos para tener los valores de tensión y corriente */
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

/*  */
void elementos_clave() {
  thing["LED_On_Board"] << digitalPin(2);
  thing["Lampara_cama"] << digitalPin(13);
  thing["Lampara_mesa"] << digitalPin(14);
  thing["Lampara_general"] << digitalPin(12);
  thing["Corrientes_tensiones"] >> [](pson & out) {
    out["I1"] = medicion(true, 3);
    out["I2"] = medicion(true, 4);
    out["I3"] = medicion(true, 5);
    out["V1"] = medicion(false, 0);
    out["V2"] = medicion(false, 1);
    out["V3"] = medicion(false, 2);
    out["P1"] = medicion(true, 3) * medicion(false, 0);
    out["P2"] = medicion(true, 4) * medicion(false, 1);
    out["P3"] = medicion(true, 5) * medicion(false, 2);
  };
}
void output_input_pins() {
  /*** OUTPUT ***/
  //int output_pins[]={2,13,12,14,27,26,25,15,4};
  pinMode(2, OUTPUT);//Led On_Board
  digitalWrite(2, LOW);
  pinMode(13, OUTPUT);//C1 Iluminacion
  digitalWrite(13, HIGH);
  pinMode(12, OUTPUT);//C2 Uso general
  digitalWrite(12, HIGH);
  pinMode(14, OUTPUT);//C3 Cocina y horno
  digitalWrite(14, HIGH);
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

void pantalla(mediciones mostrar[], mediciones value) {
  char tens_curnt;
  char nombre;
  int k;
  lcd.clear();
  if (ind > MAXSENS - 1) {
    ind = 0;
  }
  if (ind >= 0 && ind <= MAXSENS - 4) {
    tens_curnt = value.tp.tension;
    nombre = 'V';
    k = ind + 1;
  }
  else {
    tens_curnt = value.tp.corriente;
    nombre = 'I';
    k = ind - 2;
  }
  lcd.print(nombre);
  lcd.print(k);
  lcd.print(" = ");
  lcd.print(mostrar[ind].valores[0]);
  lcd.print(tens_curnt);
  lcd.setCursor(0, 1);
  lcd.print(value.fechora[0]);
  ind++;
}

void iniciar_tarjeta() {
  Serial.print(F("Iniciando SD ... "));
  lcd.setCursor(0, 0);
  lcd.print(F("Starting SD ... "));
  if (!SD.begin(Chip_Select_SD)) {
    Serial.println(F("Error al iniciar"));
    lcd.setCursor(0, 1);
    lcd.print(F("Error :("));
    return;
  }
  Serial.println(F("Correcto"));
  lcd.setCursor(0, 1);
  lcd.print(F("Correcto :)"));
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

void tarjeta(fs::FS &fs, const char * path, String  message) {
  /* Leer archivo */
  if (isFile == true) {
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;

    Serial.printf("Reading file: %s\n", path);

    file = fs.open(path);
    if (!file) {
      Serial.println("Failed to open file for reading");
      isFile = false;
      isDetected = false;
      return;
    }
    Serial.print("Read from file: ");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reading SD ...");
    /*while (file.available()) {
      Serial.write(file.read());
    }*/
    len = file.size();
    size_t flen = len;
    start = millis();
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    lcd.setCursor(0, 1);
    lcd.print("SD: ");
    lcd.print(flen);
    lcd.print(" bytes");
    file.close();
    delay(1000);
    isFile = false;
  }
  /* Escribir archivo */
  if (isFile == false && isDetected == false) {
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file for writing");
      return;
    }
    if (file.print("/---------- VALORES ISFV ----------/\r\n")) {
      Serial.println("File written");
      isDetected = true;
      isFile = false;
    } else {
      Serial.println("Write failed");
    }
    file.close();
  }
  /* Añadir al archivo */
  if (isDetected == true) {
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
      Serial.println("Failed to open file for appending");
      return;
    }
    if (file.print(message)) {
      Serial.println("Message appended");
    } else {
      Serial.println("Append failed");
    }
    file.println();
    file.close();
  }
}
