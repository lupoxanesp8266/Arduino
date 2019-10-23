#include <ESP8266WiFi.h>//Biblioteca de conexión WiFi del módulo ESP8266
#include <ThingerESP8266.h>//Biblioteca de la plataforma thinger.io

#define SSID1 "MIWIFI_jZTH"//SSID Wifi
#define PASS1 "PIso3Wazowski2018"//Password Wifi
#define usuario "lupoxan"//Nombre de usuario de thinger
#define device_Id "ESP_8266"//Nombre del dispositivo al que nos conectaremos
#define device_credentials "#gHQ3ARec6Rh"//Credenciales generadas por Thinger
#define increase 0.2
#define TIME 100//Tiempo para mostrar valores
ThingerESP8266 thing(usuario, device_Id, device_credentials);//Variable para acceder a Thinger

const char WiFi_ssid[] = SSID1;          //Nombre de red
const char WiFi_password[] = PASS1; //Clave de red

const long A = 1000;     //Resistencia en oscuridad en KΩ
const int B = 15;        //Resistencia a la luz (10 Lux) en KΩ
const float Rc = 0.22;       //Resistencia calibracion en KΩ

const int muxSIG = A0;
const int muxS0 = D0;
const int muxS1 = D3;
const int muxS2 = D4;
const int muxS3 = D9;
unsigned long presentime;//Para contar el tiempo
unsigned long previoustime = 0;//Para contar el  tiempo
int desciende = -1, asciende = 1, k = 0;
float a = 2.00, b = 0.00, c = 0.00, d = 0.00, t = 0.00, t2 = 0.00, t3 = 0.00;
float vec[500];
void setup() {
  Serial.begin(115200);//Iniciar la transmisión Serial a 115200 bauds
  pinMode(muxS0, OUTPUT);
  pinMode(muxS1, OUTPUT);
  pinMode(muxS2, OUTPUT);
  pinMode(muxS3, OUTPUT);
  thing.add_wifi(WiFi_ssid, WiFi_password);// Inicialización de la WiFi para comunicarse con la API
  thing["Lux"] >> [](pson & out) {//Objeto que contiene las variables de luz y temperatura
    out["Tension_13v"] = entregar(vec);
  };
}

void loop() {
  presentime = millis();
  thing.handle();//Objeto que llama a la conexión con el servidor Thinger
  thing.stream(thing["Lux"]);
  Serial.println(medicion());
  if (presentime - previoustime >= TIME) {
    previoustime = presentime;
    for (int i = 0; i <= 500; i++) {
      vec[i] = medicion();
    }
  }
}
float entregar(float vec[]) {
  if (k >= 500) {
    k = 0;
  }
  k++;
  return vec[k];
}
float medicion() {
  SetMuxChannel(10);
  float tension = analogRead(muxSIG);
  tension = 3.3 * tension * 4 / 1024;
  return tension;
}
int SetMuxChannel(byte channel)
{
  digitalWrite(muxS0, bitRead(channel, 0));
  digitalWrite(muxS1, bitRead(channel, 1));
  digitalWrite(muxS2, bitRead(channel, 2));
  digitalWrite(muxS3, bitRead(channel, 3));
}
