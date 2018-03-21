#ifndef GAMEDATA
#define GAMEDATA

#include "GameUtilities.h"

#include <stdbool.h>

bool debug; // debug flag

// FUNCTIONS
int getFrameTime();
void setFrameTime(int newFrameTime);
Tactical* getTactical(int tacID);
void setMissionClock(unsigned int now);
unsigned int getMissionClock();
void setMission(char* mission);
char* getMission();
MissionEvent* getMissionEvent(int eventID);
bool readMission();
bool readObjects();
bool readAbilities();
bool createObject(char* name, int statsId, int team, Point pos, Point dst, float heading, float momentum, int lifespan);
Tactical* getClosestObjectToPoint(Point p);
bool getIsObjectUnderPoint(Tactical* obj, Point p);
void moveObject(Tactical* obj);
void moveAllObjects();
void clearAllObjects();
bool detectCollision(Tactical* obj1, Tactical* obj2);
int updateShieldStatus(Tactical* obj);
int getNumOfObjects();
int getNumOfAssets();
Asset* getAsset(int a);
void renderAllNonPlayerObjects();
Asset* saveAsset(char* path);

// ability scripts
void doAbility(Tactical *src, Tactical *tgt, int whichAbility);
void salvoCheck(Tactical *src, Tactical *tgt);
int getPhaserLinger();
void script_Phasers(Tactical *src, Tactical *tgt);
void script_PhotonTorpedo(Tactical *src, Tactical *tgt);
void script_QuantumTorpedo(Tactical *src, Tactical *tgt);
void script_PhaseCloak(Tactical *src, Tactical *tgt);
void script_GreenTorpedo(Tactical *src, Tactical *tgt);
void script_GreenTorpedoAimed(Tactical *src, Tactical *tgt);

#endif // GAMEDATA
