/*
  Example Timer0 Interrupt
  Flash LED every second
*/

#define ledPin D8
#define timer0_preload 40161290

#define my_delay 2 // Set interrupt time in secs. Value = 2 x Number of Seconds, so 10-Secs = 10 x 2 = 20


volatile int toggle;

void inline handler (void) {
  if (analogRead(A0) <= 255) {
    toggle = (toggle == 1) ? 0 : 1;
    digitalWrite(ledPin, toggle);
  }
  timer0_write(ESP.getCycleCount() + timer0_preload * my_delay); //
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D6, INPUT_PULLUP);
  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(handler);
  timer0_write(ESP.getCycleCount() + timer0_preload * my_delay);
  interrupts();
}

void loop() {
  Serial.println(ESP.getCycleCount());
  /*for (int i = 0; i <= 255; i++) {
    analogWrite(D7, i);
    delay(5);
  }
  for (int i = 255; i >= 0; i--) {
    analogWrite(D7, i);
    delay(5);
  }*/
}
