/************* MONITORIZACION DE UNA INSTALACION SOLAR FOTOVOLTAICA CON UN ESP32 *************/
/************** Fecha de inicio: 10/11/2017 -- Fecha de finalización: 00/00/0000 *************/
/************************************* THINGER.io ********************************************/

/******************************** MATERIALES NECESARIOS **************************************/

/*    - Pantalla LCD 16X2       x1 //Se alimenta a 5V y no a 3.3V.
      - RTC 3231                x1 //Tambien puede ser RTC 1307, ha de alimentarse a 5V.
      - Zocalo microSD          x1 //Se alimenta a 5V y no a 3.3V.
      - Tarjeta microSD         x1 //Cualquier tipo es posible.
      - Sensores de tension     x3 //Divisor resistivo con multivueltas.
      - Sensores de corriente   x3 //Sensor efecto hall 20A.
      - Fuente de alimentación  x1 //Mínimo 3.3V,500mA (no más de 5V).                       */

/********************************* Problemas generales ***************************************/

/* 1- El primer problema de este proyecto fue la integración del ESP32 en el IDE de Arduino
    que se solucionó cuando conseguí unir todos los archivos necesarios para el compilador

   2- El segundo problema era que de las 14 entradas analógicas sólo seis estaban disponibles,
    luego leí que no era posible "activar" las otras 8 estando el WiFi encendido y conectado.

   3- El bus de I2C (SDA: D21,SCL: D22) funciona perfectamente, solo que todos los dispositivos
    deben ser alimentados a 5V o se "volverá loco" y no hará bien su función.

   4- El bus SPI para el zocalo de SD es: MISO: D19,MOSI: D23,CLK: D18, CS: D5.              */

/************************************ IMPLEMENTACION ****************************************/
/*** BIBLIOTECAS ***/
#include <WiFiClientSecure.h>
#include <WiFi.h>//Para conectarse a una red WiFi
#include <ThingerESP32.h>//Para conectarse a Thinger.io

/*** CONSTANTES ***/
#define USERNAME "lupoxan"//Nombre de usuario de Thinger.io
#define DEVICE_ID "ifttt"//Nombre del dispositivo Thinger
#define DEVICE_CREDENTIAL "vj5gWDCB1W%t"//Credenciales del dispositivo
#define SSID "MOVISTAR_9DBC"//SSID Wifi
#define SSID_PASSWORD "RY64GqbfCCqeKmGUpJxN"//Password Wifi

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);//Variable para Thinger.io
unsigned long ahora;//Para contar el tiempo
unsigned long antes = 0;//Para contar el  tiempo
int calltime = 3;//Lo que viene siendo cada doce horas
int maxi = 9;
void setup() {
  thing.add_wifi(SSID, SSID_PASSWORD);//Conectar a la red WiFi
  elementos_clave();//Add Resources
  output_input_pins();//Iniciar los pines a OUTPUT
  Serial.begin(115200);//Iniciar comunicacion serial
}

void loop() {
  thing.handle();
  ahora = millis();
  if (ahora - antes >= calltime * 60000) {
      antes = ahora;
      call_prueba_endpoint();
      Serial.println("Ya :)");
    }
}

void elementos_clave() {
  thing["LED_On_Board"] << digitalPin(2);
  thing["Calltime"] << inputValue(calltime);//Esperemos que funcione
  thing["Valores"] << inputValue(maxi);
}

void call_prueba_endpoint(){
  pson prueba;
  int m;
  String strEndpoint;
  char buffer1[128];
  for (m = 0; m <= maxi - 1; m++) {
    sprintf(buffer1, " %d",m+1);
    strEndpoint += buffer1;
  }
  prueba["value1"] = strEndpoint;
  thing.call_endpoint("Prueba", prueba);
}
void output_input_pins() {
  pinMode(2, OUTPUT);//Led On_Board
  digitalWrite(2, LOW);
}

