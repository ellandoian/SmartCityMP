#include <Arduino_APDS9960.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// wifi og wifipassord
const char* ssid = "NTNU-IOT";
const char* password = "";

//Broker adresse
const char* mqtt_server = "10.25.18.138";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
string bomID = "NAVN PÅ BOM"  //Fyll inn med en unik ID for hver bom.

  int pushButton = 25;

void setup() {
  Serial.begin(115200);
  APDS.begin();
  pinMode(pushButton, INPUT);
  Serial.println("start");
  // mqtt settup
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  // Kobler til wifi:
  Serial.println();
  Serial.print("Kobler til: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi kobling opprettet");
  Serial.println("IP addresse: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {  //Funksjon som kalles på når en melding på en abonnert topic kommer inn.
  Serial.print("Melding ankommet topic: ");
  Serial.print(topic);
  Serial.print(". Melding: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/output") {
    Serial.print("Endrer output til: ");
    if (messageTemp == "on") {
      Serial.println("på");
    } else if (messageTemp == "off") {
      Serial.println("av");
    }
  }
}

void reconnect() {  //Denne funksjonen kobler ESPen til MQTT
  client.subscribe("esp32/output");
  // Looper til en kobling er opprettet
  while (!client.connected()) {
    Serial.print("Forsøker å opprette kobling til mqtt...");
    // Attempt to connect
    if (client.connect(bomID, "njaal", "3Inshallah4")) {
      Serial.println("connected");
      // Topic som det subscribes til
      client.subscribe("esp32/output");
    } else {
      Serial.print("mislykket kobling, rc=");
      Serial.print(client.state());
      Serial.println(" Prøver igjen om 5 sekund");
      delay(5000);
    }
  }
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

short proxRead() {  //leser av nærhetsensoren og returnerer det som en short int
  static short proximity;
  if (APDS.proximityAvailable()) proximity = APDS.readProximity();
  return proximity;
}

short carCount(short proxy) {  //teller hvor mange biler som har kjørt forbi bommen, tar inn proximity dataen
  static short cCounter = -1;  //vil autmatisk telle +1 når koden kjøres første gang, verdi på -1 gjør at den starter på 0
  static bool countState = false;
  if (proxy < 200) countState = true;  //veien ligger på runt 220, registerer når noe har kommet til bommen
  if (proxy > 200 && countState) {     //når bilen har kjørt helt gjennom bommen, vil bilen bli telt
    cCounter++;
    countState = false;
  }
  return cCounter;
}

short carCount60s() {  //teller hvor mange biler som har passert de siste 60 sekundene
  static unsigned long carArr[100] = {};
  static short prevCount = carCount(proxRead());
  static short cc60;
  static bool flag = false;
  if (prevCount != carCount(proxRead()) && flag) {  //om det har passert en ny bil telles det opp en verdi
    cc60++;                                         //og tidspunktet lagres
    carArr[cc60 - 1] = millis();
    prevCount = carCount(proxRead());
  } else if (prevCount != carCount(proxRead())) {  //uten flagget vil funksjonen telle en bil mer enn det faktisk er
    flag = true;
    prevCount = carCount(proxRead());
  }

  if (carArr[0] + 60000 <= millis() && carArr[0] != 0) {  //etter at det har gått 60 sekunder i første indeks vil cc60 telle -1
    for (int i = 0; i < cc60; i++) {                      //og alle tidspunkter går ned en indeks
      carArr[i] = carArr[i + 1];
    }
    cc60--;
  }
  return cc60;
}

int* colorRead() {  //leser av fargesensoren og returnerer det som ett array
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
    Serial.printf("count: %d\n", count);
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

String IDcheck() {  //retunerer en komma seperert farge kode med carCount60s på slutten
  String ID;
  int* baseColor;
  baseColor = calibrateCol();
  int* curColor;
  curColor = colorRead();
  static int colorCheck[3];
  for (short i; i <= 2; i++) {  //får verdiene fra bommringen til å stemme overens med verdiene på ladestasjonen
    if (i == 2) {
      curColor[i]++;
    }
    ID += String(colorCheck[i] = map(colorCheck[i] = curColor[i] - baseColor[i], -10, 255, 0, 24));  //tar kalibrerte farge dataen, mapper det til ønsket omerådet
    ID += ",";                                                                                       //konverter til string og komma seperer de
  }
  ID += String(carCount60s());
  return ID;
}

void printOnce() {  //printer kun når det er ny informasjon, og om den lagra informasjonen har hendt de siste 50 nye avlesningene vil det heller ikke bli printet
  static String prevInput = IDcheck();
  static String dataArr[50] = { String(0) };
  static uint32_t dataTime[50] = { millis() };
  static int j;
  static bool io = false;
  if (j < 10) {
    j = 0;
  }
  if (prevInput != IDcheck()) {
    String data = IDcheck();
    for (int i; i <= 10; i++) {  //sjekker om nåværende datapunktet er anderledes fra de 10 siste
      if (data == dataArr[i]) {
        io = true;
        break;
      } else io = false;
    }
    if (!io) {                               //om ny data er anderledes, send data
      int length = data.length();            // kilde https://www.geeksforgeeks.org/convert-string-char-array-cpp/
      char* sendArr = new char[length + 1];  // -----""-----
      strcpy(sendArr, data.c_str());         // -----""-----
      Serial.println(data);
      client.publish("esp32/output", sendArr);
      dataArr[j] = data;
      j++;
    }
    prevInput = IDcheck();
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  printOnce();
}