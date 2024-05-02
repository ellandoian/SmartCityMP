#include <Arduino_APDS9960.h>
int potPin = 27;
int pushButton = 25;

void setup() {
  Serial.begin(115200);
  APDS.begin();
  pinMode(pushButton, INPUT);
}

//kilde, ellandoian/filedump
bool button(int trueTime)  //trueTime is how long you want the button to return TRUE
//"pushButton" is the physical button, change name accordingly
{
  static bool val, buttonVar, lastButtonState = false;
  static uint32_t timer;
  if (digitalRead(pushButton) == HIGH) {
    buttonVar = true;  // setter buttonVar til true mens knappen er klikket ned
    timer = millis();
  }
  if ((digitalRead(pushButton) == LOW) && (buttonVar == true))  // vil kjøre når knappen slippes og endrer retur variablen
  {
    val = true;
    if (millis() - timer > trueTime) val, buttonVar = false;
  }
  return val;
}


int carCount(int proxy) {    //teller hvor mange biler som har kjørt forbi bommen, tar inn proximity dataen
  static int cCounter = -1;  //vil autmatisk telle +1 når koden kjøres første gang, verdi på -1 gjør at den starter på 0
  static bool countState = false;

  if (proxy < 235) countState = true;    //veien ligger på runt 242, redisterer når noe har kommet til bommen
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

int proxRead() {
  static int proximity;
  if (APDS.proximityAvailable()) proximity = APDS.readProximity();
  return proximity;
}
/*int* calibrateCol() {
  int* base;
  if (button(1000)){
//ta ett par målinger og gjennomsnitt eller no for å finne grei tilpassning av grunn fargen, siden den
//pleier å endre litt på veridene de måler fra måling til måling tror jeg gjennomsnitt er smart
  }
}*/ //ikke ferdig skal brukes til å finne fargen som leses når ingen biler er i nærrheten

void IDcheck(int baseColor[]) {  //funksjonen som skal identisere fargene
//denne funksjonen bygger på gammelt design, men tanken er å ta inn data fra kalibreringa ta den dataen minus nåværende 
//vurColor for å se etter store utslag
  int* curColor;
  curColor = colorRead();
  static int colorCheck[3];
  for (int i; i <= 2; i++) {
    colorCheck[i] = baseColor[i] - curColor[i];
  }
  Serial.print(colorCheck[0]);
  Serial.print(colorCheck[1]);
  Serial.println(colorCheck[2]);
}

void display() {
  int* colorDis;           //om mann skal kalle på farge detektoren, gjør det slikt
  colorDis = colorRead();  //------''------
  Serial.print("red: ");
  Serial.print(colorDis[0]);
  Serial.print(" ");
  Serial.print("green: ");
  Serial.print(colorDis[1]);
  Serial.print(" ");
  Serial.print("blue: ");
  Serial.print(colorDis[2]);
  Serial.print(" ");
  /*Serial.print("prox: ");
  Serial.println(proxRead());
  Serial.print("count:");
  Serial.println(carCount(proxRead()));*/
  Serial.println();
}

void loop() {
  //colorRead();
  //proxRead();
  static int potVal;
  potVal = analogRead(potPin);  //alt med "pot" i seg er bare for testing, skal fjernes fra ferdig produkt
  if (potVal < 900) display();  //kan starte og stoppe vising av målinger med potmetere (gadd ikke å lage bryter)
  if (potVal > 2000) IDcheck();
  //Serial.println(potVal);
  delay(100);
}