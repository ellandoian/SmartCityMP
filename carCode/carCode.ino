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
  motors.setSpeeds(130, -130);
  uint32_t startTime = millis();
  while (startFlag) {
    lineSensors.calibrate();
    if (millis() - startTime >= 4500) startFlag = false;
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
      static bool leftFlag3 = true;
      static byte leftCounter = 0;
      static uint32_t leftTime = millis();
      if (leftFlag3) {
        lineFollowPID(lineSensorRead());
        Serial.println("pid");
      }
      if (lineSensors.readOneSens(drip) >= 900 && leftFlag2 == false && leftFlag3) {
        leftFlag = true;
        leftFlag2 = true;
        Serial.println("first flag");
      } else if (lineSensors.readOneSens(drip) < 100 && leftFlag) {
        leftCounter++;
        Serial.print("leftCounter:");
        Serial.println(leftCounter);
        leftFlag = false;
      }

      if (lineSensors.readOneSens(drip) >= 900 && leftCounter > 0) {
        motors.setSpeeds(-150, 200);
        Serial.println("truning");
        leftTime = millis();
        leftCounter = 0;
        leftFlag2 = false;
        leftFlag3 = false;
      }
      if (leftFlag3 == false) {
        Serial.println(millis() - leftTime);
      }
      if (leftFlag3 == false && millis() - leftTime >= 1000) {
        Serial.println("turn Finish");
        motors.setSpeeds(0, 0);
        delay(2000000);
        leftFlag3 = true;
      }
    case 2:
      static bool straightFlag = false;
      static byte straightCounter = 0;
      if (straightCounter < 2) {  //fjern if setningen
        lineFollowPID(lineSensorRead());
      }
      if (lineSensors.readOneSens(drip) >= 900) straightFlag = true;  //merker at den har kommet på en svart linje på venstre side av bilen
      else if (lineSensors.readOneSens(drip) == 0 && straightFlag) {  //teller + 1 etter bilen har pasert linja
        straightCounter++;                                            //
        straightFlag = false;                                         //
      }
      if (straightCounter >= 2) {  //om den har pasert to linjer går den videre til neste steg
        motors.setSpeeds(0, 0);    //her skal break eller no og "straightCounter = 0;"
      }

    case 3:
      static bool rightFlag = false;
      static uint32_t rightTime = millis();
      if (lineSensors.readOneSens(drip) >= 900) {  //Om bilen har kommet til et kryss vil den svinge til høyere
        rightTime = millis();
        motors.setSpeeds(150, -100);
        rightFlag = true;
      }
      if (millis() - rightTime >= 500 && rightFlag) {  //om bilen har fullført svingen hopper bilen til neste case
        //input = 2;  // bytt ut dette med break eller no
        rightFlag = false;
        break;
      } else if (millis() - rightTime >= 500) lineFollowPID(lineSensorRead());  //kjører PID om ingen sving
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
  //Serial.println(lineSensors.readOneSens(drip));
  drivingMain();
}