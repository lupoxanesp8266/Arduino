/*
 * Copyright (c) 2015, circuits4you.com
 * All rights reserved.
/* Generates PWM on Internal LED Pin GPIO 2 of ESP8266*/

#include <ESP8266WiFi.h>
#define LED D2


//=======================================================================
//                    Power on setup
//=======================================================================
void setup() {

  pinMode(LED,OUTPUT);
}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  for(int i= 0; i<= 255; i++){
    analogWrite(LED,i);
    delay(200);
  }
  for(int i= 255; i>= 0; i--){
    analogWrite(LED,i);
    delay(200);
  }
}
//=======================================================================
