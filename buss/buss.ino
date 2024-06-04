#include <Zumo32U4.h>
/*
******************************************************************************************************************
***************************************************|----------|***************************************************
***************************************************|LES README|***************************************************
***************************************************|----------|***************************************************
******************************************************************************************************************
*/
Zumo32U4Motors motors;
Zumo32U4ButtonC buttonC;
Zumo32U4LineSensors lineSensors;

bool pidFlag = true;  //for å kunne tvinge PID av
byte input;
int courseArray[8] = { 1, 1, 1, 1, 3, 3, 3, 3 };
byte courseArrlength = 0;
static int drip[5];  //trengs for å kunne lese av spesfik sensor

int rightSpeed = 200;
int leftSpeed = 200;
int previousError;
float output;
double integral;
double derivative;
unsigned int lineSensorValues[5];

void lineFollowPID() {  // tar inn posisjonen
  if (pidFlag) {
    int posisjon = lineSensors.readLine(lineSensorValues);
    int error = (posisjon - 2000) / 5;
    integral += error;
    derivative = error - previousError;
    previousError = error;
    output = error + 0.0001 * integral + 4 * derivative;
    motors.setSpeeds(leftSpeed + output, rightSpeed - output);
  }
}
void drivingMain() {
  static byte turnCount = 0;
  switch (input) {
    case 1:  //høyere
      static bool rightFlag = false;
      static uint32_t rightTime = millis();
      if (lineSensors.readOneSens(drip) >= 700) {  //Om bilen har kommet til et kryss vil den svinge til høyere
        rightTime = millis();
        motors.setSpeeds(150, -100);
        rightFlag = true;
      }
      if (millis() - rightTime >= 350 && rightFlag) {  //om bilen har fullført svingen hopper bilen til neste case
        input = 4;
        rightFlag = false;
        break;
      } else if (millis() - rightTime >= 350) lineFollowPID();  //kjører PID om ingen sving
      break;
    case 3:  //venstre
      static bool leftFlag = false;
      static bool leftFlag2 = true;
      static byte leftCounter = 0;
      static uint32_t leftTime = millis();
      lineFollowPID();
      if (lineSensors.readOneSens(drip) >= 700) {  //merker at den rører en linje og setter av et flag
        leftFlag = true;
      } else if (lineSensors.readOneSens(drip) < 100 && leftFlag) {  //når bilen har gått av linjen flippes flaget tilbake og counter går +1
        leftCounter++;
        leftFlag = false;
      }

      if (lineSensors.readOneSens(drip) >= 700 && leftCounter == 1) {  //når bilen kommer til en linje etter å ha pasert en vil den svinge til venstre
        motors.setSpeeds(-100, 100);
        leftTime = millis();
        leftFlag2 = false;
        pidFlag = false;  //skrur av PID kjøring
        leftCounter++;
      }
      if (leftFlag2 == false && millis() - leftTime >= 500) {  //avsluttersvingen og skrur på PID kjøring
        leftFlag2 = true;
        pidFlag = true;
      }
      if (leftCounter >= 4) {  //tar å resetter counter og fullfører denne svingen etter bilen er ute av kryset
        leftCounter = 0;
        input = 4;
        break;
      }
      break;
    case 4:  //iterer
      static bool switcher = true;
      static uint32_t switcherTime = millis();
      lineFollowPID();
      if (turnCount >= 7) {
        turnCount = 0;
        input = courseArray[turnCount];
        break;
      } else {
        if (switcher) {
          switcherTime = millis();
          switcher = false;
        }
        if (millis() - switcherTime >= 300) {
          turnCount++;
          switcher = true;
          input = courseArray[turnCount];
          break;
        }
      }
      break;
    default:
      input = courseArray[turnCount];
      break;
  }
}

void setup() {
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

void loop() {
  drivingMain();
}
