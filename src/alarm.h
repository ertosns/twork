#define ALARM

#ifndef UTILS
#include "utils.h"
#endif

const char LIGHTS_LIGHT;
const char LIGHTS_ON;
const char LIGHTS_OFF;

int initalarm();
void localalert();
void networkalert();
void alert(String, int sec, int level);
void lights_flag(int);
void mute_lights();
