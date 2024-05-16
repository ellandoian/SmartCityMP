#include <Arduino_APDS9960.h>
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

short carCount(short proxy) {  //teller hvor mange biler som har kjørt forbi bommen, tar inn proximity dataen
  static short cCounter = -1;  //vil autmatisk telle +1 når koden kjøres første gang, verdi på -1 gjør at den starter på 0
  static bool countState = false;

  if (proxy < 215) countState = true;    //veien ligger på runt 220, registerer når noe har kommet til bommen
  else if (proxy > 215 && countState) {  //når bilen har kjørt helt gjennom bommen, vil bilen bli telt
    cCounter++;
    countState = false;
  }
  return cCounter;
}

short carCount60s() {
  static unsigned long carArr[100] = {};
  static short prevCount = carCount(proxRead());
  static short cc60;
  static bool flag = false;
  if (prevCount != carCount(proxRead()) && flag) {
    cc60++;
    carArr[cc60 - 1] = millis();
    prevCount = carCount(proxRead());
  } else if (prevCount != carCount(proxRead())) {
    flag = true;
    prevCount = carCount(proxRead());
  }

  if (carArr[0] + 60000 <= millis() && carArr[0] != 0) {
    for (int i = 0; i < cc60; i++) {
      carArr[i] = carArr[i + 1];
    }
    cc60--;
  }
  return cc60;
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

String IDcheck() {  //funksjonen som skal identisere fargene
                    //denne funksjonen bygger på gammelt design, men tanken er å ta inn data fra kalibreringa ta den dataen minus nåværende
                    //curColor for å se etter store utslag
  String ID;
  int* baseColor;
  baseColor = calibrateCol();
  int* curColor;
  curColor = colorRead();
  static int colorCheck[3];
  for (short i; i <= 2; i++) {
    ID += String(colorCheck[i] = map(colorCheck[i] = curColor[i] - baseColor[i], -10, 255, 0, 20));
    ID += ",";
  }
  //Serial.println(ID);
  ID += String(carCount60s());
  return ID;
}

void printOnce() {
  static String prevInput = IDcheck();
  if (prevInput != IDcheck()) {
    Serial.println(IDcheck());
    prevInput = IDcheck();
  }
}

void loop() {
  printOnce();
}