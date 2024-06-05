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
byte power, input;
float disGlobal;
unsigned long distance;
int courseArray[30] = {};
byte courseArrlength = 0;
bool distSend = false;
static int drip[5];  //trengs for å kunne lese av spesfik sensor

int rightSpeed = 200;
int leftSpeed = 200;
int previousError;
float output;
double integral;
double derivative;
unsigned int lineSensorValues[5];

//Avstand kjørt, 1m kjøring er 10km simulert kjøring.

float distMeasure() {
  int currRotLeft = encoder.getCountsAndResetLeft();
  int currRotRight = encoder.getCountsAndResetRight();
  float leftDist = ((abs(currRotLeft)) * 3.1415 * 0.039) / 910;
  float rightDist = ((abs(currRotRight)) * 3.1415 * 0.039) / 910;
  float distPart = disGlobal + (10 * (leftDist + rightDist) / 2);
  EEPROM.write(1, disGlobal);
  return distPart;
}

//Genererer batterinivået, mellom 0 og 80

int batteryDrain(byte battery) {
  battery = 80 - (disGlobal / 5);
  if (battery < 0) {
    battery = 0;
  }
  EEPROM.write(0, battery);
  return battery;
}

//Viser batteriet og avstand kjørt på displayet

void showBattery() {
  display.gotoXY(0, 0);
  display.print("Power:  ");
  display.gotoXY(0, 1);
  display.print(power);
  display.gotoXY(0, 3);
  display.println("Distance drove; ");
  display.gotoXY(0, 4);
  display.print(disGlobal);
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
  if (distSend == true) {
    int kWhCharged = 0.2 * disGlobal;  //static_cast<int>(disGlobal);
    Serial.print(kWhCharged);
    Wire.write(kWhCharged);
    disGlobal = 0;  //Resetter avstanden etter den er sendt
    Serial.print("Sender melding   ");
    Serial.println(disGlobal);
    distSend = false;
  }
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
  static byte turnCount = 0;
  switch (input) {
    case 1:  //høyere
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

    case 2:  //rettfrem
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
    case 3:  //venstre
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
        motors.setSpeeds(-100, 150);
        leftTime = millis();
        leftFlag2 = false;
        pidFlag = false;  //skrur av PID kjøring
        leftCounter++;
      }
      if (leftFlag2 == false && millis() - leftTime >= 700) {  //avsluttersvingen og skrur på PID kjøring
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
      break;
    case 5:  //lading
      static uint32_t chargeEndTime = millis();
      static bool chargeEndFlag, chargeSendFlag = true;
      if (lineSensors.readOneSens(drip) >= 700) {
        Charge();
        if (chargeSendFlag == true) {
          distSend = true;
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
    case 6: //parker
      if (lineSensors.readOneSens(drip) >= 700) input = 0;
      else lineFollowPID();
      break;
    default:
      showBattery();
      motors.setSpeeds(0, 0);
      if (turnCount != courseArrlength) {
        input = courseArray[turnCount];
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
  power = EEPROM.read(0);
  disGlobal = EEPROM.read(1);
  pidSetup();
}

void loop() {
  /*skal bort
  static long time = millis();
  motors.setSpeeds(100,100);
  if (millis()-time >= 5000) {
    Serial.print("Distanse  ");
    Serial.println(disGlobal);
    distSend = true;
    time = millis();
  } //skal bort*/
  disGlobal = distMeasure();
  //Serial.println(disGlobal);
  power = batteryDrain(power);

  drivingMain();
}