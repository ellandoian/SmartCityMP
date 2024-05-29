# 1 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino"
//Antatt forbruk 0.2kWh per km, 400 km rekkevidde. Maks batteri er 80 kWh.

# 4 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino" 2
# 5 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino" 2
# 6 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino" 2

Zumo32U4Motors motors;
Zumo32U4ButtonC buttonC;
Zumo32U4LineSensors lineSensors;
Zumo32U4OLED display;
Zumo32U4Encoders encoder;

byte topSpeed = 200;
byte power, distMultiplier;
unsigned long totalDistance;
float partDisGlobal;

//Avstand kjørt, 1m kjøring er 10km simulert kjøring.
//Etter 255km kjørt simulert, deles totaldistansen opp i et multiplum av 255 og en rest, slik at EEprom kan lagre hele distansen.

float distMeasure() {
  if (partDisGlobal >= 255) {
    distMultiplier += 1;
    partDisGlobal -= 255;
  }
  int currRotLeft = encoder.getCountsAndResetLeft();
  int currRotRight = encoder.getCountsAndResetRight();
  float leftDist = ((((currRotLeft)>0?(currRotLeft):-(currRotLeft))) * 3.1415 * 0.039) / 910;
  float rightDist = ((((currRotRight)>0?(currRotRight):-(currRotRight))) * 3.1415 * 0.039) / 910;
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
  display.print((reinterpret_cast<const __FlashStringHelper *>(
# 51 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino" 3
               (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 51 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino"
               "Power: "
# 51 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino" 3
               ); &__c[0];}))
# 51 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino"
               )));
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
  while (0 < Wire.available()) // loop through all
  {
    int c = Wire.read(); // receive byte as an int
    Serial.println(c); // print the character
    if (c == 5) {
      Charge();
    }
  }
}

//Lader opp batteriet og pauser i 5 sekund

void Charge() {
  unsigned long time = millis();
  distMultiplier = 0;
  partDisGlobal = 0;
  display.clear();
  display.println("CHARGING");
  delay(5000);
  display.clear();
  motors.setSpeeds(50, 50);
  delay(500);
  motors.setSpeeds(0, 0);
}

//Sende distanse kjørt til ESP

void sendDistance() {
  Wire.write(totalDistance);
}

short lineSensorRead() {
  static unsigned short lineSensorVal[5]; // lager en variable med like mange indekser som det er sensorer
  short error = map(lineSensors.readLine(lineSensorVal), 0, 4000, -2000, 2000);
  return error;
}

void lineFollowPID(int pos) { // tar inn posisjonen
  static short prevPos;
  short correction = pos / 4 + 6 * (pos - prevPos); // kilde eksempelkode
  prevPos = pos;
  byte lSpeed = ((topSpeed + correction)<(0)?(0):((topSpeed + correction)>(topSpeed)?(topSpeed):(topSpeed + correction))); // farten på venstre side lik topSpeed + correction
  byte rSpeed = ((topSpeed - correction)<(0)?(0):((topSpeed - correction)>(topSpeed)?(topSpeed):(topSpeed - correction))); // farten på høgre side lik topspeed - correction
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

//Main

void setup() {
  Wire.begin(1);
  Serial.begin(115200);
  Wire.onRequest(sendDistance);
  Wire.onReceive(Receive);
  display.setLayout21x8();
  delay(100);
  EEPROM.write(0, 80);
  EEPROM.write(1, 0);
  EEPROM.write(2, 0);
  power = EEPROM.read(0);
  partDisGlobal = EEPROM.read(1);
  distMultiplier = EEPROM.read(2);
  pidSetup();
}

void loop() {
  lineFollowPID(lineSensorRead());
  partDisGlobal = distMeasure();
  totalDistance = partDisGlobal + (distMultiplier * 255);
  power = batteryDrain(power);
  showBattery();
}
