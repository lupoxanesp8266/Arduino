/*** BIBLIOTECAS ***/
#include <SD.h>//Para la SD
#include <RTClib.h>//Para RTC
#include <Wire.h>//Para protocolo I2C
#include <LiquidCrystal_I2C.h>//Para LCD
/*** CONSTANTES ***/
#define MAX 500//Máximo para media de valores
#define TIME 500//Tiempo de espera para mostrar valores
#define MAXCORRIENTE 20.00//Corriente máxima antes de dar alarma
#define MAXTENSION 15.00//Tension máxima antes de dar alarma
#define SENSORES 6//Número de pines analógicos
/*** VARIABLES NECESARIAS ***/
RTC_DS1307 RTC;//Variable de RTC
LiquidCrystal_I2C lcd(0x27, 16, 2);//Variable para lcd
File dataFile;//Variable para SD
int sensores[] ={A3,A6,A7,A0,A1,A2};//Vector con las entradas
/*** VARIABLES A CONDICIÓN ***/
const int Chip_Select_SD = 10;//Pin para SD
unsigned long presentime;//Para contar el tiempo
unsigned long previoustime = 0;//Para contar el  tiempo
int indice;//Para saber qué sensor es el que está leyendo en alarmas
float valor;//Para la tension y para la corriente
boolean tipo;//Para saber cuando mide tension (false) y cuando corriente (true)
boolean condicion;//La condicion de si salta la alarma (true)o no (false)
int j = 0;//Indice para mostrar por pantalla, grabar SD y mostrar Serial
/*** FUNCIONES ***/
float medicion(int, int);//Para hacer las mediciones
void alarmas();//Para dar las alarmas
void pantalla(boolean, int, float);//Para LCD
void mostrar_por_serial(boolean, int, float);//Para PC
void leer_tarjeta();//Para leer SD
void escribir_tarjeta(boolean, int, float);//Para escribir SD

void setup() {
  lcd.begin();
  lcd.backlight();
  Wire.begin();
  Serial.begin(115200);
  RTC.begin();
  leer_tarjeta();
  //RTC.adjust(DateTime(__DATE__, __TIME__));//Para ajustar fecha y hora
}
void loop(){
  presentime = millis();
  if(presentime - previoustime >= TIME){
    previoustime = presentime;
    if(j > SENSORES-1){
      j = 0;
    }
    if(j >= 0 && j <= 2){
      tipo = true;
    }
    if(j >= 3 && j <= SENSORES-1){
      tipo = false;
    }
    valor = medicion(tipo,j);
    pantalla(tipo,j,valor);
    mostrar_por_serial(tipo,j,valor);
    escribir_tarjeta(tipo,j,valor);
    j++;
  }
  //alarmas();//Descomentar si se quisiera notificar alguna alarma
}

float medicion(boolean tipo, int indice) {
  float value;
  float media = 0.00;
  if(tipo == true){//Corriente
    for(int i = 0; i <= MAX-1; i++){
      value = analogRead(sensores[indice]);
      value = ((5 * value / 1023) - 2.5) / 0.100;//0.185 for 5 A
      media = media + value;
      }
    }
  else{//Tensión
    for(int i = 0; i <= MAX-1; i++){
      value = analogRead(sensores[indice]);
      value = 5 * value * 4.8 / 1023;
      media = media + value;
      }
    }
  media = media / MAX;
  return media;

}

void pantalla(boolean tipo,int indice,float valor){
  DateTime now = RTC.now();
  if(tipo == true){//Corriente
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("I");
    lcd.print(indice+1);
    lcd.print(" = ");
    lcd.setCursor(6,0);
    lcd.print(valor);
    lcd.setCursor(12,0);
    lcd.print(" A");
  }
  else{//tensión
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("V");
    lcd.print(indice-2);
    lcd.print(" = ");
    lcd.setCursor(6,0);
    lcd.print(valor);
    lcd.setCursor(12,0);
    lcd.print("v");
  }
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
}

void mostrar_por_serial(boolean tipo, int indice, float valor){
  DateTime now = RTC.now();
  if(indice == 0){
    Serial.print(now.year(), DEC); // Año
    Serial.print('/');
    Serial.print(now.month(), DEC); // Mes
    Serial.print('/');
    Serial.print(now.day(), DEC); // Dia
    Serial.print(' ');
    Serial.print(now.hour(), DEC); // Horas
    Serial.print(':');
    Serial.print(now.minute(), DEC); // Minutos
    Serial.print(':');
    Serial.print(now.second(), DEC); // Segundos
    //Serial.print(" --- ");
  }
  if(tipo == true){//Corriente
    Serial.print(" -- ");
    Serial.print("I");
    Serial.print(indice+1);
    Serial.print(" = ");
    Serial.print(valor);
    Serial.print(" A");
  }
  else{
    Serial.print(" -- ");
    Serial.print("V");
    Serial.print(indice-2);
    Serial.print(" = ");
    Serial.print(valor);
    Serial.print(" v");
    if(indice == SENSORES-1){
      Serial.println("");
    }
  }
}

void leer_tarjeta(){
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
}

void escribir_tarjeta(boolean tipo, int indice, float valor){
  File logFile;
  DateTime now = RTC.now();
  logFile = SD.open("valores.txt", FILE_WRITE);
  if (logFile){
    if(indice == 0){
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
    }
    if(tipo == true){//Corriente
      logFile.print(",");
      logFile.print(valor);
    }
    else{
      logFile.print(",");
      logFile.print(valor);
      if(indice == SENSORES-1){
        logFile.println("");
      }
    }
    logFile.close();
    }
  else{
    Serial.println("Error al abrir el archivo");
    }
}

void alarmas(){
  for(int i = 0; i <= SENSORES-1; i++){
    if(i > SENSORES-1){
        i = 0;
      }
      if(i >= 0 && i <= 2){
        tipo = true;
        valor = medicion(tipo,i);
        if(valor >= MAXCORRIENTE){
          Serial.print("ATENCION --- I");
          Serial.print(i+1);
          Serial.print(" = ");
          Serial.print(valor);
          Serial.println(" A");
        }
      }
      if(i >= 3 && i <= SENSORES-1){
        tipo = false;
        valor = medicion(tipo,i);
        if(valor >= MAXTENSION){
          Serial.print("ATENCION --- V");
          Serial.print(i-2);
          Serial.print(" = ");
          Serial.print(valor);
          Serial.println(" v");
        }
      }
  }
}

