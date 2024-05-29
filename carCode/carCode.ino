#include <Wire.h>
#include <Zumo32U4.h>
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4ButtonC buttonC;
Zumo32U4IMU imu;
Zumo32U4OLED display;

int topSpeed = 200;

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

int lineSensorRead() {
  static unsigned int lineSensorVal[5];  // lager en variable med like mange indekser som det er sensorer
  int error = map(lineSensors.readLine(lineSensorVal), 0, 4000, -2000, 2000);
  return error;
}

void lineFollowPID(int pos) {  // tar inn posisjonen
  static int prevPos;
  int correction = pos / 4 + 6 * (pos - prevPos);  // kilde eksempelkode
  prevPos = pos;
  int lSpeed = topSpeed + correction;       // farten på venstre side lik topSpeed + correction
  int rSpeed = topSpeed - correction;       // farten på høgre side lik topspeed - correction
  lSpeed = constrain(lSpeed, 0, topSpeed);  // setter slik at verdien vil alltids være mellom 200 og 0, vil forhindre for høye hastigheter, men viktigs
  rSpeed = constrain(rSpeed, 0, topSpeed);  // hindrer at det vil fort gå fra positiv hastighet til negativ hastighet som kan skade motorene.
  motors.setSpeeds(lSpeed, rSpeed);
}

void loop() {
  lineFollowPID(lineSensorRead());
}