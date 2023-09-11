#ifndef VARIBLES_H_
#define VARIBLES_H_

#include <Arduino.h>
/***** Priavte Varibles Definition*****/
/* Main Program Varibles */
extern unsigned long time_launch;
extern unsigned long time_para;
extern float current_time;
extern float height;
extern float height_filter;

extern char str_height[20];
extern char str_height_filter[20];
extern char str_time[20];
extern char InformationToPrint[100];
extern float H0;
extern float T_DETACH;
extern float T_PARACHUTE;
extern int HEIGHT_PARACHUTE;

extern String nowTime;
extern String fileName;

extern bool Sign_Parachute;
extern bool sign_setTime;
extern bool sign_beginNTPClient;
// extern char temp[20];

#endif /* VARIBLES_H_ */
