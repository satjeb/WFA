#include "GameLoad.h"
/*
    GameLoad.c
    Loads and stores data related to the game.
*/

#include "GameData.h"
#include "GameSDL.h"
#include "GameUtilities.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int nextMissionEvent = 0;

// initializes game resources, returning true if successful, or false if failed.
bool gameInit(char* mission) {
    setMission(mission);
    // load game data for mission
    if (!readObjects()) {
        printf("Failed to load ships.ini, file may be missing or improperly formatted. Unable to continue.\n");
        return false;
    }
    if (!readAbilities()) {
        printf("Failed to load abilities.ini, file may be missing or improperly formatted. Unable to continue.\n");
        return false;
    }
    if (!readMission()) {
        printf("Failed to load mission file, file may be missing or improperly formatted. Unable to continue.\n");
        return false;
    }
    // load SDL
    if (!init_GameSDL()) {
        printf("Failed to load SDL. Unable to continue.\n");
        return false;
    }
    // load assets
    if (!setAssets()) {
        printf("Failed to load all assets. Unable to continue.\n");
        return false;
    }

    // Create assets, this goes out and pulls all the textures needed, sparing us from load hangs during game time.
    for (int i = 0; i < getNumOfAssets(); i++) {
        getTexture(getAsset(i));
    }

    return true;
}

bool gameClose() {
    for (int i = 0; i < getNumOfAssets(); i++) {
        freeAsset(getAsset(i));
    }
    closeSDL();
    return true;
}

void restartMission() {
    clearAllObjects();
    nextMissionEvent = 0;
    setMissionClock(0);
}

// runs mission events according to script.
void runMission(int toTime) {
    MissionEvent* mEvent = getMissionEvent(nextMissionEvent);
    while (mEvent->time <= toTime && mEvent->eventType != 0) {
        if (debug) printf("%d: Execute mission event at %d\n", toTime, mEvent->time);
        // handle mission event
        switch (mEvent->eventType) {
        case 1: // create object (only one supported right now)
            createObject("Object", mEvent->objectType, mEvent->team, mEvent->pos, mEvent->dst, mEvent->heading, mEvent->momentum, mEvent->lifespan);
            break;
        default:
            break;
        }
        mEvent = getMissionEvent(++nextMissionEvent); // move to next missionEvent.
    }
}


