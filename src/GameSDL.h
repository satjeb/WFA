#ifndef GAME_SDL_HEADER_GUARD
#define GAME_SDL_HEADER_GUARD

#include "GameData.h"

#include <stdbool.h>

bool init_GameSDL();
bool createSDL();
bool createRenderer();
bool createIMG();
bool createTTF();
bool setAssets();
bool closeSDL();

SDL_Texture* getTexture(Asset* a);
void freeAsset(Asset* a);
void renderObject(Tactical* obj);
void renderTextAsset(Asset* a, char* text, int size, SDL_Color color, Point pos);
void renderPaused();
void renderFPS(int currentFPS);
void renderShieldsDown();
void renderAbility1cd(Tactical *obj);
void renderAbility2cd(Tactical *obj);
void renderAbility3cd(Tactical *obj);
void renderAbility4cd(Tactical *obj);
void renderShieldsStatus(Tactical *obj);
void renderGameOver();
void renderRestartAdvice();
void renderTargetingReticle(Tactical *obj);
void renderPhaser(Point a, Point b);
void renderGreenLine(Point a, Point b);
void renderRedLine(Point a, Point b);

void clearScreen();
void presentScreen();

// starfield functions
void initStarfield();
void renderStarfield(int scroll);
void freeStarfield();

#endif // GAME_SDL_HEADER_GUARD
