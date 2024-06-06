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
int send, lastSent;



void setup_wifi() { //Setter opp wifi tilkobling
  delay(10);
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

void callback(char* topic, byte* message, unsigned int length) { //Funksjon som kalles på når en melding på en abonnert topic kommer inn.
  Serial.print("Melding ankommet topic: ");
  Serial.print(topic);
  Serial.print(". Melding: ");
  char courseArray[length+1]={};
  
  for (int i = 0; i < length; i++) {;
    courseArray[i] = (char)message[i];
    courseLength++;
  }
  courseArray[length] = '\0';
  for (int i = 0; i < length; i++) {
    int intValue = courseArray[i] - '0';  // Konverterer elemetene i courseArray til integers
    Serial.print(intValue);
    courseGlobal[i]=courseArray[i];
  }
  Serial.println();
}

void reconnect() { //
  client.subscribe("web2Zumo");
  // Looper til en kobling er opprettet 
  while (!client.connected()) {
    Serial.print("Forsøker å opprette kobling til mqtt...");
    //Prøver å koble seg til
    if (client.connect("ESP32client", "njaal", "3Inshallah4")) {
      Serial.println("connected");
      // Topic som det subscribes til
      client.subscribe("web2Zumo");
    } else { //Om tilkobling mislykkes prøves det igjen etter 5 sekunder.
      Serial.print("mislykket kobling, rc=");
      Serial.print(client.state());
      Serial.println(" Prøver igjen om 5 sekund");
      delay(5000);
    }
  }
}



void setup()
{
  Wire.begin(); //Starter I2C kommunikasjon som master
  Serial.begin(115200);
  Serial.println("start");
  // mqtt settup
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); 
}

byte x = 0;

void loop() {
  Wire.beginTransmission(1); // transmit to device #1
  for (int i=0; i < courseLength; i++){
    Wire.write(courseGlobal[i]);
    }
  Wire.endTransmission();    // stop transmitting
  courseLength = 0;
  Wire.requestFrom(1, 1);
  while(Wire.available() > 0) { 
    int c = Wire.read();
    if ((c > 0) && c != lastSent) { //Sender kun data videre om verdien ikke er 0 eller samme data den nettopp har sendt.
      Serial.println(c);
      send=c;
    }
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (send>0) { //Konverterer meldingen til et char array og publisher på car2Charge topicen.
    lastSent = send;
    char sendString[8];
    itoa(send, sendString, 10);
    Serial.print("Verdi som blir sendt:  ");
    Serial.println(sendString);

    client.publish("car2Charge", sendString);
    send = 0;
  }
}