#ifndef GAMELOAD
#define GAMELOAD
#include <stdbool.h>

bool gameInit(char* mission);
bool gameClose();
void restartMission();
void runMission(int toTime);

#endif // GAMELOAD
