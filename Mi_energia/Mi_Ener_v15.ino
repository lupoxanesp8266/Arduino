// Fco Javier Arias Senso

/*
 3-6-2015 Versión v9
 Biblioteca Mi_Energia 
    Terminada (con trifásica). Precisión con carga resistiva FVI = 0,999XXX
 Testeada día graduación 28-5-2015
 Últimos cambios:
   - Calibración sensores:
        - si modificamos los factores V e I dos veces sin guardar los acumulaba
   - No guardaba, en el 'log' la fecha-hora correcta (tan sólo la de inicio del programa)
   - Modificada librería SD'. SD.begin() tan sólo se podía ejecutar una vez -> no cambiar SD sin reiniciar programa
   - La variable fase (fase activa) la declaré como global. Siempre comienza con la Fase_1 como activa
 Temas pendientes:
   - Si fase sin señal no permite cambiar la fase activa
       Esto ocurre cuando el tiempo entre medidas es inferior al timeout de Medir(...)
   - Calibrar sensor sin corriente se sale sin mensaje (a veces ¿?)
   - Reestudiar menus
 4-6-2015 Versión v10
 Cambiado nombre menú: Medidas -> General
 Menu General:
     - Rediseñado:
       - Seg / Med
       - Ciclos / Med
       - Tipo Red Mono / Tri
       - Mostrar N Graf / M Tbl
       - Mostrar Fase_Act Todas
 4-6-2015 Versión v10
 Temas pendientes:
     - Plantear:  espaciado entre líneas (modo texto)
     - Menú oculto (pulsar C-I-S del título)
           - Borrado EEPROM
           - Generar fich con config EEPROM
           - Modo SERIAL
           - Modo DEBUG
     - Menú Registro
           - Añadir Nombre: DATOSLOG.log  AAMMDDXX.log
 10-6-2015 v10
  Menu Configurar:
     - Rediseñado:
       - Fecha y Hora
       - General
       - Registro
       
   Menu General:
     - Rediseñado:
       - ID_Dispositivo
       - Seg / Med
       - Ciclos / Med
       - Tipo Red Mono / Tri
       - Mostrar N Graf / M Tbl
       - Mostrar Fase_Act Todas

    Menú Registro:
      Añadida selección fichero autmomático y manual.
       - Automático:      Datalog.log
                              - Reiniciar
                              - Agregar
       - Manual:          aammddXX.log
       
 13-6-2015 v11
      Añadida opción 'Guardar Muestras' a menú Conf. Registro
      
 24-06-2015
 
     Optimizado Timeout en obtención muestras (se trata como una función del Num_Ciclos)
     Coordinados los valores de Tiempo_Entre_Lecturas, Tipo_Red y Num_Ciclos / Lecturas
     ( El Tiempo entre lecturas no podrá ser inferior a 1, ya que los procesos adicionales 
     consumen 1,5 segundos en el peor de los casos (Presentación datos, SD valores y muestras, vía serie, radio ..)
     
 24-06-2015
 
     Añadido FileStamp Date-Time a los ficheros 'log'
     
 25-06-2015
 
     Midiendo trifásica (motores en triángulo), los FP no eran correctos.
     Hechos observados: desplazamiento de los niveles de contínua tanto de tensión como de corriente
                       dependiendo de cada fase
     Recojo datos de las medidas (*.Log y *.dat)
     
 26-06-2015
 
       Tras simular los cálculos, tanto en Arduino como en Excel, los resultados no se pueden mejorar
     cambiando, tan solo, el nivel de contínua de las medidas. (Problema adiccional: hasta ahora se
     guardan los valores de las muestras tras aplicar las FVI's obtenidas tras la última calibración)
       
       Probablemente los problemas provengan de las calibraciones efectuadas con un nivel de contínua 
     no nulo.
     
     Medidas:
         - Modificar, tanto sketch como librería, para que se guarden las muestras obtenidas directamente
        del conversor A/D.
             Esto fué una pesadilla. La rutina de Guardar_SD tuve que dividirla en dos: Guardar_SD_Valores y
           Guardar_SD_Muestras, ejecutarlas en instantes distintos y desde el punto de vista de programación
           implementarlas en lugares distintos ('Valores' en el Sketch y 'Muestras' en la biblioteca)
        - Introducir una nueva función en el procesado de las medidas para que su valor medio siempre fuera nulo.
   
     Pruebas:
        - Todo funcionó com estaba previsto -> casi perfecto
        - Test:
             - Uno o dos motores tanto en triángulo como en estrella
     
     Los nuevos módulos: Mi_Ener_v14.ino y Energía.cpp

 27-06-2015

     -  Modifiqué 'Tomar_Muestras': los valores de las tablas de V e I para la muestra 0 no eran correctos,
        ya que siempre se les añadía el íltimo valor muestreado (pico en I al final del semiciclo negativo)
     -  Modifiqué 'Calc_Desf_VI': al valor tablaI[numMuestras] no se le aplicaba la parte decimal del desfase
     
      Finalmente: consigo 3 nueves y 1 siete en el cálculo del FP
*/

  // Controles compilación  ( Ojo: Visibilidad -> en las 'lib' no se ven)

#define TFT_ST7783
//#define TFT_S6D0154

//#define MI_DEBUG
//#define SERIALPRINT

#define VERSION 1

  // Bibliotecas y sus parámetros

#include <Adafruit_GFX.h>           // Core graphics library

#ifdef TFT_ST7783
    #include <SWTFT.h>                  // TFT Hardware-specific library
#else
    #ifdef TFT_S6D0154
        #include <TFTLCD.h>
            #define LCD_CS A3
            #define LCD_CD A2
            #define LCD_WR A1
            #define LCD_RD A0
            #define LCD_RESET A4       
    #endif
#endif

    #define BLACK   0x0000
    #define BLUE    0x001F
    #define RED     0xF800
    #define GREEN   0x07E0
    #define CYAN    0x07FF
    #define MAGENTA 0xF81F
    #define YELLOW  0xFFE0
    #define WHITE   0xFFFF
    
#include <TouchScreen.h>

    #define YP A1  // must be an analog pin, use "An" notation! A1
    #define XM A2  // must be an analog pin, use "An" notation! A2
    #define YM 7   // can be a digital pin                      7
    #define XP 6   // can be a digital pin                      6
   
    #define MINPRESSURE 50
    #define MAXPRESSURE 1000

#include <EEPROM.h>
#include "Mi_EEPROM.h"

#include <SD.h>            // Atención SD modificada por Bennet: https://code.google.com/p/arduino/issues/detail?id=465
    #define SD_CS 10

#include <Wire.h>

#include <RTClib.h>

#include "Mi_Energia.h"
extern int tablaV[];
extern int tablaI[];

// Nuevas variables

struct Punto
{
  int x;
  int y;
} punto;                  // 4 bytes

struct Punto_cf
{
  int col;
  int fila;
} punto_cf;               // 4 bytes

// Instancias de objetos

#ifdef TFT_ST7783
    SWTFT tft;
#else
    #ifdef TFT_S6D0154
        Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
    #endif
#endif

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 400);    // 300 -> Resistencia (ohmios) entre X+ y X- del Touch
File fichLog;
File fichMuesLog;
RTC_DS1307 rtc;
Ener_IESCJ ener_IESCJ_1;

// Variables

String strFecHora;

byte  PENRADIUS=2;                   // Tamaño punto

      // Coeficientes calibración Touch
      
int coefX1 = 897, coefX2 = 118;
int coefY1 = 143, coefY2 = 874 ;

      // Varios
 
struct Fase fase_1, fase_2, fase_3;
struct Fase *fase;

byte  ID_Dispo;                         // Identificador para comunicaciones
unsigned int  seg_Entre_Medidas;        // Tiempo entre muestras
unsigned int  seg_Entre_Registros;      // Tiempo entre Registros
byte  Ciclos_Medida;                    // Ciclos por medida
byte  NGraf;                            // Mostrar: N_Graf / N_Texto
byte  NTexto;                           // Mostrar: N_Graf / N_Texto
boolean  red_Tri;                       // Red Mono o trifásica
boolean  presenTodas;                   // Si trifásica, en modo texto, mostrar Fase_Activa, Todas
boolean  nomFich_Aut;                   // Log: Datalog.log AAMMDDXX.log
boolean  init_Log;                      // Si Datalog.log: reiniciar o añadir
boolean  MuesLog;                       // Guardar muestras
char  sepCampos;                        // Separador de campos en el 'log'

char buffer[128];
unsigned int numBucles = 0;
unsigned int numMuestras;  		// Número de muestras realmente obtenidas

void setup()
{  
  pinMode(SD_CS, OUTPUT);
  
  Serial.begin(115200);

  tft.reset();
  uint16_t identifier = tft.readID();
 
 // EEPROM.write(0, 0);        // Destruimos el formato de la cabecera de la EEPROM

#ifdef MI_DEBUG
  Serial.print(F("LCD driver chip: "));  
  Serial.println(identifier, HEX);
  Serial.print("Compilado para Pantalla: ");
  #ifdef TFT_ST7783 
    Serial.println("ST7783");
  #else
    Serial.println("S6D0154");
  #endif  
  Serial.print("  SRAM Libre: "); 
  Serial.println(freeRam());
#endif    // MI_DEBUG

  tft.begin(identifier);
  tft.setRotation(3);              // Apaisado Botón Reset a la derecha -> Punto(x,y), "x" (0 a 319), de izq a derecha, "y" (0 a 239) -> de arriba a abajo
  BorrarPantalla();
 
  Leer_Aplicar_Param_EEPROM();

  Wire.begin();
  rtc.begin();
  
  if (! rtc.isrunning()) Serial.println("RTC no detectado");

  fase = &fase_1;        // Establecemos como fase activa la 1

  Menu();      // Con espera
   
}  // Setup()

void loop()
{
}    // loop()

void Analizar()
{
  unsigned long tiempo;
  unsigned long tiemLecturas;
  unsigned int timeout;
  DateTime now;
  boolean detener;
  boolean salir;
  long micros_Entre_Medidas = seg_Entre_Medidas * 1000000L;
  int tmpInt;
  boolean monSer = true;

  BorrarPantalla();

  tft.setTextColor(MAGENTA);
  Mostrar_Texto(2, 7, "Abriendo fichero(s) Log");
  Abrir_Log();
  Borrar_Texto(2, 7, 23);        // Abriendo ...
  
  fase_1.Energia = 0;
  fase_2.Energia = 0;
  fase_3.Energia = 0;

  timeout = (Ciclos_Medida + 3) * 20;
  numBucles = 0;
  
  do
  {
    tiempo = micros();
    now = rtc.now();
    sprintf(buffer, "%04d/%02d/%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    strFecHora = buffer;
    
    tiemLecturas = ener_IESCJ_1.Medir(fase, Ciclos_Medida, timeout);                 // Lecturas y cálculos. Lee 10 periodos, el time-out en milisegundos
    //sprintf(buffer, "Tiempo lecturas (%d ciclos): %ld", Ciclos_Medida, micros()-tiempo);
    //Serial.println(buffer);
    fase_1.Energia += fase_1.Pot_Act * seg_Entre_Medidas / 3600.0;            // En wh
    if (red_Tri)
    { 
      fase_2.Energia += fase_2.Pot_Act * seg_Entre_Medidas / 3600.0;          // En wh
      fase_3.Energia += fase_3.Pot_Act * seg_Entre_Medidas / 3600.0;          // En wh
    }
    
    numBucles++;
    
    if ((numBucles % seg_Entre_Registros) == 0)
    {
        tmpInt = numBucles % (NGraf + NTexto);
        if ((NGraf == 0) || (NTexto >= NGraf) && (tmpInt != 0) || ((NTexto < NGraf) && (tmpInt == 0) && (NTexto != 0)))
          Mostrar_Result_TFT(fase, "Datos", tiemLecturas);
        else
          Mostrar_Result_TFT(fase, "Graf", tiemLecturas);
    }
  
#ifdef DEBUF 
    Serial.print("  SRAM Libre: "); Serial.println(freeRam());
#endif

    if (fichLog) Grabar_SD_Valores(tiemLecturas); 
    
    if (monSer) EnviarSerie();
    
    sprintf(buffer, "Tiempo Total (%d ciclos): %ld", Ciclos_Medida, micros()-tiempo);
    Serial.println(buffer);
    
    salir = false;
    while ((micros() - tiempo) < micros_Entre_Medidas)
    {
        detener = false;
        do
        {
          if (Lee_Punto_cf()) 
          {
            if (punto_cf.fila == 13)
            {
                switch (punto_cf.col)
                {
                    case 20:
                      fase = &fase_1;
                      break;
                      
                    case 22:
                      fase = &fase_2;
                      break;
                      
                    case 24:
                      fase = &fase_3;
                      break;
                }
            }

            if ((punto_cf.fila == 14) && punto_cf.col < 18)
            {
                if (punto_cf.col < 9)          // Detener/Continuar
                {
                    detener = !detener;
                    Borrar_Texto(0, 14, 9);
                    if (detener)
                         Mostrar_Texto(0, 14, "Continuar");
                    else
                    {
                        Mostrar_Texto(0, 14, "Detener ");
                        break;
                    }
                }
                else
                  if (punto_cf.col > 10)        // Salir
                  {
                      salir = true;
                      break;
                  }
            }
            delay(500);
          }
        } while(detener);  
    }
    
    if (salir)
      break;
      
  } while(true);
  
  if (fichLog)
  {
    fichLog.close();
    fichMuesLog.close();
  }
  
  return;
  
}    // Analizar()

void Leer_Aplicar_Param_EEPROM()
{
 
        // Leemos la cabecera de la EEPROM

    EEPROM_Leer(0,cabEE);
    
    if ((cabEE.Id1 != 'C') || (cabEE.Id2 != 'J') || (cabEE.Version != VERSION))
    {
      tft.setTextColor(RED);
      Mostrar_Texto(6, 7, "EEProm invalida");
      delay(2000);
      Grabar_EEPROM_STD();                                          // Grabamos una por defecto (Completa)
      tft.fillScreen(BLACK);
      Calibrar_TOUCH();
      BorrarPantalla();
    }
      // Establecemos los parámetros obtenidos de la cabecera

    coefX1 = cabEE.CAL_coefX1;
    coefX2 = cabEE.CAL_coefX2;
    coefY1 = cabEE.CAL_coefY1;
    coefY2 = cabEE.CAL_coefY2;
    
    ID_Dispo = cabEE.ID_Dispo;
    seg_Entre_Medidas = cabEE.Seg_Entre_Medidas;
    seg_Entre_Registros = cabEE.Seg_Entre_Registros;
    Ciclos_Medida = cabEE.Ciclos_Medida;
    red_Tri = cabEE.Red_Tri;
    NGraf = cabEE.NGraf;
    NTexto = cabEE.NTexto;
    presenTodas = cabEE.presenTodas;
    init_Log = cabEE.Init_Log;
    nomFich_Aut = cabEE.nomFich_Aut;
    MuesLog = cabEE.MuesLog;
    sepCampos = cabEE.SepCampos;

      // Leemos y establecemos los parámetros de los sensores

    EEPROM_Leer(sizeof(cabEE),sensores);

    fase_1.numFase = 1;
    fase_1.PIN_V = 8;
    fase_1.PIN_I = 9;
    fase_1.CAL_V = sensores.sensor1.CAL_V;
    fase_1.CAL_I = sensores.sensor1.CAL_I;
    fase_1.CAL_FVI = sensores.sensor1.CAL_FVI;
    fase_1.CAL_OFFV = sensores.sensor1.CAL_OFFV;
    fase_1.CAL_OFFI = sensores.sensor1.CAL_OFFI;
   
    fase_2.numFase = 2;
    fase_2.PIN_V = 10;
    fase_2.PIN_I = 11;
    fase_2.CAL_V = sensores.sensor2.CAL_V;
    fase_2.CAL_I = sensores.sensor2.CAL_I;
    fase_2.CAL_FVI = sensores.sensor2.CAL_FVI;
    fase_2.CAL_OFFV = sensores.sensor2.CAL_OFFV;
    fase_2.CAL_OFFI = sensores.sensor2.CAL_OFFI;

    fase_3.numFase = 3;
    fase_3.PIN_V = 12;
    fase_3.PIN_I = 13;
    fase_3.CAL_V = sensores.sensor3.CAL_V;
    fase_3.CAL_I = sensores.sensor3.CAL_I;
    fase_3.CAL_FVI = sensores.sensor3.CAL_FVI;
    fase_3.CAL_OFFV = sensores.sensor3.CAL_OFFV;
    fase_3.CAL_OFFI = sensores.sensor3.CAL_OFFI;
  
    return;
}    // Leer_Aplicar_Param_EEPROM()

void Monitorizar()
{
}    // Monitorizar()

void Mostrar_Result_TFT(struct Fase *fase, String tipoPresen, unsigned long tiempo)
{
  char strTmp[10];
  char strTmp2[10];
  char strTmp3[10];
  unsigned int nrTrama;
  int max_V = 0, max_I = 0;
  long VI, max_VI = 0;
  
  tft.fillScreen(BLACK);
  
  tft.setTextColor(GREEN);
  tft.setTextSize(2); Mostrar_Texto(5, 0, "IES Ciudad Jardin");
  tft.setTextSize(1); tft.setCursor(84, 20); tft.print(strFecHora); tft.print("  -  "); tft.println(numBucles);
 
  tft.setTextColor(YELLOW);

#ifdef SREIALPRINT
  Serial.print("CAL_V: "); Serial.println(dtostrf(fase->CAL_V, 8, 6, strTmp));
  Serial.print("CAL_I: "); Serial.println(dtostrf(fase->CAL_I, 8, 6, strTmp));
#endif
  
  if  (tipoPresen == "Datos")
  {
    if (red_Tri && presenTodas)
    {
      tft.setTextSize(1);
      tft.print("\n\n\n");
      sprintf(buffer,"  Vef:     %s  %s  %s\n", dtostrf(fase_1.Vef, 8, 2, strTmp), dtostrf(fase_2.Vef, 8, 2, strTmp2), dtostrf(fase_3.Vef, 8, 2, strTmp3));tft.print(buffer);
      sprintf(buffer,"  Ief:     %s  %s  %s\n", dtostrf(fase_1.Ief, 8, 2, strTmp), dtostrf(fase_2.Ief, 8, 2, strTmp2), dtostrf(fase_3.Ief, 8, 2, strTmp3)); tft.print(buffer);
      sprintf(buffer,"  P_Act:   %s  %s  %s\n", dtostrf(fase_1.Pot_Act, 8, 2, strTmp), dtostrf(fase_2.Pot_Act, 8, 2, strTmp2), dtostrf(fase_3.Pot_Act, 8, 2, strTmp3)); tft.print(buffer);
      sprintf(buffer,"  P_Reac:  %s  %s  %s\n", dtostrf(fase_1.Pot_Reac, 8, 2, strTmp), dtostrf(fase_2.Pot_Reac, 8, 2, strTmp2), dtostrf(fase_3.Pot_Reac, 8, 2, strTmp3)); tft.print(buffer);
      sprintf(buffer,"  P_Apa:   %s  %s  %s\n", dtostrf(fase_1.Pot_Apa, 8, 2, strTmp), dtostrf(fase_2.Pot_Apa, 8, 2, strTmp2), dtostrf(fase_3.Pot_Apa, 8, 2, strTmp3)); tft.print(buffer);
      sprintf(buffer,"  F_Pot:   %s  %s  %s\n", dtostrf(fase_1.FP, 8, 4, strTmp), dtostrf(fase_2.FP, 8, 4, strTmp2), dtostrf(fase_3.FP, 8, 4, strTmp3)); tft.print(buffer);
      sprintf(buffer,"  Ang_D:   %s  %s  %s\n", dtostrf(fase_1.Ang_Desf, 8, 2, strTmp), dtostrf(fase_2.Ang_Desf, 8, 2, strTmp2), dtostrf(fase_3.Ang_Desf, 8, 2, strTmp3)); tft.print(buffer);
      sprintf(buffer,"  Energia: %s  %s  %s\n", dtostrf(fase_1.Energia, 8, 2, strTmp), dtostrf(fase_2.Energia, 8, 2, strTmp2), dtostrf(fase_3.Energia, 8, 2, strTmp3)); tft.print(buffer);
      
      // Totales
       
      float Pot_Act, Pot_Reac, Pot_Apa, Energia;
      
      Pot_Act = fase_1.Pot_Act + fase_2.Pot_Act + fase_3.Pot_Act;
      Pot_Reac = fase_1.Pot_Reac + fase_2.Pot_Reac + fase_3.Pot_Reac;
      Pot_Apa = fase_1.Pot_Apa + fase_2.Pot_Apa + fase_3.Pot_Apa;
      Energia = fase_1.Energia + fase_2.Energia + fase_3.Energia;
      
      tft.setCursor(246, 8*8); sprintf(buffer,"  %s\n", dtostrf(Pot_Act, 8, 2, strTmp)); tft.print(buffer);
      tft.setCursor(246, 9*8); sprintf(buffer,"  %s\n", dtostrf(Pot_Reac, 8, 2, strTmp)); tft.print(buffer);
      tft.setCursor(246, 10*8); sprintf(buffer,"  %s\n", dtostrf(Pot_Apa, 8, 2, strTmp)); tft.print(buffer);
      tft.setCursor(246, 13*8); sprintf(buffer,"  %s\n", dtostrf(Energia, 8, 2, strTmp)); tft.print(buffer);
    }
    else
    {
      // Monofásico
      
      tft.setTextSize(2);
      sprintf(buffer,"Vef:     %s", dtostrf(fase->Vef, 8, 2, strTmp)); Mostrar_Texto(5, 3, buffer);
//      sprintf(buffer,"Vef:     %8.2f", fase->Vef); Mostrar_Texto(5, 3, buffer);
      sprintf(buffer,"Ief:     %s", dtostrf(fase->Ief, 8, 2, strTmp)); Mostrar_Texto(5, 4, buffer);
      sprintf(buffer,"P_Act:   %s", dtostrf(fase->Pot_Act, 8, 2, strTmp)); Mostrar_Texto(5, 5, buffer);
      sprintf(buffer,"P_Reac:  %s", dtostrf(fase->Pot_Reac, 8, 2, strTmp)); Mostrar_Texto(5, 6, buffer);
      sprintf(buffer,"P_Apa:   %s", dtostrf(fase->Pot_Apa, 8, 2, strTmp)); Mostrar_Texto(5, 7, buffer);
      sprintf(buffer,"F_Pot:   %s", dtostrf(fase->FP, 8, 6, strTmp)); Mostrar_Texto(5, 8, buffer);
      sprintf(buffer,"Ang_D:   %s", dtostrf(fase->Ang_Desf, 8, 2, strTmp)); Mostrar_Texto(5, 9, buffer);
      sprintf(buffer,"Energia: %s", dtostrf(fase->Energia, 8, 2, strTmp)); Mostrar_Texto(5, 10, buffer);
    
#ifdef MI_DEBUG

      tft.setTextSize(1); 
      tft.print(F("\n\n\n"));
      sprintf(buffer, " Bucles: %6u  ", numBucles); tft.print(buffer);
      sprintf(buffer, "Muestras: %3u  ", numMuestras); tft.print(buffer);
      sprintf(buffer, " Tiempo: %8ld  ", tiempo); tft.println(buffer);
      
      sprintf(buffer, " Desfase Aplicado : %s", dtostrf(fase->CAL_FVI, 4, 2, strTmp)); tft.println(buffer);
      
      tft.print(" Medias (V, I) : ");
      sprintf(buffer,"%s", dtostrf(fase->vMed, 8, 3, strTmp)); tft.print(buffer);
      sprintf(buffer,", %s", dtostrf(fase->iMed, 8, 3, strTmp)); tft.println(buffer);
      
      tft.print(" Maximos (V, I) : ");
      sprintf(buffer,"%s", dtostrf(fase->max_V, 8, 3, strTmp)); tft.print(buffer);
      sprintf(buffer,", %s", dtostrf(fase->max_I, 8, 3, strTmp)); tft.println(buffer);
    
#endif
    }
  }
  else
  {
    // Gráfico apaisado (rot(3) -> Reset TFT a la derecha, la 'x' como siempre, la 'y' de arriba a abajo). Esq Sup_Izq -> (0,0), Esq Inf_Der (320,240)
    
    unsigned int offset_tbl;
    float coordY_V, coordY_I, coordY_VI;
    float escY_V, escY_I, escY_VI;
    const int ALT_GRAF = 90;
    const int MED_GRAF = 130;
    int alto;
    unsigned int ultY_V, ultY_I, ultY_VI;
    
    // Altura máxima del gráfico ALT_GRAF

    offset_tbl = (fase->numFase - 1) * 100;
    
    max_V = fase->max_V;
    max_I = fase->max_I;
    max_VI = fase->max_VI;

    escY_V = (float) ALT_GRAF * 0.85 / max_V;       // El 85%
 
    if (max_V < 5)
      escY_V = 0;
    else
    {
      escY_VI = (float) ALT_GRAF / max_VI;            // El 100% de la altura dedicada a los gráficos
      escY_I = (float) ALT_GRAF * 0.65 / max_I;       // el 65%
    }
    
    if (max_I < 5)
    {
      escY_VI = 0;
      escY_I = 0;
    }
    else
    {
      escY_VI = (float) ALT_GRAF / max_VI;            // El 100% de la altura dedicada a los gráficos
      escY_I = (float) ALT_GRAF * 0.65 / max_I;       // el 65%
    }
    
    // Ejes
    
    tft.drawLine(4, MED_GRAF, 200, MED_GRAF, MAGENTA);                                  // Eje de adcisas
    tft.drawLine(8, MED_GRAF - ALT_GRAF - 10, 8,MED_GRAF + ALT_GRAF + 10, MAGENTA);     // Eje de ordenadas

#ifdef MI_DEBUG

    // Líneas Valores máximos
    
    alto = ALT_GRAF * 0.85;
    tft.drawLine(4, MED_GRAF + alto , 200, MED_GRAF + alto, YELLOW);
    tft.drawLine(4, MED_GRAF - alto , 200, MED_GRAF - alto, YELLOW);
    
    alto = ALT_GRAF * 0.65;
    tft.drawLine(4, MED_GRAF + alto , 200, MED_GRAF + alto, CYAN);
    tft.drawLine(4, MED_GRAF - alto , 200, MED_GRAF - alto, CYAN);
    
    tft.drawLine(4, MED_GRAF + ALT_GRAF , 200, MED_GRAF + ALT_GRAF, RED);
    tft.drawLine(4, MED_GRAF - ALT_GRAF , 200, MED_GRAF - ALT_GRAF, RED);
    
#endif    // MI_DEBUG
 
    // Gráfico de líneas
    
     int zoom = 2;
     uint16_t eje_x=8;
     
     nrTrama = 0;

     tablaV[offset_tbl + numMuestras] = tablaV[offset_tbl];
     tablaI[offset_tbl + numMuestras] = tablaI[offset_tbl];
     tablaV[offset_tbl + numMuestras + 1] = tablaV[offset_tbl + 1];
     tablaI[offset_tbl + numMuestras + 1] = tablaI[offset_tbl + 1];

     ultY_V =  MED_GRAF - escY_V * tablaV[offset_tbl];
     ultY_I =  MED_GRAF - escY_I * tablaI[offset_tbl];
     ultY_VI =  MED_GRAF - escY_V * tablaV[offset_tbl] * escY_I * tablaI[offset_tbl];     
     
     for(nrTrama = offset_tbl + 1; nrTrama < offset_tbl + numMuestras + 2 ; nrTrama++ )      // Zoom:  horizontal
     {
        coordY_V = MED_GRAF - escY_V * tablaV[nrTrama];
        tft.drawLine(eje_x-1, ultY_V, eje_x, coordY_V, YELLOW);
        coordY_I = MED_GRAF - escY_I * tablaI[nrTrama];
        tft.drawLine(eje_x-1, ultY_I, eje_x, coordY_I, CYAN);
        coordY_VI = MED_GRAF - escY_VI * tablaV[nrTrama] * tablaI[nrTrama];
        tft.drawLine(eje_x-1, ultY_VI, eje_x, coordY_VI, RED);
        ultY_I =  coordY_I;
        ultY_V =  coordY_V;
        ultY_VI =  coordY_VI;
        eje_x += zoom;
     }
  
     // Datos en gráficos
     
     tft.setTextColor(WHITE);
     tft.setTextColor(YELLOW); tft.setCursor(244, 6*8);  sprintf(buffer,"%s V\n", dtostrf(fase->Vef, 8, 2, strTmp)); tft.print(buffer);
     tft.setTextColor(CYAN); tft.setCursor(244, 7*8);  sprintf(buffer,"%s A\n", dtostrf(fase->Ief, 8, 2, strTmp)); tft.print(buffer);
     tft.setTextColor(WHITE); tft.setCursor(244, 8*8);  sprintf(buffer,"%s W\n", dtostrf(fase->Pot_Act, 8, 2, strTmp)); tft.print(buffer);
     tft.setCursor(244, 9*8);  sprintf(buffer,"%s VAR\n", dtostrf(fase->Pot_Reac, 8, 2, strTmp)); tft.print(buffer);
     tft.setCursor(244, 10*8); sprintf(buffer,"%s VA\n", dtostrf(fase->Pot_Apa, 8, 2, strTmp)); tft.print(buffer);
     tft.setCursor(244, 11*8); sprintf(buffer,"%s\n", dtostrf(fase->FP, 8, 2, strTmp)); tft.print(buffer);
     tft.setCursor(244, 12*8); sprintf(buffer,"%s\n", dtostrf(fase->Ang_Desf, 8, 2, strTmp)); tft.print(buffer);
     tft.setCursor(244, 13*8); sprintf(buffer,"%s Wh\n", dtostrf(fase->Energia, 8, 2, strTmp)); tft.print(buffer);
     tft.setTextColor(RED); tft.setCursor(268, 15*8); tft.print("P_Inst");
     tft.setTextColor(YELLOW); 
  }

  tft.setTextSize(1);
  tft.setCursor(238, 196); tft.print("Fase Activa");  
  tft.setTextSize(2);
  Mostrar_Box(20, 13);
  Mostrar_Box(22, 13);
  Mostrar_Box(24, 13);
  tft.fillCircle(222 + fase->numFase*24, 214, 3, CYAN);        // Radio = 3
  
  tft.setTextSize(2);
  Mostrar_Texto(0, 14, "Detener    Salir");
//                      012345678901234567890
 
  return;
  
}    // Mostrar_Result_TFT

/*
  Si el nombre del fichero está establecido en automático (EEPROM->nomFich_Aut): 
    - Se crea un fichero con nombre aammddXX.log. XX es un NR que comienza en 01
    - Atención:  tras 99 ficheros -> Problemas
  Si el nombre del fichero está establecido en fijo:
    - Se crea o se añade si ya existía y así está configurado (EEPROM->init_Log)
      el fichero Datalog.log'
*/
void Abrir_Log()
{
  DateTime now;
  String nomFich = "Datoslog.log";
  byte tmpByte = 0;

  if (SD.begin(SD_CS))
  {
    if (nomFich_Aut)
    {
        now = rtc.now();
        do
        {
          sprintf(buffer, "%02d%02d%02d%02d.log", now.year()-2000, now.month(), now.day(), ++tmpByte);      // aammddXX.log
        } while (SD.exists(buffer));
    }
    else
    {
      nomFich.toCharArray(buffer,nomFich.length()+1);
      if (SD.exists(buffer) && init_Log)
          SD.remove(buffer);
    }
    
    // Fichero Valores    

    SdFile::dateTimeCallback(dateTime);
    fichLog = SD.open(buffer, FILE_WRITE);
    if (fichLog && (init_Log || nomFich_Aut))
        Grabar_SD_Cab();
  
    // Fichero Muestras
    
    if (MuesLog)
    {
        strcpy(&buffer[9], "dat");
        if (SD.exists(buffer) && init_Log)
          SD.remove(buffer);       
        fichMuesLog = SD.open(buffer, FILE_WRITE);
        if (fichMuesLog && (init_Log || nomFich_Aut))
            Grabar_SD_Cab_Mues();
    }    

  }
  return;
}    // Abrir_Log()

void Grabar_SD_Cab()
{
  if (fichLog)
  {
    fichLog.println("Fecha\tHora\tNr\tFase\tVef\tIef\tP_Act\tP_Reac\tP_Apa\tFP\tAng_D\tEnergia");
    fichLog.flush();
  }
}    // Grabar_SD_Cab()

void Grabar_SD_Cab_Mues()
{
  char strTmp[10];
  char strTmp2[10];
  char strTmp3[10];
  
  if (fichMuesLog)
  {
    sprintf(buffer, "Ciclos_Medida: %d", Ciclos_Medida);
    fichMuesLog.println(buffer);
    fichMuesLog.println("    Fase   Factor_V  Factor_I  Desf_VI");
    sprintf(buffer,"      1     %s    %s    %s", dtostrf(fase_1.CAL_V, 5, 4, strTmp), dtostrf(fase_1.CAL_I, 5, 4, strTmp2), dtostrf(fase_1.CAL_FVI, 5, 3, strTmp3)); 
    fichMuesLog.println(buffer);
    sprintf(buffer,"      2     %s    %s    %s", dtostrf(fase_2.CAL_V, 5, 4, strTmp), dtostrf(fase_2.CAL_I, 5, 4, strTmp2), dtostrf(fase_2.CAL_FVI, 5, 3, strTmp3)); 
    fichMuesLog.println(buffer);
    sprintf(buffer,"      3     %s    %s    %s", dtostrf(fase_3.CAL_V, 5, 4, strTmp), dtostrf(fase_3.CAL_I, 5, 4, strTmp2), dtostrf(fase_3.CAL_FVI, 5, 3, strTmp3)); 
    fichMuesLog.println(buffer);
    fichMuesLog.println("Fecha\tHora\tNr\tFase\tV\tI\tV\tI (80) ...");
    
    fichMuesLog.flush();
  }
}    // Grabar_SD_Cab_Mues()

/*
void Grabar_SD_Lineas(unsigned long tiempo, boolean muestras)
{
  String bufferSD = "";
  
  tft.setCursor(260, 230);    // (y,x)
  tft.setTextSize(1);
  
  if (fichLog)
  {
    tft.print(F("  Grab ..."));
    
    Genera_Lin_SD(&fase_1, &bufferSD);
    fichLog.println(bufferSD);
   
    if (red_Tri)
    {
        Genera_Lin_SD(&fase_2, &bufferSD);
        fichLog.println(bufferSD);
        Genera_Lin_SD(&fase_3, &bufferSD);
        fichLog.println(bufferSD);
    }
        
    fichLog.flush();
    
    if (muestras)
    {
        Genera_Lin_SD_Mues(&fase_1, &bufferSD);
        fichMuesLog.println(bufferSD);
        
        if (red_Tri)
        {
            Genera_Lin_SD_Mues(&fase_2, &bufferSD);
            fichMuesLog.println(bufferSD);
            Genera_Lin_SD_Mues(&fase_3, &bufferSD);
            fichMuesLog.println(bufferSD);
        }
        fichMuesLog.flush();
    }
  }
  else
      tft.print(F("Sin SD ..."));
      
  tft.setTextSize(2);
  
}    // Grabar_SD_Lineas
*/

void Grabar_SD_Valores(unsigned long tiempo)
{
  String bufferSD = "";
  
  tft.setCursor(260, 230);    // (y,x)
  tft.setTextSize(1);
  
  if (fichLog)
  {
    tft.print(F("  Grab ..."));
    
    Genera_Lin_SD(&fase_1, &bufferSD);
    fichLog.println(bufferSD);
   
    if (red_Tri)
    {
        Genera_Lin_SD(&fase_2, &bufferSD);
        fichLog.println(bufferSD);
        Genera_Lin_SD(&fase_3, &bufferSD);
        fichLog.println(bufferSD);
    }
        
    fichLog.flush();
  }
  else
      tft.print(F("Sin SD ..."));
      
  tft.setTextSize(2);
  
}    // Grabar_SD_Valores

void Genera_Lin_SD(struct Fase *fase, String *bufferSD)
{
  *bufferSD = String(strFecHora) + sepCampos;
  *bufferSD += String(numBucles) + sepCampos + "Fase_" + String(fase->numFase) + sepCampos;
  *bufferSD += String(fase->Vef) + sepCampos + String(fase->Ief) + sepCampos;
  *bufferSD += String(fase->Pot_Act) + sepCampos + String(fase->Pot_Reac) + sepCampos + String(fase->Pot_Apa) + sepCampos;
  *bufferSD += String(fase->FP, 4) + sepCampos + String(fase->Ang_Desf) + sepCampos;
  *bufferSD += String(fase->Energia);
}    // Genera_Lin_SD

