#include <Wire.h>
#include <Zumo32U4.h>
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4ButtonC buttonC;
Zumo32U4OLED display;
/*
******************************************************************************************************************
***************************************************|----------|***************************************************
***************************************************|LES README|***************************************************
***************************************************|----------|***************************************************
******************************************************************************************************************
*/
static int drip[5];

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
    if (millis() - startTime >= 5500) startFlag = false;
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
  static byte input = 1;

  switch (input) {
    case 1:
      static bool leftFlag, leftFlag2 = false;
      static byte leftCounter = 0;
      static uint32_t leftTime = millis();
      lineFollowPID(lineSensorRead());
      if (lineSensors.readOneSens(drip) >= 900 && leftFlag2 == false) leftFlag, leftFlag2 = true;
      else if (lineSensors.readOneSens(drip) == 0 && leftFlag) {
        leftCounter++;
        leftFlag = false;
      }
      if (lineSensors.readOneSens(drip) >= 900 && leftCounter > 0){
        motors.setSpeeds(-100,150);
        delay(500);
        leftCounter = 0;
        leftFlag2 = false;
      }
      case 2:
        static bool straightFlag = false;
      static byte straightCounter = 0;
      if (straightCounter < 2) {  //fjern if setningen
        lineFollowPID(lineSensorRead());
      }
      if (lineSensors.readOneSens(drip) >= 900) straightFlag = true;
      else if (lineSensors.readOneSens(drip) == 0 && straightFlag) {
        straightCounter++;
        straightFlag = false;
      }
      if (straightCounter >= 2) {
        motors.setSpeeds(0, 0);  //her skal break eller no og "straightCounter = 0;"
      }

    case 3:
      static bool rightFlag = false;
      static uint32_t rightTime = millis();
      if (lineSensors.readOneSens(drip) >= 900) {
        rightTime = millis();
        motors.setSpeeds(150, -100);
        rightFlag = true;
      }
      if (millis() - rightTime >= 500 && rightFlag) {
        //input = 2;  // bytt ut dette med break eller no
        rightFlag = false;
        break;
      } else if (millis() - rightTime >= 500) lineFollowPID(lineSensorRead());
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
  motors.setSpeeds(lSpeed, rSpeed);
}

void loop() {
  Serial.println(lineSensors.readOneSens(drip));
  drivingMain();
}