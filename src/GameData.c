#include "GameData.h"

#include "GameUtilities.h"
#include "GameSDL.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

bool debug = false; // sets debug across application

// GAME DATA
char currentMission[30]; // holds current mission file name
Asset assets[200];
Stats tacStats[200]; // Stores all entered stats
Ability abilities[100]; // Stores data related to all abilities
Tactical tac[200]; // stores instances of objects which are actively on the screen.
MissionEvent missionEvents[10000]; // Stores the entire mission's script
unsigned int missionClock = 0;
int numOfObjects = 0;
int numOfAssets = 0;
int frameTime = 0;

int playerSalvoCount = 0; // temporary measure to get a salvo/phaser linger working until new measures can be made to properly support it.
int playerSalvoNext = 0;
int playerPhaserLinger = 0;

int getFrameTime() {
    return frameTime;
}

void setFrameTime(int newFrameTime) {
    frameTime = newFrameTime;
}

Tactical* getTactical(int tacID) {
    return &tac[tacID];
}

// sets the file name to parse to generate the current mission
void setMission(char* mission) {
    strcpy(currentMission, mission);
}

char* getMission() {
    return currentMission;
}

MissionEvent* getMissionEvent(int eventID) {
    return &missionEvents[eventID];
}

// parses current mission file into memory and returns true if successful.
bool readMission() {
    bool success = true;

    char buf[1024];
    char file[40] = "missions\\";
    strcat(file, currentMission);
    FILE *missionFile = fopen(file, "r");

    // We can afford to use the first line to sanity check our file, as by convention it's just column headers
    if (fgets(buf, 1024, (FILE*)missionFile) != NULL) {
        // Read the first real line and all that come after
        clock_t startTimer = clock();
        if (debug) printf("File at path %s found, parsing data to memory...", file);
        int i = 0;
        while (fgets(buf, 1024, (FILE*)missionFile) != NULL) {
            missionEvents[i].time = atoi(strtrim(strtok(buf, "|"))); // first token should include the buffer.
            missionEvents[i].eventType = atoi(strtrim(strtok(NULL, "|"))); // subsequent ones NULL until next line.
            missionEvents[i].objectType = atoi(strtrim(strtok(NULL, "|")));
            missionEvents[i].team = atoi(strtrim(strtok(NULL, "|")));
            missionEvents[i].pos.x = atof(strtrim(strtok(NULL, "|")));
            missionEvents[i].pos.y = atof(strtrim(strtok(NULL, "|")));
            missionEvents[i].dst.x = atof(strtrim(strtok(NULL, "|")));
            missionEvents[i].dst.y = atof(strtrim(strtok(NULL, "|")));
            missionEvents[i].heading = atof(strtrim(strtok(NULL, "|")));
            missionEvents[i].momentum = atof(strtrim(strtok(NULL, "|")));
            missionEvents[i].lifespan = atoi(strtrim(strtok(NULL, "|")));
            i++;
        }
        if (debug) printf(" Done. Loading took %ld ms.\n", clock() - startTimer);
    } else {
        printf("Unable to load mission at path = %s (File is either missing or empty)\n", file);
        success = false;
    }

    fclose(missionFile);

    return success;
}

// read object assets from file into memory.
bool readObjects() {
    bool success = true;

    char buf[1024];
    char file[40] = "ships\\ships.ini";
    FILE *statsFile = fopen(file, "r");

    // We can afford to use the first line to sanity check our file, as by convention it's just column headers
    if (fgets(buf, 1024, (FILE*)statsFile) != NULL) {
        // Read the first real line and all that come after
        clock_t startTimer = clock();
        if (debug) printf("File at path %s found, parsing data to memory...\n", file);
        int i = 0;
        while (fgets(buf, 1024, (FILE*)statsFile) != NULL) {
            strcpy(tacStats[i].shipClass, strtrim(strtok(buf, "|"))); // first token should include the buffer.
            tacStats[i].asset = saveAsset(strtrim(strtok(NULL, "|"))); // subsequent ones NULL until next line.
            tacStats[i].assetScale = atof(strtrim(strtok(NULL, "|")));
            tacStats[i].speed = atof(strtrim(strtok(NULL, "|"))) / 1000.0;
            tacStats[i].turn = atof(strtrim(strtok(NULL, "|"))) / 1000.0;
            tacStats[i].accel = atof(strtrim(strtok(NULL, "|"))) / 1000.0 / 1000.0;
            tacStats[i].maxHP = atoi(strtrim(strtok(NULL, "|")));
            tacStats[i].regenHP = atoi(strtrim(strtok(NULL, "|")));
            tacStats[i].regenCycleRate = atoi(strtrim(strtok(NULL, "|")));
            tacStats[i].ability1 = atoi(strtrim(strtok(NULL, "|")));
            tacStats[i].ability2 = atoi(strtrim(strtok(NULL, "|")));
            tacStats[i].ability3 = atoi(strtrim(strtok(NULL, "|")));
            tacStats[i].ability4 = atoi(strtrim(strtok(NULL, "|")));
            tacStats[i].defaultCollision = atoi(strtrim(strtok(NULL, "|")));
            tacStats[i].defaultEdgePattern = atoi(strtrim(strtok(NULL, "|")));
            tacStats[i].dmg = atoi(strtrim(strtok(NULL, "|")));
            tacStats[i].script = atoi(strtrim(strtok(NULL, "|")));
            if (debug) printf("Loaded %s to tacStats[%d] with asset path %s.\n", tacStats[i].shipClass, i, tacStats[i].asset->path);

            i++;
        }
        if (debug) printf("Done loading file at path %s. Loading took %ld ms.\n", file, clock() - startTimer);
    } else {
        printf("Unable to load object data at path = %s (File is either missing or empty)\n", file);
        success = false;
    }

    fclose(statsFile);

    return success;
}

// read available abilities from file
bool readAbilities() {
    bool success = true;

    char buf[1024];
    char file[40] = "ships\\abilities.ini";
    FILE *statsFile = fopen(file, "r");

    // We can afford to use the first line to sanity check our file, as by convention it's just column headers
    if (fgets(buf, 1024, (FILE*)statsFile) != NULL) {
        // Read the first real line and all that come after
        clock_t startTimer = clock();
        if (debug) printf("File at path %s found, parsing data to memory...\n", file);
        int i = 0;
        while (fgets(buf, 1024, (FILE*)statsFile) != NULL) {
            strcpy(abilities[i].name, strtrim(strtok(buf, "|"))); // first token should include the buffer.
            switch (atoi(strtrim(strtok(NULL, "|")))) {
                case 1:
                    abilities[i].script = script_Phasers;
                    break;
                case 2:
                    abilities[i].script = script_PhotonTorpedo;
                    break;
                case 3:
                    abilities[i].script = script_QuantumTorpedo;
                    break;
                case 4:
                    abilities[i].script = script_PhaseCloak;
                    break;
                default:
                    break;
            }
            abilities[i].relatedObject = atoi(strtrim(strtok(NULL, "|"))); // Typically used for graphic assets.
            abilities[i].cooldown = atoi(strtrim(strtok(NULL, "|")));
            abilities[i].attr1 = atoi(strtrim(strtok(NULL, "|")));
            abilities[i].attr2 = atoi(strtrim(strtok(NULL, "|")));
            abilities[i].attr3 = atoi(strtrim(strtok(NULL, "|")));
            abilities[i].attr4 = atoi(strtrim(strtok(NULL, "|")));
            abilities[i].attr5 = atoi(strtrim(strtok(NULL, "|")));
            if (debug) printf("Loaded %s to abilities[%d] with asset path %s.\n", abilities[i].name, i, tacStats[i].asset->path);

            i++;
        }
        if (debug) printf("Done loading file at path %s. Loading took %ld ms.\n", file, clock() - startTimer);
    } else {
        printf("Unable to load object data at path = %s (File is either missing or empty)\n", file);
        success = false;
    }

    fclose(statsFile);

    return success;
}

// creates an object to the tactical screen with a set name, stats from tacStats, team, pos being its position,
// dst being its destination (if any), heading, initial momentum, and how long to exist.
// Note that a lifespan of 0 indicates indefinite lifespan. All other values are in milliseconds.
bool createObject(char* name, int statsId, int team, Point pos, Point dst, float heading, float momentum, int lifespan) {
    bool success = true;

    if (debug) printf("Created object \"%s\" (ID: %d) at location (%f,%f).\n", name, statsId, pos.x, pos.y);

    // Attempt to reuse old memory locations which are no longer relevant, but avoid replacing the player object.
    for (int i = 0; i <= numOfObjects; i++) {
        if (i == numOfObjects || (tac[i].hp < 0 && i > 0)) {
            strcpy(tac[i].name, name);
            tac[i].stats = &tacStats[statsId];
            tac[i].team = team;
            tac[i].pos = pos;
            tac[i].dst = dst;
            tac[i].heading = heading;
            tac[i].momentum = momentum;
            tac[i].hp = tacStats[statsId].maxHP;
            tac[i].lastFrameHp = tacStats[statsId].maxHP;
            tac[i].nextRegenCycle = getMissionClock() + tacStats[statsId].regenCycleRate;
            tac[i].movement = (i == 0 ? 0 : MoveForward); // default to moving forward for non-player objects
            // calculate a point closer to the bow for more aesthetically pleasing weapons firing.
            float emitterX = 0;
            float emitterY = -1 * tac[i].stats->asset->h / 4 * tac[i].stats->assetScale;
            tac[i].emitter = (Point){emitterX, emitterY};
            tac[i].ability1avail = getMissionClock() + 1000; // all abilities on a 1s cooldown to start with to avoid "accidents" and help bots not fire the moment they spawn
            if (statsId == 12) tac[i].ability1avail = getMissionClock() + 3000; // extra time for asteroid turrets until I get better cooldown management.
            tac[i].ability2avail = getMissionClock() + 1000;
            tac[i].ability3avail = getMissionClock() + 1000;
            tac[i].ability4avail = getMissionClock() + 1000;
            if (lifespan > 0) {
                tac[i].lifespan = getMissionClock() + lifespan;
            } else {
                tac[i].lifespan = 0; // indicates indefinite lifespan
            }
            if (i == numOfObjects) {
                numOfObjects++; // increment only if we could not find an empty spot.
            }
            if (debug) printf("Used location: %d out of %d.\n", i, numOfObjects);
            break;
        }
    }
    return success;
}

// returns a pointer to the closest object to a Point, within reason.
Tactical* getClosestObjectToPoint(Point p) {
    Tactical *closestObject = &tac[0]; // player object assumed to be no target
    for (int i = 0; i < numOfObjects; i++) {
        if (tac[i].team != 1 &&
            tac[i].hp > -1 &&
            getIsObjectUnderPoint(&tac[i], p) &&
            getDistanceBetweenTwoPoints(tac[i].pos, p) < getDistanceBetweenTwoPoints(tac[i].pos, closestObject->pos)
            )
        {
            closestObject = &tac[i];
        }
    }
    return closestObject;
}

// returns true if object is actually under the point, though this only determines it by a circle of diameter equal to its width,
// regardless of whether or not the object is square.
bool getIsObjectUnderPoint(Tactical* obj, Point p) {
    float maximumDistance = obj->stats->asset->h / 2.0 * obj->stats->assetScale;
    if (getDistanceBetweenTwoPoints(obj->pos, p) <= maximumDistance) {
        return true;
    } else {
        return false;
    }
}

// move an object along to the following inputs
void moveObject(Tactical* obj) {
    bool forward = obj->movement & MoveForward;
    bool back = obj->movement & MoveBackward;
    bool turnLeft = obj->movement & TurnLeft;
    bool turnRight = obj->movement & TurnRight;

    // handle turns
    if (turnLeft) obj->heading = obj->heading - (obj->stats->turn * frameTime);
    if (turnRight) obj->heading = obj->heading + (obj->stats->turn * frameTime);
    if (obj->heading >= 360) obj->heading = obj->heading - 360;
    if (obj->heading < 0) obj->heading = obj->heading + 360;

    // handle movement
    float x = obj->pos.x;
    float y = obj->pos.y;
    if (forward) {
        obj->momentum = obj->momentum + obj->stats->accel * frameTime;
        if (obj->momentum > obj->stats->speed) {
            obj->momentum = obj->stats->speed;
        }
    }
    if (back) {
        obj->momentum = obj->momentum - obj->stats->accel * frameTime;
        if (obj->momentum < obj->stats->speed / -2.5) {
            obj->momentum = obj->stats->speed / -2.5;
        }
    }
    if (!forward && !back) {
        if (obj->momentum <= obj->stats->accel && obj->momentum >= -1.0*obj->stats->accel) {
            obj->momentum = 0;
        } else if (obj->momentum < 0) {
            obj->momentum = obj->momentum + obj->stats->accel * frameTime;
        } else if (obj->momentum > 0) {
            obj->momentum = obj->momentum - obj->stats->accel * frameTime;
        }
    }
    // if any momentum is generated, perform movement with respect to its edge pattern.
    if (obj->momentum != 0) {
        x = x + cos((obj->heading-90) / 360.0 * 2.0 * M_PI) * obj->momentum * frameTime;
        y = y + sin((obj->heading-90) / 360.0 * 2.0 * M_PI) * obj->momentum * frameTime;
        Point p = (Point){x,y};
        // If no valid point is set, destroy the object.
        if (validPoint(&p, obj->stats->defaultEdgePattern)) {
            obj->pos = p;
        } else {
            if (debug) printf("Object %s was destroyed for leaving the relevant area at point (%.1f, %.1f).\n", obj->name, x, y);
            obj->hp = -1; // if it can only give us an invalid point, then "destroy" the object by setting it to -1 HP, where much of the game will ignore it.
        }
    }
}

// Moves all objects in accordance with their desired behavior.
void moveAllObjects() {
    for (int i = 0; i < numOfObjects; i++) {
        // check if it's a temporary object first and if its lifespan is exceeded, if so kill it (again, zero indicates indefinite lifespan)
        if (tac[i].lifespan != 0 && tac[i].lifespan < getMissionClock() && tac[i].hp > -1) {
            tac[i].hp = -1;
            if (debug) printf("Destroyed object %s for exceeding its lifespan.\n", tac[i].name);
        }
        if (tac[i].stats->asset == NULL || tac[i].hp == -1) {
            continue; // object does not exist! Or is ded, move on
        }

        // temp AI test (Limited Pursuit = 1, Unrestricted Pursuit = 2)
        // Limited Pursuit will only turn between +/- 20 degrees from 0.
        // Unrestricted Pursuit is a bloodhound.
        if (tac[i].stats->script != 0) {
            float turningRemaining = getHeadingToLocation(tac[i].pos, tac[0].pos) - tac[i].heading;
            if (abs(turningRemaining) > 180) { // flip turn if needed so we always turn the shortest way
                turningRemaining = -1 * turningRemaining - 180;
            }
            if (turningRemaining < 0) {
                tac[i].movement &= ~TurnRight; // stop turning right if turning right and start turning left.
                tac[i].movement |= TurnLeft;
            } else {
                tac[i].movement &= ~TurnLeft; // stop turning left if turning left and start turning right.
                tac[i].movement |= TurnRight;
            }

            if (tac[i].stats->script == 1) { // limited pursuit mode for tailers
                if (turningRemaining < 0 && tac[i].heading < 340 && tac[i].heading > 180) {
                    tac[i].movement &= ~(TurnLeft);
                }
                if (turningRemaining > 0 && tac[i].heading > 20 && tac[i].heading < 180) {
                    tac[i].movement &= ~(TurnRight);
                }
            }
        }

        // Run script to move objects in accordance with their current commands.
        moveObject(&tac[i]);

        // Have it fire its primary ability if it has one and it's not on cooldown, as long as it's not the player.
        if (tac[i].hp > -1 && tac[i].stats->ability1 > 0 && tac[i].ability1avail <= getMissionClock() && i > 0) {
            tac[i].ability1avail = getMissionClock() + 4000;
            switch (tac[i].stats->ability1) {
            case 5:
                script_GreenTorpedo(&tac[i], &tac[0]);
                break;
            case 6:
                script_GreenTorpedoAimed(&tac[i], &tac[0]);
                break;
            default:
                break;
            }
        }
    }
}

// sets all objects to -1 HP, effectively clearing them, though lazily.
void clearAllObjects() {
    for (int i = 0; i < numOfObjects; i++) {
        tac[i].hp = -1;
    }
    numOfObjects = 0;
}

// COLLISION DETECTION
// determines if there has been a collision between the two given objects, returning true if so, false if no (damn you rhymes)
bool detectCollision(Tactical* obj1, Tactical* obj2) {
    bool collision = false;

    int firstNumOfPoints = 4; // hard set until data changes complete
    int secondNumOfPoints = 4;

    Point cTac1[firstNumOfPoints];
    Point cTac2[secondNumOfPoints];

    // generate each hit box
    // get offsets (should be included in Tactical later)
    float obj1Width = obj1->stats->asset->w * obj1->stats->assetScale;
    float obj1Height = obj1->stats->asset->h * obj1->stats->assetScale;
    cTac1[0].x = 0 - obj1Width/2;
    cTac1[0].y = 0 - obj1Height/2;
    cTac1[1].x = 0 + obj1Width/2;
    cTac1[1].y = 0 - obj1Height/2;
    cTac1[2].x = 0 + obj1Width/2;
    cTac1[2].y = 0 + obj1Height/2;
    cTac1[3].x = 0 - obj1Width/2;
    cTac1[3].y = 0 + obj1Height/2;

    float obj2Width = obj2->stats->asset->w * obj2->stats->assetScale;
    float obj2Height = obj2->stats->asset->h * obj2->stats->assetScale;
    cTac2[0].x = 0 - obj2Width/2;
    cTac2[0].y = 0 - obj2Height/2;
    cTac2[1].x = 0 + obj2Width/2;
    cTac2[1].y = 0 - obj2Height/2;
    cTac2[2].x = 0 + obj2Width/2;
    cTac2[2].y = 0 + obj2Height/2;
    cTac2[3].x = 0 - obj2Width/2;
    cTac2[3].y = 0 + obj2Height/2;

    // calculate collision points of obj1
    for (int i = 0; i < firstNumOfPoints; i++) {
        cTac1[i] = rotatePointAroundCenter(cTac1[i], obj1->pos, obj1->heading);
    }
    for (int i = 0; i < secondNumOfPoints; i++) {
        cTac2[i] = rotatePointAroundCenter(cTac2[i], obj2->pos, obj2->heading);
    }

    if (debug) {
        for (int i = 0; i < firstNumOfPoints; i++) {
            renderGreenLine(cTac1[i], cTac1[(i+1) % firstNumOfPoints]);
        }
    }

    bool debugLines1[4];
    bool debugLines2[4];

    debugLines1[0] = false;
    debugLines1[1] = false;
    debugLines1[2] = false;
    debugLines1[3] = false;
    debugLines2[0] = false;
    debugLines2[1] = false;
    debugLines2[2] = false;
    debugLines2[3] = false;


    // check if either center point is in the other object (might drop this for performance if needed, it won't detect being in the edges of a concave structure)
    // besides most all collisions will be line segment intersects
    if (isPointInShape(obj1->pos, cTac2) || isPointInShape(obj2->pos, cTac1)) {
        collision = true;
    } else {
        for (int i = 0; i < firstNumOfPoints; i++) {
            Point lineI[2] = { cTac1[i], cTac1[(i+1)%firstNumOfPoints] };
            for (int j = 0; j < secondNumOfPoints; j++) {
                Point lineJ[2] = { cTac2[j], cTac2[(j+1)%secondNumOfPoints] };
                //printf("Checking line a%d to b%d.\n",i,j);
                if (isIntersected(lineI, lineJ)) {
                    collision = true;
                    if (!debug) break; // one collision is all we need, no need to check more, debug disables so we can check all collisions
                    else {
                        debugLines1[i] = true;
                        debugLines2[j] = true;
                    }
                }
            }
            if (collision && !debug) break; // if we found a collision, break out of this loop too to avoid needless extra processing
        }
    }

    if (debug) { // collision line drawing
        for (int i = 0; i < firstNumOfPoints; i++) {
            if (debugLines1[i]) {
                renderRedLine(cTac1[i], cTac1[(i+1) % firstNumOfPoints]);
            } else {
                renderGreenLine(cTac1[i], cTac1[(i+1) % firstNumOfPoints]);
            }
        }
        for (int i = 0; i < secondNumOfPoints; i++) {
            if (debugLines2[i]) {
                renderRedLine(cTac2[i], cTac2[(i+1) % secondNumOfPoints]);
            } else {
                renderGreenLine(cTac2[i], cTac2[(i+1) % firstNumOfPoints]);
            }
        }
    }

    return collision;
}

// Handle damage states and shield regen for object, if any
int updateShieldStatus(Tactical *obj) {
    // Object had no shields and was hit = ded for sure.
    if (obj->hp < 0 && obj->lastFrameHp == 0) {
        createObject("Explosion", 9, 1, obj->pos, obj->dst, 180, 59, 750);
    }
    // Object had shields before but now no more.
    if (obj->hp <= 0 && obj->lastFrameHp > 0) {
        if (debug) printf("SHIELDS ARE DOWN! Restoring will take 10 seconds!\n");
        obj->hp = 0;
        //obj->invincibleUntil = getMissionClock() + 1000; // grace period before death can occur (at least for the player)
        obj->nextRegenCycle = getMissionClock() + 10000; // Restoring will take ten seconds from shields down.
    }
    // Object still has shields but took damage: reset regen cycle
    if (obj->hp > 0 && obj->hp < obj->lastFrameHp) {
        if (debug) printf("Shields down to %d%%.\n", obj->hp / (obj->stats->maxHP / 100));
        obj->nextRegenCycle = getMissionClock() + obj->stats->regenCycleRate; // No freebies!
    }
    // Object has shields and was not hit (technically if one healed for more than the hit in the same frame, this would still trigger, but I'm fine with that)
    if (obj->hp > 0 && obj->hp >= obj->lastFrameHp && obj->lastFrameHp != obj->stats->maxHP) {
        if (getMissionClock() >= obj->nextRegenCycle) { // Shield regen tick if appropriate
            if (debug) printf("Shields restored to %d%%.\n", obj->hp / (obj->stats->maxHP / 100));
            obj->hp = obj->hp + obj->stats->regenHP;
            obj->nextRegenCycle = getMissionClock() + obj->stats->regenCycleRate;
        }
    }
    if (obj->hp == 0 && obj->lastFrameHp == 0) {
        if (getMissionClock() >= obj->nextRegenCycle) { // Shield regen tick if appropriate
            if (debug) printf("Shields are back online at %d%%.\n", obj->hp / (obj->stats->maxHP / 100));
            obj->hp = obj->stats->maxHP / 4;
            obj->nextRegenCycle = getMissionClock() + obj->stats->regenCycleRate;
        }
    }
    // Make sure we don't get more shields than the max.
    if (obj->hp > obj->stats->maxHP) {
        obj->hp = obj->stats->maxHP;
    }

    return obj->hp;
}

// returns the number of tac nodes which are active. Deleted objects are reclaimed so this number only goes up
// if we get more active objects in the game.  Currently doesn't attempt to cut it back down.
int getNumOfObjects() {
    return numOfObjects;
}

// returns the total number of Assets saved from mission file.
int getNumOfAssets() {
    return numOfAssets;
}

// retrieves pointer to an asset
Asset* getAsset(int a) {
    return &assets[a];
}

// This will search the asset tree to see if we already have an asset for that file, and avoid multiple loads of the same graphic.
Asset* saveAsset(char* path) {
    int i = 0;
    bool alreadyExists = false;
    char fullPath[50] = "assets\\";
    strcat(fullPath, path);

    while (assets[i].path[0] != '\0' && !alreadyExists) {
        if (strcmp(fullPath, assets[i].path) == 0) {
            alreadyExists = true;
            if (debug) printf("Asset %s compared to %s and found to already exist at %d. Will not be created.\n", fullPath, assets[i].path, i);
        }
        else i++;
    }
    if (!alreadyExists) {
        if (debug) printf("Asset %s did not exist, creating it at %d.\n", fullPath, i);
        strcpy(assets[i].path, fullPath);
        numOfAssets++;
    }

    return &assets[i];
}

// ABILITY SCRIPTS - will eventually get its own thing
void doAbility(Tactical *src, Tactical *tgt, int whichAbility) {
    Ability *abil = NULL;
    int cooldownRemaining = 0;
    bool fire = true;
    switch (whichAbility) {
        case 1:
            abil = &abilities[src->stats->ability1]; // point to relevant ability
            cooldownRemaining = src->ability1avail - missionClock;
            if (cooldownRemaining <= 0) { // if cooldown is done
                if (tgt->hp < 0 || tgt == src) { // Quick bug fix to prevent Phasers from going on cooldown when fired at already dead targets, will be replaced with a reworked system.
                    fire = false;
                    printf("%s did not fire: No target or target is already dead.\n", abil->name);
                } else {
                    src->ability1avail = missionClock + abil->cooldown; // set new cooldown
                }
            }
            break;
        case 2:
            abil = &abilities[src->stats->ability2];
            cooldownRemaining = src->ability2avail - missionClock;
            if (cooldownRemaining <= 0) {
                src->ability2avail = missionClock + abil->cooldown;
            }
            break;
        case 3:
            abil = &abilities[src->stats->ability3];
            cooldownRemaining = src->ability3avail - missionClock;
            if (cooldownRemaining <= 0) {
                src->ability3avail = missionClock + abil->cooldown;
            }
            break;
        case 4:
            abil = &abilities[src->stats->ability4];
            cooldownRemaining = src->ability4avail - missionClock;
            if (cooldownRemaining <= 0) {
                src->ability4avail = missionClock + abil->cooldown;
            }
            break;
        default:
            break;
    }
    if (cooldownRemaining > 0) {
        printf("%s is still on cooldown for %.1f more seconds!\n", abil->name, cooldownRemaining / 1000.0);
    } else if (fire) {
        if (abil->script == NULL) {
            printf("Error: Ability %d script not found for object %s.\n", whichAbility, src->name);
        } else {
            printf("%s activated. Cooldown is %.1f seconds.\n", abil->name, abil->cooldown / 1000.0);
            abil->script(src, tgt);
        }
    }
}

// quick way to do salvos, checking to see if there is more. Only works with quantums right now.
void salvoCheck(Tactical* src, Tactical* tgt) {
    if (playerSalvoCount > 0 && playerSalvoNext < getMissionClock()) {
        script_QuantumTorpedo(src, tgt);
    }
}

// returns phaser linger time, since single-frame phasers aren't very phasery.
int getPhaserLinger() {
    return playerPhaserLinger;
}

void script_Phasers(Tactical *src, Tactical *tgt) {
    // target is alive and not the user
    if (tgt->hp > -1 && tgt != src) {
        if (debug) printf("Phasers fired by %s at %s.\n", src->name, tgt->name);
        tgt->hp = tgt->hp - abilities[1].attr1; // reduce target's HP by attr1 value, which in this case will represent damage
        if (tgt->hp < 0) {
            createObject("Explosion", 9, 1, tgt->pos, tgt->dst, 180, 59, 750);
        }
        playerPhaserLinger = getMissionClock() + 500;
    }
}

void script_PhotonTorpedo(Tactical *src, Tactical *tgt) {
    if (debug) printf("Photon Torpedo fired.\n");
    createObject("Photon Torpedo", 3, src->team, rotatePointAroundCenter(src->emitter, src->pos, src->heading), src->dst, src->heading, src->momentum, 0);
    createObject("Photon Torpedo", 3, src->team, rotatePointAroundCenter(src->emitter, src->pos, src->heading), src->dst, src->heading+12, src->momentum, 0);
    createObject("Photon Torpedo", 3, src->team, rotatePointAroundCenter(src->emitter, src->pos, src->heading), src->dst, src->heading+24, src->momentum, 0);
    createObject("Photon Torpedo", 3, src->team, rotatePointAroundCenter(src->emitter, src->pos, src->heading), src->dst, src->heading-12, src->momentum, 0);
    createObject("Photon Torpedo", 3, src->team, rotatePointAroundCenter(src->emitter, src->pos, src->heading), src->dst, src->heading-24, src->momentum, 0);
}

void script_QuantumTorpedo(Tactical *src, Tactical *tgt) {
    if (debug) printf("Quantum Torpedo fired.\n");
    createObject("Quantum Torpedo", 4, src->team, rotatePointAroundCenter(src->emitter, src->pos, src->heading), src->dst, src->heading, src->momentum, 0);
    if (playerSalvoCount == 0) {
        playerSalvoCount = 3;
    }
    playerSalvoCount--;
    if (playerSalvoCount > 0) {
        playerSalvoNext = getMissionClock() + 300;
    }
}

void script_PhaseCloak(Tactical *src, Tactical *tgt) {
    src->invincibleUntil = getMissionClock() + abilities[4].attr1; // sets invincible for a period of milliseconds equal to attr1.
    printf("User is invulnerable for %.1f seconds.\n", abilities[4].attr1 / 1000.0);
}

void script_GreenTorpedo(Tactical *src, Tactical *tgt) {
    createObject("Photon Torpedo", 8, src->team, rotatePointAroundCenter(src->emitter, src->pos, src->heading), src->dst, src->heading, src->momentum, 0);
}

void script_GreenTorpedoAimed(Tactical *src, Tactical *tgt) {
    createObject("Photon Torpedo",
                 8,
                 src->team,
                 rotatePointAroundCenter(src->emitter, src->pos, src->heading),
                 src->dst,
                 getHeadingToLocation(src->pos, tgt->pos),
                 src->momentum,
                 0);
}
