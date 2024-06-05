#include <Arduino_APDS9960.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

//skal bruke prox sensor til å vite når den skal lese av, trenger kun å lese mens det er en bil under sensoren


// wifi og wifipassord
const char* ssid = "NTNU-IOT";
const char* password = "";

//Broker adresse
const char* mqtt_server = "10.25.18.138";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];

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

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Melding ankommet topic: ");
  Serial.print(topic);
  Serial.print(". Melding: ");
  char charMessage;
  for(int i=0; i<length; i++) {
    charMessage += (char)message[i];
  }
  int intValue = charMessage - '0';
  Serial.println(intValue);
}

void reconnect() {
  client.subscribe("car2Charge");
  // Looper til en kobling er opprettet
  while (!client.connected()) {
    Serial.print("Forsøker å opprette kobling til mqtt...");
    // Attempt to connect
    if (client.connect("ESP32client", "njaal", "3Inshallah4")) {
      Serial.println("connected");
      // Topic som det subscribes til
      client.subscribe("car2Charge");
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

short proxRead() {
  static short proximity;
  if (APDS.proximityAvailable()) proximity = APDS.readProximity();
  return proximity;
}

byte taken() {
  if (proxRead() <= 150) {
    return 1;
  } else return 0;
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
  if (button(1300, true) && millis() - colCalTime >= 100) {  //hvert 100 millisekund tar den en måling,
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

String IDcheck() {
  String ID;
  int* baseColor;
  baseColor = calibrateCol();
  int* curColor;
  curColor = colorRead();
  static int colorCheck[3];
  for (short i; i <= 2; i++) {
    ID += String(colorCheck[i] = map(colorCheck[i] = curColor[i] - baseColor[i], -10, 255, 0, 24));
    ID += ",";
  }
  ID += String(taken());
  return ID;
}

void printOnce() {  //printer kun når det er ny informasjon, og om den lagra informasjonen har hendt de siste 50 nye avlesningene vil det heller ikke bli printet
  static String prevInput = IDcheck();
  static String dataArr[50] = { String(0) };
  static uint32_t dataTime[50] = { millis() };
  static int j;
  static bool io = false;
  j++;
  if (j < 50) {
    j = 0;
  }
  if (prevInput != IDcheck()) {
    String data = IDcheck();
    //Serial.println(IDcheck());
    for (int i; i <= 50; i++) {
      if (data == dataArr[i]) {
        io = true;
        break;
      } else io = false;
    }
    if (io == false) {
      int length = data.length();            // kilde https://www.geeksforgeeks.org/convert-string-char-array-cpp/
      char* sendArr = new char[length + 1];  // -----""-----
      strcpy(sendArr, data.c_str());         // -----""-----
      Serial.println(data);
      client.publish("esp32/output", sendArr);
      dataArr[j] = data;
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
