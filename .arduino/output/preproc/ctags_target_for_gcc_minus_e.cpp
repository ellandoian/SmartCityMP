# 1 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino"
//Antatt forbruk 0.2kWh per km, 400 km rekkevidde. Maks batteri er 80 kWh.

# 4 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino" 2
# 5 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino" 2
# 6 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino" 2
# 7 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino" 2
# 8 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino" 2

Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4ButtonC buttonC;
Zumo32U4LineSensors lineSensors;
Zumo32U4OLED display;
Zumo32U4Buzzer buzzer;
Zumo32U4Encoders encoder;


int power, distMultiplier;
unsigned long totalDistance;
float partDisGlobal;

//Avstand kjørt, 1m kjøring er 10km simulert kjøring.
//Etter 255km kjørt simulert, deles totaldistansen opp i et multiplum av 255 og en rest, slik at EEprom kan lagre hele distansen.

float distMeasure()
{
    if (partDisGlobal>=255) {
        distMultiplier += 1;
        partDisGlobal -= 255;
    }
    int currRotLeft = encoder.getCountsAndResetLeft();
    int currRotRight = encoder.getCountsAndResetRight();
    float leftDist = ((((currRotLeft)>0?(currRotLeft):-(currRotLeft)))*3.1415*0.039)/910;
    float rightDist = ((((currRotRight)>0?(currRotRight):-(currRotRight)))*3.1415*0.039)/910;
    float distPart = partDisGlobal + (10*(leftDist + rightDist)/2);
    EEPROM.write(1, partDisGlobal);
    EEPROM.write(2, distMultiplier);
    return distPart;
}

//Genererer batterinivået, mellom 0 og 80

int batteryDrain(int battery) {
    battery = 80 - (totalDistance/5);
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
# 57 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino" 3
                 (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 57 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino"
                 "Power: "
# 57 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino" 3
                 ); &__c[0];}))
# 57 "C:\\Users\\Magnus\\Documents\\GitHub\\SmartCityMP\\BilForbruk\\BilForbruk.ino"
                 )));
    display.gotoXY(0, 1);
    display.print(power);
    display.gotoXY(0, 3);
    display.println("Distance drove; ");
    display.gotoXY(0, 4);
    display.print(totalDistance);
    display.print("km");
}

//Tolker meldinger fra ESP

void Receive(int howMany) {
    while(0 < Wire.available()) // loop through all but the last
  {
    int c = Wire.read(); // receive byte as a character
    Serial.println(c); // print the character
    if (c == 5) {
        Charge();
    }
  }
}

//Lader opp batteriet

void Charge() {
    distMultiplier = 0;
    partDisGlobal = 0;
    display.clear();
}

//Sende distanse kjørt til ESP

void sendDistance() {
    Wire.write(totalDistance);
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
    EEPROM.write(2,0);
    power = EEPROM.read(0);
    partDisGlobal = EEPROM.read(1);
    distMultiplier = EEPROM.read(2);
}

void loop() {
    motors.setSpeeds(100,100);
    partDisGlobal = distMeasure();
    totalDistance = partDisGlobal+(distMultiplier*255);
    power = batteryDrain(power);
    showBattery();
}
