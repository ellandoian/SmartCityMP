//Antatt forbruk 0.2kWh per km, 400 km rekkevidde. Maks batteri er 80 kWh.

#include <Wire.h>
#include <Zumo32U4.h>
#include <EEPROM.h>

Zumo32U4Motors motors;
Zumo32U4ButtonC buttonC;
Zumo32U4LineSensors lineSensors;
Zumo32U4OLED display;
Zumo32U4Encoders encoder;

byte topSpeed = 200;
byte power, distMultiplier, input;
unsigned long totalDistance;
float partDisGlobal;
int courseArray[]={};
bool sendChargeDist = false;

//Avstand kjørt, 1m kjøring er 10km simulert kjøring.
//Etter 255km kjørt simulert, deles totaldistansen opp i et multiplum av 255 og en rest, slik at EEprom kan lagre hele distansen.

float distMeasure() {
  if (partDisGlobal >= 255) {
    distMultiplier += 1;
    partDisGlobal -= 255;
  }
  int currRotLeft = encoder.getCountsAndResetLeft();
  int currRotRight = encoder.getCountsAndResetRight();
  float leftDist = ((abs(currRotLeft)) * 3.1415 * 0.039) / 910;
  float rightDist = ((abs(currRotRight)) * 3.1415 * 0.039) / 910;
  float distPart = partDisGlobal + (10 * (leftDist + rightDist) / 2);
  EEPROM.write(1, partDisGlobal);
  EEPROM.write(2, distMultiplier);
  return distPart;
}

//Genererer batterinivået, mellom 0 og 80

int batteryDrain(byte battery) {
  battery = 80 - (totalDistance / 5);
  if (battery < 0) {
    battery = 0;
  }
  EEPROM.write(0, battery);
  return battery;
}

//Viser batteriet og avstand kjørt på displayet

void showBattery() {
  display.gotoXY(0, 0);
  display.print(F("Power:  "));
  display.gotoXY(0, 1);
  display.print(power);
  display.gotoXY(0, 3);
  display.println("Distance drove; ");
  display.gotoXY(0, 4);
  display.print(totalDistance);
  display.print("km ");
}

//Tolker meldinger fra ESP

void Receive(int howMany) {
  byte i = 0;
  while (0 < Wire.available())  // loop through all
  {
    byte receivedByte = Wire.read();
    courseArray[i] = receivedByte - '0'; // Convert from ASCII to integer
    Serial.println(courseArray[i]);
    i++;
  }
}

//Lader opp batteriet og pauser i 5 sekund

void Charge() {
  display.clear();
  display.println("CHARGING");
  sendChargeDist = true; //Flagg som gjør at bilen sender distansen til ESP32
  motors.setSpeeds(0, 0);
  delay(1000); //Skal bort senere
}

//Sende distanse kjørt til ESP, kjøres når bilen lader

void sendDistance() {
  if (sendChargeDist == true) {
    //Serial.println(totalDistance); //Skal bort senere
    int partDis = static_cast<int>(partDisGlobal);
    Wire.write(partDis);
    Wire.write(distMultiplier);
    sendChargeDist = false;
    distMultiplier = 0;
    partDisGlobal = 0; //Resetter avstanden etter den er sendt
  }
}

short lineSensorRead() {
  static unsigned short lineSensorVal[5];  // lager en variable med like mange indekser som det er sensorer
  short error = map(lineSensors.readLine(lineSensorVal), 0, 4000, -2000, 2000);
  return error;
}

void lineFollowPID(int pos) {  // tar inn posisjonen
  static short prevPos;
  short correction = pos / 4 + 6 * (pos - prevPos);  // kilde eksempelkode
  prevPos = pos;
  byte lSpeed = constrain(topSpeed + correction, 0, topSpeed);  // farten på venstre side lik topSpeed + correction
  byte rSpeed = constrain(topSpeed - correction, 0, topSpeed);  // farten på høgre side lik topspeed - correction
                                                                // setter slik at verdien vil alltids være mellom 200 og 0, vil forhindre for høye hastigheter, men viktigs
                                                                // hindrer at det vil fort gå fra positiv hastighet til negativ hastighet som kan skade motorene.
  Serial.println(lSpeed);
  motors.setSpeeds(lSpeed, rSpeed);
}

void pidSetup() {
  lineSensors.initFiveSensors();
  bool startFlag = true;
  buttonC.waitForPress();
  motors.setSpeeds(100, -100);
  uint32_t startTime = millis();
  while (startFlag) {
    lineSensors.calibrate();
    if (millis() - startTime >= 4000) startFlag = false;
  }
  motors.setSpeeds(0, 0);
}

void drivingMain() {
  int filler[3] = { 3, 2, 1 };
  switch (input) {
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
        Charge();

        break;
    default:
      lineFollowPID(lineSensorRead());
  }
}

//Main

void setup() {
  Wire.begin(1);
  Serial.begin(115200);
  Wire.onRequest(sendDistance);
  Wire.onReceive(Receive);
  display.setLayout21x8();
  EEPROM.write(0, 80);
  EEPROM.write(1, 0);
  EEPROM.write(2, 1);
  power = EEPROM.read(0);
  partDisGlobal = EEPROM.read(1);
  distMultiplier = EEPROM.read(2);
  //pidSetup();
}

void loop() {
  motors.setSpeeds(100,100); //skal bort
  static long tid; //skal bort
  partDisGlobal = distMeasure();
  totalDistance = partDisGlobal + (distMultiplier * 255);
  power = batteryDrain(power);
  showBattery();
  if (millis()-tid >= 5000) { //if-setningen skal bort
    Charge();
    tid = millis();
  }
  int size = sizeof(courseArray);
  Serial.print("Size: ");
  Serial.print(size);
  Serial.print("  ");
  Serial.println(courseArray[2]);
}