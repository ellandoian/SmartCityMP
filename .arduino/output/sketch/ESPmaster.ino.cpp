#include <Arduino.h>
#line 1 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\ESPmaster\\ESPmaster.ino"
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

// wifi og wifipassord
const char* ssid = "NTNU-IOT";
const char* password = "";

//Broker adresse
const char* mqtt_server = "10.25.18.138";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
char courseGlobal[]={};
byte courseLength;

int variabel1 = 0;
int variabel2 = 0;



#line 25 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\ESPmaster\\ESPmaster.ino"
void setup_wifi();
#line 45 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\ESPmaster\\ESPmaster.ino"
void callback(char* topic, byte* message, unsigned int length);
#line 70 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\ESPmaster\\ESPmaster.ino"
void reconnect();
#line 91 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\ESPmaster\\ESPmaster.ino"
void setup();
#line 104 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\ESPmaster\\ESPmaster.ino"
void receiveEvent(int howMany);
#line 115 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\ESPmaster\\ESPmaster.ino"
void loop();
#line 25 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\ESPmaster\\ESPmaster.ino"
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
  char courseArray[length+1]={};
  
  /*for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();*/
  
  for (int i = 0; i < length; i++) {;
    courseArray[i] = (char)message[i];
    courseLength++;
  }
  courseArray[length] = '\0';  // Null-terminate the string
  for (int i = 0; i < length; i++) {
    int intValue = courseArray[i] - '0';  // Convert character to integer
    Serial.print(intValue);
    courseGlobal[i]=courseArray[i];
  }
  Serial.println();
}

void reconnect() {
  client.subscribe("web2Zumo");
  // Looper til en kobling er opprettet 
  while (!client.connected()) {
    Serial.print("Forsøker å opprette kobling til mqtt...");
    // Attempt to connect
    if (client.connect("ESP32client", "njaal", "3Inshallah4")) {
      Serial.println("connected");
      // Topic som det subscribes til
      client.subscribe("web2Zumo");
    } else {
      Serial.print("mislykket kobling, rc=");
      Serial.print(client.state());
      Serial.println(" Prøver igjen om 5 sekund");
      delay(5000);
    }
  }
}



void setup()
{
  Wire.begin(); //Starter kommunikasjon som master
  Serial.begin(115200);
  Serial.println("start");
  // mqtt settup
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); 
}

byte x = 0;

void receiveEvent(int howMany)
{
  while(0 < Wire.available()) //x loop through all but the last
  {
    int c = Wire.read(); // receive byte as an integer
    if(c != -1) {
      Serial.print("Hei");         // print the character
  }
  }
}

void loop() {
  int send;
  Wire.beginTransmission(1); // transmit to device #1
  for (int i=0; i < courseLength; i++){
    Wire.write(courseGlobal[i]);
    }
  Wire.endTransmission();    // stop transmitting
  courseLength = 0;
  Wire.requestFrom(1, 1);
  while(Wire.available() > 0) {
    int c = Wire.read();
    if (c > 0) {
      Serial.println(c);
      send=c;
    }
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();

  if (send>0) {
    char sendString[8];
    dtostrf(send, 1, 2, sendString);
    Serial.print("Verdi som blir sendt:  ");
    Serial.println(sendString);

    client.publish("car2Charge", sendString);
  }
  /*if (now - lastMsg > 5000) {
    lastMsg = now;
    int val1 = 8;
    //Konverterer verdien fra i
    int til char array, sender tempstring til gitt topic
    char Value_1[8];
    dtostrf(val1, 1, 2, Value_1);
    Serial.print("Verdi 1: ");
    Serial.println(Value_1);

    client.publish("esp32/output", Value_1);*/
}
