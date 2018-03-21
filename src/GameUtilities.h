#ifndef GAMEUTILITIES_HEADER_GUARD
#define GAMEUTILITIES_HEADER_GUARD

#include <SDL.h>
#include <stdbool.h>

// Universally useful things yet not mess making things
// This file contains enums, structs, and any functions which return generic values, and does not manipulate data directly.

// ENUMS

// Enum for movement bit flags
enum Movement {
    MoveForward = 0x01,
    MoveBackward = 0x02,
    TurnLeft = 0x04,
    TurnRight = 0x08,
};

// Enum for patterns used when an object hits the edge of the screen.
enum EdgePatterns {
    GAME_EDGE_NONE,
    GAME_EDGE_BLOCKS,
    GAME_EDGE_RESPAWN,
    GAME_EDGE_RESPAWN_X_RANDOM,
    GAME_EDGE_RESPAWN_Y_RANDOM,
    GAME_EDGE_RESPAWN_XY_RANDOM,
    GAME_EDGE_RESPAWN_FULL_RANDOM,
    GAME_EDGE_LOOP,
    GAME_EDGE_EXTENDED,
    GAME_EDGE_LOCKED_BOTTOM,
};

// STRUCTS

// Stores a Point on a two-dimensional screen
typedef struct Point {
    float x;
    float y;
} Point;

// Stores material pertinent to a graphical asset
typedef struct Asset {
    char path[100];
    SDL_Texture* texture;
    int h;
    int w;
    int loadFailures;
} Asset;

// Stats stores variables which are static to a type of object, like their max HP, top speed, and acceleration
typedef struct Stats {
    char shipClass[40];
    Asset* asset;
    float assetScale;
    float speed;
    float turn;
    float accel;
    int maxHP;
    int regenHP;
    int regenCycleRate;
    int ability1;
    int ability2;
    int ability3;
    int ability4;
    int defaultCollision;
    int defaultEdgePattern;
    int dmg;
    int script;
} Stats;

typedef struct Ability {
    char name[40];
    void (*script)();
    int relatedObject;
    int cooldown;
    int attr1;
    int attr2;
    int attr3;
    int attr4;
    int attr5;
} Ability;

// Tactical stores variables related to specific object instances, such as their current HP, trajectory, and ability use
typedef struct Tactical {
    char name[40];
    Stats* stats;
    Asset* tag;
    float tagScale;
    int team;
    Point pos;
    Point dst;
    float momentum;
    float heading;
    int hp;
    int lastFrameHp;
    int nextRegenCycle;
    int ability1avail;
    int ability2avail;
    int ability3avail;
    int ability4avail;
    int invincibleUntil;
    Point emitter;
    unsigned char movement; // bit flag implementation
    unsigned int lifespan;
} Tactical;

// MissionEvent stores variables associated with a single event from a mission, such as when it should execute and what it should do.
typedef struct MissionEvent {
    int time;
    int eventType;
    int objectType;
    int team;
    Point pos;
    Point dst;
    float heading;
    float momentum;
    int lifespan;
} MissionEvent;

// FUNCTIONS
void setMissionClock(unsigned int now);
unsigned int getMissionClock();
int getScreenWidth();
int getScreenHeight();
float degreesToRadians(float degrees);
Point rotatePointAroundCenter(Point a, Point center, float rotation);
char* strtrim(char* str);
float getHeadingToLocation(Point org, Point dst);
float getSlope(Point pA, Point pB);
float getYInt(Point p, float slope);
float getDistanceBetweenPointAndLine(Point p, float slope, float yInt);
float getDistanceBetweenTwoPoints(Point a, Point b);
bool isNumberBetweenValues(float x, float a, float b);
bool isPointInShape(Point a, Point s[4]);
bool isIntersected(Point lineA[2], Point lineB[2]);
bool validPoint(Point* p, int edgePattern);

#endif // GAMEUTILITIES
