#include <ESP8266WiFi.h>//Biblioteca de conexión WiFi del módulo ESP8266
#include <ThingerESP8266.h>//Biblioteca de la plataforma thinger.io
//#include <WiFiClient.h>//Para cuando hace de servidor

#define SSID1 "MiFibra-D250"//SSID Wifi
#define PASS1 "P54XgDEt"//Password Wifi
#define usuario "lupoxan"//Nombre de usuario de thinger
#define device_Id "esp8266"//Nombre del dispositivo al que nos conectaremos
#define device_credentials "kuDHNF#c9a1Z"//Credenciales generadas por Thinger

ThingerESP8266 thing(usuario, device_Id, device_credentials);//Variable para acceder a Thinger

const int releMesa = 2;

void setup() {
  Serial.begin(115200);//Iniciar comunicacion serial
  pinMode(releMesa,OUTPUT);
  thing.add_wifi(SSID1, PASS1);//Conectar a la red WiFi
  thing["Mesa"] << digitalPin(releMesa);
}

void loop() {
  thing.handle();

}
