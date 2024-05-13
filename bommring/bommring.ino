#include <Arduino_APDS9960.h>
int potPin = 27;
int pushButton = 25;

void setup() {
  Serial.begin(115200);
  APDS.begin();
  pinMode(pushButton, INPUT);
}

//kilde, ellandoian/filedump
bool button(int trueTime, bool pulldown) {
  //trueTime is how long you want the button to return "true", input "true" if using a pulldown system or "false" if pullup
  //"pushButton" is the physical button, change name accordingly
  static bool val, buttonVar, lastButtonState = false;
  static uint32_t timer;
  if (digitalRead(pushButton) == pulldown) {
    buttonVar = true;  // setter buttonVar til true mens knappen er klikket ned
    timer = millis();
  }
  if ((digitalRead(pushButton) != pulldown) && (buttonVar == true))  // vil kjøre når knappen slippes og endrer retur variablen
  {
    val = true;
    if (millis() - timer > trueTime) {
      val = false;
      buttonVar = false;
    }
  }
  return val;
}

short proxRead() {
  static short proximity;
  if (APDS.proximityAvailable()) proximity = APDS.readProximity();
  return proximity;
}

short carCount(int proxy) {    //teller hvor mange biler som har kjørt forbi bommen, tar inn proximity dataen
  static short cCounter = -1;  //vil autmatisk telle +1 når koden kjøres første gang, verdi på -1 gjør at den starter på 0
  static bool countState = false;

  if (proxy < 235) countState = true;    //veien ligger på runt 242, registerer når noe har kommet til bommen
  else if (proxy > 235 && countState) {  //når bilen har kjørt helt gjennom bommen, vil bilen bli telt
    cCounter++;
    countState = false;
  }
  return cCounter;
}

int* colorRead() {
  static int rgb[3];
  while (!APDS.colorAvailable()) {
    delay(5);
  }
  APDS.readColor(rgb[0], rgb[1], rgb[2]);
  return rgb;
}

int* calibrateCol() {  //tar 10 målinger over 1,2 sekunder og finner gjennomsnittet
  static uint32_t colCalTime = millis();
  static bool ccFlag = false;
  static short count;
  static int base[3], prevBase[3];
  if (button(1200, true) && millis() - colCalTime >= 100) {  //hvert 100 millisekund tar den en måling,
    int* read;
    read = colorRead();
    for (short i; i <= 2; i++) {
      base[i] = base[i] + read[i];
    }
    count++;
    colCalTime = millis();
  }
  if (count == 10) {  // etter 10 målinger vil gjennomsnittet bli lagret
    for (short i; i <= 2; i++) {
      base[i] = (base[i] - prevBase[i]) / 10;
      prevBase[i] = base[i];
    }
    count = 0;
  }
  return base;
}

void IDcheck() {  //funksjonen som skal identisere fargene
                  //denne funksjonen bygger på gammelt design, men tanken er å ta inn data fra kalibreringa ta den dataen minus nåværende
                  //curColor for å se etter store utslag
  static uint32_t testTime = millis();
  int* baseColor;
  baseColor = calibrateCol();
  int* curColor;
  curColor = colorRead();
  static int colorCheck[3];
  for (short i; i <= 2; i++) {
    colorCheck[i] = curColor[i] - baseColor[i];
  }
  if (millis() - testTime > 1000) {
    Serial.print(colorCheck[0]);
    Serial.print(colorCheck[1]);
    Serial.println(colorCheck[2]);
    testTime = millis();
  }
}

void display() {
  /*int* colorDis;           //om mann skal kalle på farge detektoren, gjør det slikt
  colorDis = colorRead();  //------''------
  Serial.print("red: ");
  Serial.print(colorDis[0]);
  Serial.print(" ");
  Serial.print("green: ");
  Serial.print(colorDis[1]);
  Serial.print(" ");
  Serial.print("blue: ");
  Serial.print(colorDis[2]);
  Serial.print(" ");*/
  /*Serial.print("prox: ");
  Serial.println(proxRead());
  Serial.print("count:");
  Serial.println(carCount(proxRead()));*/
  Serial.println();
}

void loop() {
  //static int potVal;
  //potVal = analogRead(potPin);  //alt med "pot" i seg er bare for testing, skal fjernes fra ferdig produkt
  IDcheck();
  //if (potVal < 900) display();  //kan starte og stoppe vising av målinger med potmetere (gadd ikke å lage bryter)
  //if (potVal > 2000) IDcheck();
}