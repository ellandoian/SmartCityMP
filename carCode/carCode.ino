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
bool pidFlag = true;
byte topSpeed = 150;

void setup() {
  bool startFlag = true;
  Serial.begin(9600);
  lineSensors.initFiveSensors();
  buttonC.waitForPress();
  motors.setSpeeds(100, -100);
  Serial.println("calibrating");
  uint32_t startTime = millis();
  while (startFlag) {
    lineSensors.calibrate();
    if (millis() - startTime >= 3000) startFlag = false;
  }
  motors.setSpeeds(0, 0);
  Serial.println("start");
}

short lineSensorRead() {
  static unsigned short lineSensorVal[5];  // lager en variable med like mange indekser som det er sensorer
  short error = map(lineSensors.readLine(lineSensorVal), 0, 4000, -2000, 2000);
  return error;
}

void drivingMain() {
  int filler[6] = { 2, 1, 1, 1, 3, 3 };
  static byte input = 1;


  switch (input) {
    case 1:  //svinger til venstre
      static bool leftFlag = false;
      static bool leftFlag2 = true;
      static byte leftCounter = 0;
      static uint32_t leftTime = millis();
      lineFollowPID(lineSensorRead());
      if (lineSensors.readOneSens(drip) >= 900) {  //merker at den rører en linje og setter av et flag
        leftFlag = true;
      } else if (lineSensors.readOneSens(drip) < 100 && leftFlag) {  //når bilen har gått av linjen flippes flaget tilbake og counter går +1
        leftCounter++;
        Serial.println(leftCounter);
        leftFlag = false;
      }

      if (lineSensors.readOneSens(drip) >= 800 && leftCounter == 1) {  //når bilen kommer til en linje etter å ha pasert en vil den svinge til venstre
        motors.setSpeeds(-100, 100);
        leftTime = millis();
        leftFlag2 = false;
        pidFlag = false;  //skrur av PID kjøring
        leftCounter++;
      }
      if (leftFlag2 == false && millis() - leftTime >= 500) {  //avsluttersvingen og skrur på PID kjøring
        leftFlag2 = true;
        Serial.println("turn Complete");
        pidFlag = true;
      }
      if (leftCounter >= 4) {  //tar å resetter counter og fullfører denne svingen etter bilen er ute av kryset
        leftCounter = 0;
        //input = 0;
        break;
      }
      break;
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
        straightCounter = 0;
        input = 4;
        break;
      }
      break;
    case 3:
      static bool rightFlag = false;
      static uint32_t rightTime = millis();
      if (lineSensors.readOneSens(drip) >= 900) {  //Om bilen har kommet til et kryss vil den svinge til høyere
        rightTime = millis();
        motors.setSpeeds(150, -100);
        Serial.println("truning Right");
        rightFlag = true;
      }
      if (millis() - rightTime >= 500 && rightFlag) {  //om bilen har fullført svingen hopper bilen til neste case
        input = 4;
        rightFlag = false;
        break;
      } else if (millis() - rightTime >= 500) lineFollowPID(lineSensorRead());  //kjører PID om ingen sving
      break;
    case 4:
      delay(1000);
      motors.setSpeeds(0, 0);
      /*static bool switcher = true;
      static uint32_t switcherTime = millis();
      lineFollowPID(lineSensorRead());
      if (swithcer) {
        switcherTime = millis();
        switcher = false;
      }
      if(millis()-switcherTime >= 2000){
        
      }*/
  }
}

void lineFollowPID(int pos) {  // tar inn posisjonen
  static short prevPos;
  if (pidFlag) {
    short correction = pos / 4 + 6 * (pos - prevPos);  // kilde eksempelkode
    prevPos = pos;
    byte lSpeed = constrain(topSpeed + correction, 0, topSpeed);  // farten på venstre side lik topSpeed + correction
    byte rSpeed = constrain(topSpeed - correction, 0, topSpeed);  // farten på høgre side lik topspeed - correction
                                                                  // setter slik at verdien vil alltids være mellom 200 og 0, vil forhindre for høye hastigheter, men viktigs
                                                                  // hindrer at det vil fort gå fra positiv hastighet til negativ hastighet som kan skade motorene.
    motors.setSpeeds(lSpeed, rSpeed);
  }
}

void loop() {
  drivingMain();
}