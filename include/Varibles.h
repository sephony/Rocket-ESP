#ifndef VARIBLES_H_
#define VARIBLES_H_

#include <Arduino.h>
#include <FS.h>
/***** Priavte Varibles Definition*****/
// extern File file;
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
extern double T_detach;
extern double T_para;
extern double H_para;
extern double T_protectPara;
extern unsigned int rgbBrightness;

extern String nowTime;
extern String fileName;
extern String lastFileName;
extern String lastRunMode;
extern String lastLaunchMode;
extern String lastLaunchReady;

extern bool Sign_Parachute;
extern bool sign_setTime;
extern bool sign_beginNTPClient;
extern bool sign_timeUpdate;
extern bool sign_needReset;

// extern char temp[20];

#endif /* VARIBLES_H_ */
