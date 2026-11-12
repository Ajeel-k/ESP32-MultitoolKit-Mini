#ifndef SENZ_HPP
#define SENZ_HPP

#include "main.hpp"

/* Function protos */
float readTemperatureCelsius();
float fetchSensorValue(int type);
String sensorName(int t);
String sensorUnit(int t);
int16_t valToY(float val, int type);
void drawGraphFrame(int type);
void redrawGraph(int type, int totalSamples, int windowSamples, int writeHead);
void runSensorApp(int type, int mode);
void runAllTogetherSensor();

#endif