#include <WiFiClientSecure.h>
#include <WiFi.h>//Para conectarse a una red WiFi
#include <ThingerESP32.h>//Para conectarse a Thinger.io
#include "DHT.h"
/*** CONSTANTES ***/
#define USERNAME "lupoxan"//Nombre de usuario de Thinger.io
#define DEVICE_ID "ifttt"//Nombre del dispositivo Thinger
#define DEVICE_CREDENTIAL "#K6gsoh0RtsG"//Credenciales del dispositivo
#define SSID "MOVISTAR_9DBC"//SSID Wifi
#define SSID_PASSWORD "RY64GqbfCCqeKmGUpJxN"//Password Wifi
#define DHTTYPE DHT11
//Variables necesarias para cálculo de Luxes en "how_to_lux()"
const long A = 1000;     //Resistencia en oscuridad en KΩ
const int B = 15;        //Resistencia a la luz (10 Lux) en KΩ
const int Rc = 5;       //Resistencia calibracion en KΩ
const int LDRPin = A0;   //Pin del LDR
DHT dht(14, DHTTYPE);

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);//Variable para Thinger.io
void setup() {
  thing.add_wifi(SSID, SSID_PASSWORD);//Conectar a la red WiFi
  elementos_clave();//Add Resources
  output_input_pins();//Iniciar los pines a OUTPUT
  Serial.begin(115200);//Iniciar comunicacion serial
  dht.begin();

}

void loop() {
  thing.handle();
}

void elementos_clave() {
  thing["LED_On_Board"] << digitalPin(2);
  thing["Lampara_mesa"] << digitalPin(13);
  thing["Lux_y_temp"] >> [](pson & out) {//Objeto que contiene las variables de luz y temperatura
    out["Lux"] = how_to_lux();
    out["Temp"] = temperatura();
  };
}
int how_to_lux() {
  int V;
  int ilum;
  V = analogRead(A0);
  ilum = ((long)(4096 - V) * A * 10) / ((long)B * Rc * V); //usar si LDR entre GND y A0
  //ilum = ((long)V*A*10)/((long)B*Rc*(4096-V));    //usar si LDR entre A0 y Vcc (como en el esquema anterior)
  Serial.print("LUX: ");
  Serial.println(ilum);
  return ilum;
}
void output_input_pins() {
  pinMode(2, OUTPUT);//Led On_Board
  pinMode(13, OUTPUT);//C1 Iluminacion
  pinMode(A0,INPUT);
  pinMode(A3,INPUT);
}
float mapeo(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
float temperatura(){
  float temp = dht.readTemperature();
  Serial.print("TEMP: ");
  Serial.println(temp);
  return temp;
}
