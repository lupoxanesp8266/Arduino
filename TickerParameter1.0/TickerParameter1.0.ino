/*
  Passing paramters to Ticker callbacks

  Apart from void(void) functions, the Ticker library supports
  functions taking one argument. This argument's size has to be less or
  equal to 4 bytes (so char, short, int, float, void*, char* types will do).

  This sample runs two tickers that both call one callback function,
  but with different arguments.

  An LED connected to GPIO1 will be pulsing. Use a built-in LED on ESP-01
  or connect an external one to TXD on other boards.
*/

#include <Ticker.h>
float v = 3.27;//En voltios
float r1 = 44000;//En ohmios
float r2 = 2510;//En ohmios
float vm = v * (r1 + r2) / r2; //En voltios
float vmax = 0;
float vmin = 0;
float cons = vm / v;
Ticker readanalogpin;
int count = 0;

void read_analog() {
  float tension = analogRead(A0);
  tension = v * cons * tension / 1024.0;
  tension = tension - 16.538;
  Serial.println(tension);
  count ++;
  /*if (count == 40) {
    readanalogpin.detach();
  }*/
}

void setup() {
  Serial.begin(115200);
  pinMode(D3,OUTPUT);
  // every 125 us, call read_analog
  readanalogpin.attach_ms(1/10, read_analog);
}

void loop() {
  for(int i = 0; i <= 255; i++){
    analogWrite(D3,i);
    delay(1);
  }
    for(int i = 255; i >= 0; i--){
    analogWrite(D3,i);
    delay(1);
  }
}
