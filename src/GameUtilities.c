#include "GameUtilities.h"

#include <math.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <stdlib.h>

const int SCREEN_WIDTH = 920;
const int SCREEN_HEIGHT = 750;

unsigned int missionClock;

int getScreenWidth() {
    return SCREEN_WIDTH;
}

int getScreenHeight() {
    return SCREEN_HEIGHT;
}

void setMissionClock(unsigned int now) {
    missionClock = now;
}

unsigned int getMissionClock() {
    return missionClock; // needs to take into account time the game is paused.
}

// converts degrees to radians surprisingly
float degreesToRadians(float degrees) {
    return (degrees / 360 * 2 * M_PI);
}

// rotate point with offset from given point around said given point by given value (takes degrees), and return new value
Point rotatePointAroundCenter(Point a, Point center, float rotation) {
    rotation = degreesToRadians(rotation); // convert rotation to radians
    return (Point) { center.x + a.x * cos(rotation) - a.y * sin(rotation),
                     center.y + a.y * cos(rotation) + a.x * sin(rotation)};
}

// trims whitespace from beginning and end of a string of characters
char* strtrim(char* str) {
    if (str != NULL) {
        int i = 0;
        while (isspace(str[i]) && str[i] != '\0') {
            i++;
        }
        char* substr = &str[i];

        i = strlen(str) - 1;
        while (isspace(str[i]) && i >= 0) {
            i--;
        }
        str[i+1] = 0;

        return substr;
    } else {
        return NULL; // Nothing to trim gets nothing back
    }
}

// Returns the heading needed for Point org to be pointed at Point dst in degrees.
float getHeadingToLocation(Point org, Point dst) {
    return atan((dst.y - org.y) / (dst.x - org.x))*180/M_PI + 90 + (dst.x - org.x >= 0 ? 0 : 180);
}

// get slope between two Points
float getSlope(Point pA, Point pB) {
    if (pB.x == pA.x) {
        return FLT_MAX / 4.0; // close enough, needed to avoid divide by zero.
    } else {
        return (pB.y - pA.y) / (pB.x - pA.x);
    }
}

// get Y intercept from a Point and a slope
float getYInt(Point p, float slope) {
    return p.y - slope*p.x;
}

// Note that this does not return it in absolute value form intentionally to preserve which side of the line the point is on.
// If you only care about the distance, be sure to abs() the value before any math
// This version takes slope and yInt
float getDistanceBetweenPointAndLine(Point p, float slope, float yInt) {
    return (slope * p.x - p.y + yInt) / sqrt(pow(slope,2.0) + pow(-1.0,2.0));
}

// returns the distance between Point A and Point B
float getDistanceBetweenTwoPoints(Point a, Point b) {
    return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2));
}

// returns true if x is between values a and b (inclusive), regardless of the order of a and b.
bool isNumberBetweenValues(float x, float a, float b) {
    bool truth = false;
    if (a < b) {
        if (a <= x && x <= b) {
            truth = true;
        }
    } else {
        if (b <= x && x <= a) {
            truth = true;
        }
    }
    return truth;
}

// Checks to see if a point is inside the given shape
// Note that it does not matter which way you draw it - The first line will determine which side the others are evaluated on
// For the sake of clarity, if the point falls directly on the line it is treated as within the shape
bool isPointInShape(Point a, Point s[4]) {
    int numOfPoints = 4;
    float side = 0.0;

    for (int i = 0; i < numOfPoints; i++) {
        // determine line
        float slope = getSlope(s[i], s[(i+1) % numOfPoints]);
        float yInt = getYInt(s[i], slope);
        float dist = getDistanceBetweenPointAndLine(a, slope, yInt);

        if (side == 0.0) {
            side = dist / abs(dist); // Just want the sign. Will continue if point is on first line until it gets a side.
        } else {
            if ((dist < 0 && side > 0) ||
                (dist > 0 && side < 0)) {
                    return false;
            }
        }
    }
    //return true;
    return false; // needs work
}

// determines if two lines segments (given by two Points each) intersect each other.
bool isIntersected(Point lineA[2], Point lineB[2]) {
    bool truth = false;
    // handle vertical line special cases
    // if both lines are vertical
    if (lineB[0].x == lineB[1].x && lineA[0].x == lineA[1].x) {
        if (lineA[0].x == lineB[0].x) truth = true;
    }
    else if (lineA[0].x == lineA[1].x &&
             isNumberBetweenValues(lineA[0].x, lineB[0].x, lineB[1].x)) { // only line A is vertical and lineB can possibly intersect
        float slopeB = getSlope(lineB[0], lineB[1]);
        float yIntB = getYInt(lineB[0], slopeB);
        float aIntersected = slopeB * lineA[0].x + yIntB;
        if (isNumberBetweenValues(aIntersected, lineA[0].y, lineA[1].y)) {
            truth = true;
        }
    }
    else if (lineB[0].x == lineB[1].x &&
             isNumberBetweenValues(lineB[0].x, lineA[0].x, lineA[1].x)) { // only line B is vertical and lineA can possibly intersect
        float slopeA = getSlope(lineA[0], lineA[1]);
        float yIntA = getYInt(lineA[0], slopeA);
        float bIntersected = slopeA * lineB[0].x + yIntA;
        if (isNumberBetweenValues(bIntersected, lineB[0].y, lineB[1].y)) {
            truth = true;
        }
    }
    else { // normal cases
        float slopeA = getSlope(lineA[0], lineA[1]);
        float yIntA = getYInt(lineA[0], slopeA);
        float slopeB = getSlope(lineB[0], lineB[1]);
        float yIntB = getYInt(lineB[0], slopeB);

        float x = (yIntB - yIntA) / (slopeA - slopeB); // x-coordinate where continuous lines meet, now find if it's on the segments
        // yeah I only need X

        // if x is valid on the line segments for both segments, then they are intersected.
        if (isNumberBetweenValues(x, lineA[0].x, lineA[1].x) &&
            isNumberBetweenValues(x, lineB[0].x, lineB[1].x)) {
            truth = true;
        }
    }
    return truth;
}

// This takes a pointer to a Point and sets its values to something valid.
// If it is unable to set it to something valid (typically due to rules), then it returns false.
bool validPoint(Point* p, int edgePattern) {
    bool valid = true;
    switch (edgePattern) {
    case GAME_EDGE_NONE: // return null if invalid, indicating death of object, buffer room so things are well off screen before they pop out of existence.
        if (p->x > SCREEN_WIDTH + 250 ||
            p->x < - 250 ||
            p->y > SCREEN_HEIGHT + 250 ||
            p->y < - 250)
        {
            valid = false;
        }
        break;
    case GAME_EDGE_EXTENDED: // as NONE but with a wider acceptance range.
        if (p->x > SCREEN_WIDTH + 1000 ||
            p->x < - 1000 ||
            p->y > SCREEN_HEIGHT + 1000 ||
            p->y < - 1000)
        {
            valid = false;
        }
        break;
    case GAME_EDGE_BLOCKS: // Bounce off side
        if (p->x < 0) p->x = 0;
        if (p->y < 0) p->y = 0;
        if (p->x > SCREEN_WIDTH) p->x = SCREEN_WIDTH;
        if (p->y > SCREEN_HEIGHT) p->y = SCREEN_HEIGHT;
        break;
    case GAME_EDGE_RESPAWN: // TBD: respawn at original position
        break;
    case GAME_EDGE_RESPAWN_X_RANDOM: // TBD: respawn at random X position, Y top (probably need a bottom)
        break;
    case GAME_EDGE_RESPAWN_Y_RANDOM: // TBD: respawn at random Y position, X left (really need a right)
        break;
    case GAME_EDGE_RESPAWN_XY_RANDOM: // TBD: respawn at random left or top
        break;
    case GAME_EDGE_RESPAWN_FULL_RANDOM: // TBD: respawn ANYWHERE
        break;
    case GAME_EDGE_LOOP: // loop around, with fudge room to avoid clipping
        if (p->x > SCREEN_WIDTH + 50) p->x = -50;
        if (p->x < -50) p->x = SCREEN_WIDTH + 50;
        if (p->y > SCREEN_HEIGHT + 50) p->y = -50;
        if (p->y < -50) p->y = SCREEN_HEIGHT + 50;
        break;
    case GAME_EDGE_LOCKED_BOTTOM: // Y-axis is locked near the bottom of the screen.
        p->y = SCREEN_HEIGHT - 50;
        if (p->x < 0) p->x = 0;
        if (p->x > SCREEN_WIDTH) p->x = SCREEN_WIDTH;
    default:
        break;
    }

    return valid;
}
