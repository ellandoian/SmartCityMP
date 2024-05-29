#include <Wire.h>
#include <Zumo32U4.h>
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4ButtonC buttonC;
Zumo32U4OLED display;

byte topSpeed = 200;

void setup() {
  bool startFlag = true;
  Serial.begin(9600);
  lineSensors.initFiveSensors();
  buttonC.waitForPress();
  motors.setSpeeds(100, -100);
  uint32_t startTime = millis();
  while (startFlag) {
    lineSensors.calibrate();
    if (millis() - startTime >= 4000) startFlag = false;
  }
  motors.setSpeeds(0, 0);
}

short lineSensorRead() {
  static unsigned short lineSensorVal[5];  // lager en variable med like mange indekser som det er sensorer
  short error = map(lineSensors.readLine(lineSensorVal), 0, 4000, -2000, 2000);
  return error;
}

void drivingMain() {
  byte filler[3] = { 3, 2, 1 };
  static byte input = 0;
  switch (input) {
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;

    default:
      lineFollowPID(lineSensorRead());
  }
}

void lineFollowPID(int pos) {  // tar inn posisjonen
  static short prevPos;
  short correction = pos / 4 + 6 * (pos - prevPos);  // kilde eksempelkode
  prevPos = pos;
  byte lSpeed = constrain(topSpeed + correction, 0, topSpeed);  // farten på venstre side lik topSpeed + correction
  byte rSpeed = constrain(topSpeed - correction, 0, topSpeed);  // farten på høgre side lik topspeed - correction
                                                                 // setter slik at verdien vil alltids være mellom 200 og 0, vil forhindre for høye hastigheter, men viktigs
                                                                 // hindrer at det vil fort gå fra positiv hastighet til negativ hastighet som kan skade motorene.
  //Serial.println(lSpeed);
  motors.setSpeeds(lSpeed, rSpeed);
}

/*int test(){
  static unsigned short lineSensorValsia[5];  // lager en variable med like mange indekser som det er sensorer
  int swagger = lineSensors.readOneSens(lineSensorValsia[5]);
  return swagger;
}*/

void loop() {
  static int drip[5];
  Serial.println(lineSensors.readOneSens(drip));
  lineFollowPID(lineSensorRead());
 // Serial.println(test());
}