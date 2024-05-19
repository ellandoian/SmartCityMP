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
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
    }
    else if(messageTemp == "off"){
      Serial.println("off");
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
    int temperature = 42;
    //Konverterer verdien fra int til char array, sender tempstring til gitt topic
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);

    client.publish("esp32/output", tempString);

    int humidity = 1;
    
    // Verdien som sendes MÅ være et char array, vet ikke hva dtostrf() funksjonen gjør, men den MÅ være der
    //Denne funksjonen sender verdien fra humidity, men konvertert til char array
    char humString[8];
    dtostrf(humidity, 1, 2, humString);
    Serial.print("Humidity: ");
    Serial.println(humString);
    client.publish("esp32/output", humString);
  }
}