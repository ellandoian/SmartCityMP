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

//Måler avstand kjørt, 1m kjørt tilsvarer 10km i simuleringen.

float distMeasure() {
  int currRotLeft = encoder.getCountsAndResetLeft();
  int currRotRight = encoder.getCountsAndResetRight();
  float leftDist = ((abs(currRotLeft)) * 3.1415 * 0.039) / 910;
  float rightDist = ((abs(currRotRight)) * 3.1415 * 0.039) / 910;
  float distPart = (10 * (leftDist + rightDist) / 2);
  return distPart;
}

//Genererer batterinivået, som er mellom 0 og 80

int batteryDrain() {
  int battery = 80 - (disGlobal / 5);
  if (battery < 0) {
    battery = 0;
  }
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

//Tolker meldinger fra ESP og konverterer fra string til int

void Receive(int howMany) {
  static bool startRouteFlag = true;
  while (0 < Wire.available())  // looper gjennom butikken.
  {
    byte receivedByte = Wire.read();
    courseArray[courseArrlength] = receivedByte - '0';  // Converterer fra string til integer
    courseArrlength++;
    if (startRouteFlag) {
      input = courseArray[0];
      startRouteFlag = false;
    }
  }
}

//Viser i displayet at bilen lader

void Charge() {
  motors.setSpeeds(0, 0);
  display.clear();
  display.println("CHARGING");
}

//Sender antall kWh ladet til ESP, sendes en gang når bilen lader.

void sendCharge() {
  if (distSend == true) {
    int kWhCharged = 0.2 * disGlobal;
    Wire.write(kWhCharged);
    disGlobal = 0;  //Nullstiller avstand kjørt.
    distSend = false; //Sørger for a avstanden blir kun sendt en gang hver gang den lader.
  }
}

// Bilens linjefølgingskode. Vil følge en teip som representerer veien.

void lineFollowPID() {  
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

//Konverterer rutedata til bilens kjøremønster.

void drivingMain() {
  static byte turnCount = 0;
  switch (input) {
    case 1:  //høyresving
      showBattery();
      static bool rightFlag = false;
      static uint32_t rightTime = millis();
      if (lineSensors.readOneSens(drip) >= 700) {  //Om bilen har kommet til et kryss vil den svinge til høyre
        rightTime = millis();
        motors.setSpeeds(150, -100);
        rightFlag = true;
      }
      if (millis() - rightTime >= 350 && rightFlag) {  //om bilen har fullført svingen hopper bilen til case 4
        input = 4;
        rightFlag = false;
        break;
      } else if (!rightFlag) lineFollowPID();  //kjører linjefølging om ingen sving
      break;

    case 2:  //rett frem
      static bool straightFlag = false;
      static byte straightCounter = 0;
      lineFollowPID();
      showBattery();
      if (lineSensors.readOneSens(drip) >= 700) straightFlag = true;    //merker at den har kommet på en svart linje på venstre side av bilen
      else if (lineSensors.readOneSens(drip) <= 150 && straightFlag) {  //teller + 1 etter bilen har pasert linja
        straightCounter++;
        straightFlag = false;
      }
      if (straightCounter >= 2) {  //om den har pasert to linjer går den til case 4
        straightCounter = 0;
        input = 4;
        break;
      }
      break;
    case 3:  //venstresving
      static bool leftFlag = false;
      static bool leftFlag2 = true;
      static byte leftCounter = 0;
      static uint32_t leftTime = millis();
      showBattery();
      lineFollowPID();
      if (lineSensors.readOneSens(drip) >= 700) {  //merker at den rører en linje og setter av et flag
        leftFlag = true;
      }
      else if (lineSensors.readOneSens(drip) < 100 && leftFlag) {  //når bilen har gått av linjen flippes flaget tilbake og counter går +1
        leftCounter++;
        leftFlag = false;
      }

      if (lineSensors.readOneSens(drip) >= 700 && leftCounter == 1) {  //når bilen kommer til en linje etter å ha passert en vil den svinge til venstre
        motors.setSpeeds(-100, 150);
        leftTime = millis();
        leftFlag2 = false;
        pidFlag = false;  //skrur av linjefølging
        leftCounter++;
      }
      if (leftFlag2 == false && millis() - leftTime >= 700) {  //avslutter svingen og skrur på linjefølging
        leftFlag2 = true;
        pidFlag = true;
      }
      if (leftCounter >= 4) {  //resetter counter og fullfører svingen etter bilen er ute av kryset
        leftCounter = 0;
        input = 4;
        break;
      }
      break;
    case 4:  //iterer til neste case i rutedataen
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
      if (lineSensors.readOneSens(drip) >= 700) { //når bilen merker teip på venstre sensor stopper den og lader.
        Charge();
        if (chargeSendFlag == true) { //lar sendChargefunksjonen sende strøm ladet for én gang.
          distSend = true;
          chargeSendFlag = false;
        }
      } else { //om bilen ikke merker teip på venstre side kjøres vanlig linjefølging.
        lineFollowPID();
        showBattery();
      }
      if ((turnCount + 1) != courseArrlength) { //Om bilen blir sendt ny rute, vil den først kjøre ut av ladestasjonen før den iterer gjennom den nye ruta.
        if (chargeEndFlag) {
          chargeEndTime = millis();
          chargeEndFlag = false;
        } else if (millis() - chargeEndTime >= 5000) {
          chargeEndFlag = true;
          chargeSendFlag = true;
          input = 4;
          break;
        }
      }
      break;
    case 6: //parkere
      if (lineSensors.readOneSens(drip) >= 700) motors.setSpeeds(0,0); //Om bilen merker teip på venstre side stopper den.
      else lineFollowPID();
      if((turnCount + 1 != courseArrlength)) input = 4;
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

//Kalibrerer bilen til å følge teipen.

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
  Wire.begin(1); //Oppretter I2C kommunikasjon med ESP32.
  Wire.onRequest(sendCharge);
  Wire.onReceive(Receive);
  display.setLayout21x8();
  //EEPROM.write(1, 0);  //Kan kommentere inn denne linjen om vi vil starte bilen på full lading.
  disGlobal = EEPROM.read(1);
  pidSetup(); //Bilen vil ikke gå ut av setup før den er kalibrert.
}

void loop() {
  disGlobal += distMeasure();
  EEPROM.write(1, disGlobal);
  power = batteryDrain();
  drivingMain();
}