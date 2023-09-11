#include "Varibles.h"

/***** Priavte Varibles Definition*****/
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
float T_DETACH = 1.0;
float T_PARACHUTE = 6.0;
int HEIGHT_PARACHUTE = 65;

String nowTime;
String fileName;

bool Sign_Parachute = false;
bool sign_setTime = false;
bool sign_beginNTPClient = false;
