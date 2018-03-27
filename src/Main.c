#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <SDL.h>
#include <string.h>

#include "GameLoad.h"
#include "GameData.h"
#include "GameUtilities.h"
#include "GameSDL.h"

int main(int argc, char* args[]) {
    printf("Initializing game.\n");

    int setFPS = 60;
    bool showFPS = false;

    clock_t gameStart = clock(); // set game start

    // Load game resources to memory for the given mission file and prepare to run.
    char missionFile[30] = "Mission_001.ini";
    if (!gameInit(missionFile)) {
        printf("Failed to load game files when attempting to load mission: %s, unable to continue.\n", missionFile);
        return 1; // Failure to initialize means abort
    } else {
        SDL_Event e; // variable for player input

        Tactical* target = getTactical(0); // pointer to the player's current primary target (self treated as no target)
        Tactical* player = getTactical(0); // pointer to the player for convenience.

        // phaser beam calculations to keep the beam vfx going when something explodes.
        Point phasePoint = {-1,-1};

        // game flow variables
        bool quit = false;
        bool gg = false;
        bool restart = false;
        bool paused = false;

        // variables for tracking frames
        clock_t frameStart = 0;
        int highestFrameRenderTime = 0;
        int framesDipped = 0;
        int starfieldScroll = 0; // used for starfield scrolling.
        initStarfield();

        // primary frame loop
        while (!quit) {
            frameStart = clock(); // set clock for beginning of this frame.

            if (restart) {
                restartMission();
                gg = false;
                paused = false;
                restart = false;
                player->invincibleUntil = 0;
                setPhaserLinger(0);
            }

            if (!paused && !gg) {
                setMissionClock(getMissionClock() + getFrameTime());
                starfieldScroll++;
            }


            while (SDL_PollEvent(&e) != 0) {
                //User requests quit
                if (e.type == SDL_QUIT) {
                    quit = true;
                } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    Point p = (Point){x,y};
                    target = getClosestObjectToPoint(p);
                } else if (e.type == SDL_KEYDOWN) {
                    switch(e.key.keysym.sym) {
                        case SDLK_w:
                            player->movement = player->movement | MoveForward; // flips moving forward on
                            break;
                        case SDLK_a:
                            player->movement = player->movement | TurnLeft;
                            break;
                        case SDLK_s:
                            player->movement = player->movement | MoveBackward;
                            break;
                        case SDLK_d:
                            player->movement = player->movement | TurnRight;
                            break;
                        case SDLK_SPACE:
                            if (!gg) {
                                paused = !paused; // toggle
                                if (debug) printf("GAME %sPAUSED.\n", paused ? "" : "UN");
                            } else {
                                restart = true;
                            }
                            break;
                        case SDLK_1:
                            if (!paused) doAbility(player, target, 1);
                            break;
                        case SDLK_2:
                            if (!paused) doAbility(player, target, 2);
                            break;
                        case SDLK_3:
                            if (!paused) doAbility(player, target, 3);
                            break;
                        case SDLK_4:
                            if (!paused) doAbility(player, target, 4);
                            break;
                        default:
                            break;
                    }
                } else if (e.type == SDL_KEYUP) {
                    switch(e.key.keysym.sym) {
                        case SDLK_w:
                            player->movement &= ~MoveForward; // flips moving forward off
                            break;
                        case SDLK_a:
                            player->movement &= ~TurnLeft;
                            break;
                        case SDLK_s:
                            player->movement &= ~MoveBackward;
                            break;
                        case SDLK_d:
                            player->movement &= ~TurnRight;
                            break;
                        default:
                            break;
                    }
                }
            }

            // check for additional salvos the player is in the process of firing.
            salvoCheck(player, target);

            // handle mission events and new movement
            if (!paused) {
                // checks mission script for new events
                runMission(getMissionClock());

                moveAllObjects();
            }

            clearScreen();

            // Render screen in layers starting with least important (the lowest layer) to most important (top most) in terms of gameplay
            // First render background.
            renderStarfield(starfieldScroll);

            // render all non-player objects.
            for (int i = 0; i < getNumOfObjects(); i++) {
                Tactical* t = getTactical(i);
                if (t != player && t->hp >= 0) {
                    renderObject(getTactical(i));
                }
            }

            // render player phaser fire, beam weapons don't have objects (yet).
            // check to see if the phasers should still be rendered.
            if (getPhaserLinger() > getMissionClock()) {
                Point mainPhasers = rotatePointAroundCenter(player->emitter, player->pos, player->heading);
                if (phasePoint.x >= 0) { // if phasePoint is defined, just use that, lest we end up with new objects or new targets.
                    renderPhaser(mainPhasers, phasePoint);
                } else if (target->hp < 0) { // if target is destroyed, set the phasePoint where the beam will finish shooting.
                    phasePoint = target->pos;
                    renderPhaser(mainPhasers, phasePoint);
                } else { // normally use the target's position if the special cases above do not trigger.
                    renderPhaser(mainPhasers, target->pos);
                }
            } else {
                phasePoint.x = -1;
                phasePoint.y = -1;
            }

            // render player ship last, to ensure it is always on top of everything else.
            renderObject(player);

            // detect collisions, delayed this long for line rendering for the hit boxes, strictly for debugging purposes.
            // handle collisions - done last for the convenience of drawing the hit lines on top of everything else when debugging.
            // If the player isn't under invincible conditions, check for collisions with objects.
            if (!paused) {
                // Start by searching for objects on team 1, this is the player's team. We only care about living objects that are not invincible.
                for (int i = 0; i < getNumOfObjects(); i++) {
                    Tactical* ti = getTactical(i);
                    if ((ti->team == 1 || strcmp(ti->stats->shipClass, "Green Torpedo") == 0) &&
                        ti->hp > -1 &&
                        ti->invincibleUntil <= getMissionClock())
                    {
                        // Next go through the list and look for objects not on Team 1. Same rules apply. If found, check for collision between the two objects.
                        for (int j = 0; j < getNumOfObjects(); j++) {
                            // at the last minute I decided klingon torpedoes should be able to hit the rocks for fun cover mechanics
                            // the above checks if the object is a Klingon Torpedo, if so, it will be team 2, thus we check for collision with team 3 (the asteroids)
                            Tactical* tj = getTactical(j);
                            if (((tj->team != 1 && ti->team == 1) || (tj->team == 3 && ti->team == 2)) &&
                                tj->hp > -1 &&
                                tj->invincibleUntil <= getMissionClock())
                            {
                                // If a collision is found, damage both according to their dmg and hp stats
                                if (detectCollision(ti, tj)) {
                                    if (debug) printf("%d: Collision detected between %s (object %d) and player.\n", getMissionClock(), ti->name, i);
                                    ti->hp = ti->hp - tj->stats->dmg;
                                    tj->hp = tj->hp - ti->stats->dmg;
                                    if ((ti->hp < 0 && i != 0) || (tj->hp < 0 && j != 0)) {
                                        printf("An explosion\n");
                                        // make an explosion!
                                        // position uses the other object if the first is the player, otherwise the allied object, since it's probably a torpedo
                                        createObject("Explosion", 9, 1, (i == 0? tj->pos : ti->pos), tj->dst, 180, 59, 750);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // update shield status of the player based on the above results.
            if (updateShieldStatus(player) < 0 && !gg) {
                printf("gg.\n");
                gg = true;
                paused = true;
            }

            // Render UI elements such as player's target, cooldowns and shield indicator
            // render player's current target
            if (target->hp < 0) {
                target = player; // treated as no target!
            }
            if (target != player) {
                renderTargetingReticle(target);
            }
            // render cooldowns
            renderAbility1cd(player);
            renderAbility2cd(player);
            renderAbility3cd(player);
            renderAbility4cd(player);
            // render shields
            renderShieldsStatus(player);

            // Display the giant Shields Down warning when shields hit zero, to let the player know they're one hit from dead.
            if (player->hp == 0 && getMissionClock() / 500 % 2 == 0) {
                renderShieldsDown();
            }

            // pause indicator
            if (paused && !gg) {
                renderPaused();
            }

            if (gg) {
                renderGameOver();
                renderRestartAdvice();
            }

            // render FPS if indicated
            if (showFPS) {
                renderFPS(1000 / (getFrameTime() > 0 ? getFrameTime() : 1)); // let's avoid divide by zero when rendering FPS
            }

            // Finally, present the fully rendered screen.
            presentScreen();

            player->lastFrameHp = player->hp; // sets new lastFrameHp for hit effects and shield recharge triggering.

            // handle frame logistics
            // get time it took to render this frame
            int ms = (clock() - frameStart) / (CLOCKS_PER_SEC / 1000); // Just in case CLOCKS_PER_SEC is ever different, I want to always get the time in ms
            int delay = getFrameTime() - ms; // compute optimized delay based on desired frame rate.
            // if delay exceeds our FPS capabilities, make a realistic expectation for the next one. Will still cause a laggy frame, but hopefully not more.
            // That said this does cause extra lag when large assets are loaded
            if (delay < 0) {
                setFrameTime(ms);
                delay = 0; // minimize this frame's lag by setting its delay to 0.
                if (debug) {
                    framesDipped++;
                    //printf("Frame at %d ms dipped to %d FPS out of a desired %d FPS.\n", getGameTime(), ms > 0 ? (1000 / ms) : 9999, setFPS);
                    printf("Calculated delay was %d ms, frame took %d ms to render. New frame time: %d\n", delay, ms, getFrameTime());
                }
            } else if (delay > 0 && ms > 1000 / setFPS) { // we have unused render time but not enough to reach our desired FPS
                setFrameTime(ms);
            } else { // final case: we have plenty of time to render at our desired FPS
                setFrameTime(1000 / setFPS);
            }

            // lazy hack until I figure out what to do about window moving and resizing, lets me keep my adaptive FPS without the window moving causing excessive delay
            if (getFrameTime() > 1000) setFrameTime(1000);

            // debug record keeping
            if (debug && ms > highestFrameRenderTime) highestFrameRenderTime = ms;

            SDL_Delay(delay);
        }

        printf("END SIMULTATION\n");
        printf("Elapsed time: %dms\n", (int)(clock()-gameStart));
        if (debug) {
            printf("Longest frame render: %d ms\n", highestFrameRenderTime);
            printf("Number of frames which exceeded set FPS: %d\n", framesDipped);
        }

        freeStarfield();
        gameClose();
    }

    return 0;
}

