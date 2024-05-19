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
int value = 0;

int variabel1 = 0;
int variabel2 = 0;

void setup() {
  Serial.begin(115200);
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
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/output") {
    Serial.print("Endrer output til: ");
    if(messageTemp == "on"){
      Serial.println("på");
    }
    else if(messageTemp == "off"){
      Serial.println("av");
    }
  }
}

void reconnect() {
  client.subscribe("esp32/output");
  // Looper til en kobling er opprettet 
  while (!client.connected()) {
    Serial.print("Forsøker å opprette kobling til mqtt...");
    // Attempt to connect
    if (client.connect("Quagmire_publisher", "njaal", "3Inshallah4")) {
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

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    int val1 = 80085;
    //Konverterer verdien fra int til char array, sender tempstring til gitt topic
    char Value_1[8];
    dtostrf(val1, 1, 2, Value_1);
    Serial.print("Verdi 1: ");
    Serial.println(Value_1);

    client.publish("esp32/output", Value_1);

    int val2 = 69;
    
    // Verdien som sendes MÅ være et char array, vet ikke hva dtostrf() funksjonen gjør, men den MÅ være der
    char Value_2[8];
    dtostrf(val2, 1, 2, Value_2);
    Serial.print("Verdi 2: ");
    Serial.println(Value_2);
    client.publish("esp32/output", Value_2);
  }
}