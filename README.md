# SmartCityMP
for å kjøre carCode og BilForbruk kreves et litt modifisert versjon av biblioteket til zumo 32U4

i "QTRSensors.cpp" må følgende legges til:
"int QTRSensors::readOneSens(unsigned int *sensor_values, 
    unsigned char readMode)
{
    readCalibrated(sensor_values, readMode);
    int oneSensValue = sensor_values[0];
    return oneSensValue;
}"

i "QTRSensors.h" unner class "QTRSensors" trengs:
"int readOneSens(unsigned int* sensor_values, unsigned char readMode = QTR_EMITTERS_ON);"
