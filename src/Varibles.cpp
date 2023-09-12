#include "Varibles.h"

/***** Priavte Varibles Definition*****/
// File file;
/* Main Program Varibles */
unsigned long time_launch;
unsigned long time_para;
float current_time;
float height;
float height_filter;

char str_height[20];
char str_height_filter[20];
char str_time[20];
char InformationToPrint[100];

float H0 = 0;
double T_detach;
double T_para;
double H_para;
double T_protectPara;
unsigned int rgbBrightness;

String nowTime;
String fileName;
String lastFileName;
String lastRunMode;
String lastLaunchMode;
String lastLaunchReady;

bool Sign_Parachute = false;
bool sign_setTime = false;
bool sign_beginNTPClient = false;
bool sign_timeUpdate = false;
bool sign_needReset = false;
