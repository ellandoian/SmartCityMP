//Antatt forbruk 0.2kWh per km, 400 km rekkevidde. Maks batteri er 80 kWh.

#include <Wire.h>
#include <Zumo32U4.h>
#include <EEPROM.h>
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
Zumo32U4OLED display;
Zumo32U4Encoders encoder;

bool pidFlag = true;  //for å kunne tvinge PID av
byte power, distMultiplier, input;
unsigned long totalDistance;
float partDisGlobal;
int courseArray[30] = {};
byte courseArrlength = 0;
bool sendChargeDist = false;
static int drip[5];  //trengs for å kunne lese av spesfik sensor

int rightSpeed = 200;
int leftSpeed = 200;
int previousError;
float output;
double integral;
double derivative;
unsigned int lineSensorValues[5];

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
  static bool startRouteFlag = true;
  while (0 < Wire.available())  // loop through all
  {
    byte receivedByte = Wire.read();
    courseArray[courseArrlength] = receivedByte - '0';  // Convert from ASCII to integer
    courseArrlength++;
    if (startRouteFlag) {
      input = courseArray[0];
      startRouteFlag = false;
    }
  }
}

//Lader opp batteriet og pauser i 5 sekund

void Charge() {
  motors.setSpeeds(0, 0);
  display.clear();
  display.println("CHARGING");
}

//Sende distanse kjørt til ESP, kjøres når bilen lader

void sendDistance() {
  int partDis = static_cast<int>(partDisGlobal);
  Wire.write(partDis);
  Wire.write(distMultiplier);
  sendChargeDist = false;
  distMultiplier = 0;
  partDisGlobal = 0;  //Resetter avstanden etter den er sendt
}

void lineFollowPID() {  // tar inn posisjonen
  static short prevPos;
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
  int filler[3] = { 3, 2, 1 };
  static byte turnCount = 0;
  switch (input) {
    case 3:
      static bool leftFlag = false;
      static bool leftFlag2 = true;
      static byte leftCounter = 0;
      static uint32_t leftTime = millis();
      showBattery();
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
        Serial.println("turn Complete");
        pidFlag = true;
      }
      if (leftCounter >= 4) {  //tar å resetter counter og fullfører denne svingen etter bilen er ute av kryset
        leftCounter = 0;
        input = 4;
        break;
      }
      break;
    case 2:
      static bool straightFlag = false;
      static byte straightCounter = 0;
      lineFollowPID();
      showBattery();
      if (lineSensors.readOneSens(drip) >= 700) straightFlag = true;    //merker at den har kommet på en svart linje på venstre side av bilen
      else if (lineSensors.readOneSens(drip) <= 150 && straightFlag) {  //teller + 1 etter bilen har pasert linja
        straightCounter++;
        straightFlag = false;
      }
      if (straightCounter >= 2) {  //om den har pasert to linjer går den videre til neste steg
        straightCounter = 0;
        input = 4;
        break;
      }
      break;
    case 1:
      showBattery();
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
    case 4:
      static bool switcher = true;
      static uint32_t switcherTime = millis();
      lineFollowPID();
      showBattery();
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
      display.gotoXY(1, 10);
      display.print(turnCount);
      break;
    case 5:
      static uint32_t chargeEndTime = millis();
      static bool chargeEndFlag, chargeSendFlag = true;
      if (lineSensors.readOneSens(drip) >= 700) {
        Charge();
        if (chargeSendFlag == true) {
          sendDistance();
          chargeSendFlag = false;
        }
      } else {
        lineFollowPID();
        showBattery();
      }
      if ((turnCount + 1) != courseArrlength) {
        if (chargeEndFlag) {
          chargeEndTime = millis();
          chargeEndFlag = false;
        } else if (millis() - chargeEndTime >= 3000) {
          chargeEndFlag = true;
          chargeSendFlag = true;
          input = 4;
          break;
        }
      }
      break;
    default:
      showBattery();
      motors.setSpeeds(0, 0);
      Serial.println(turnCount);
      Serial.println(courseArrlength);
      if (turnCount != courseArrlength) {
        if (turnCount == 0){
          input = courseArray[0];
        }else input = 4;
        break;
      }
      break;
  }
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
  EEPROM.write(0, 80);
  EEPROM.write(1, 0);
  EEPROM.write(2, 0);
  power = EEPROM.read(0);
  partDisGlobal = EEPROM.read(1);
  distMultiplier = EEPROM.read(2);
  pidSetup();
}

void loop() {
  static long tid;  //skal bort
  partDisGlobal = distMeasure();
  totalDistance = partDisGlobal + (distMultiplier * 255);
  power = batteryDrain(power);
  drivingMain();
}