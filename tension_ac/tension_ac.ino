float v = 3.27;//En voltios
float r1 = 44000;//En ohmios
float r2 = 2510;//En ohmios
float vm = v * (r1 + r2) / r2; //En voltios
float vmax = 0;
float vmin = 0;
float cons = vm / v;
int i = 0;
void setup() {
  Serial.begin(115200);
  Serial.println("\n/**** V MAXIMA ****/");
  Serial.print("/**** ");
  Serial.print(vm);
  Serial.println(" v ****/");
}

void loop() {
  float a = analogRead(A0);
  a = v * cons * a / 1024.0;
  if (i == 80) {
    vmax = a;
    i = 0;
  }
  if (a > vmax) {
    vmax = a;
  }
  if (a < vmin) {
    vmin = a;
  }
  float b = (vmax + vmin) / 2;
  a = a - b;
  //vmax = vmax - b;
  Serial.print(a);
  Serial.print(",");
  Serial.print(vmax);
  Serial.print(",");
  Serial.println(vmax / sqrt(2));
  /*Serial.print("Tension max: ");
    Serial.print(vmax);
    Serial.print(" --- ");
    Serial.print("Vrms: ");
    Serial.print(vmax/sqrt(2));
    Serial.print(" --- ");
    Serial.print("V: ");
    Serial.println(a);*/
  i++;
  delayMicroseconds(250);

}
