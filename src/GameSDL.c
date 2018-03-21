#include "GameSDL.h"
/*
    GameSDL.c
    Performs functions related to SDL, basically graphics and rendering.
*/
#include "GameUtilities.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

// SDL globals
SDL_Window* mainWindow = NULL;
SDL_Surface* mainScreenSurf = NULL;
SDL_Renderer* mainRenderer = NULL; // window renderer
TTF_Font *mainFont = NULL; //text font

const SDL_Color defaultTextColor = {142,224,250};
Asset shields;
Asset ability1cd;
Asset ability2cd;
Asset ability3cd;
Asset ability4cd;
Asset targeting;
Asset gameOver;
Asset toRestart;

// Paused texture assets
SDL_Texture* pausedTexture = NULL;
int pausedWidth = 0;
int pausedHeight = 0;
// FPS assets
SDL_Texture* fpsTexture = NULL;
int fpsTextureWidth = 0;
int fpsTextureHeight = 0;
int lastFPS = 60;
// Shields Down UI assets
SDL_Texture* shieldsDownTexture = NULL;
int shieldsDownHeight = 0;
int shieldsDownWidth = 0;
// Game Over UI assets
SDL_Texture* gameOverTexture = NULL;
int gameOverHeight = 0;
int gameOverWidth = 0;
SDL_Texture* restartTexture = NULL;
int restartHeight = 0;
int restartWidth = 0;

const int numOfStars = 2900; // number of background stars.
int *starfield; // array of values

// Initialize chain through a single if. Add more to initialize by adding procedures here to the if.
// Currently creates SDL, renderer, and IMG.
bool init_GameSDL() {
    if (createSDL() &&
        createRenderer() &&
        createIMG() &&
        createTTF()) {
            return true;
    } else {
        printf("init_GameSDL() failed.\n");
        return false;
    }
}

// Initialize SDL and set up main window
bool createSDL() {
    bool success = true;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL failed to initialize. SDL_Error: %s\n", SDL_GetError());
		success = false;
    } else {
        mainWindow = SDL_CreateWindow("WFA", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, getScreenWidth(), getScreenHeight(), SDL_WINDOW_SHOWN);
        if (mainWindow == NULL) {
            printf("Window failed to initialize. SDL_Error: %s\n", SDL_GetError());
            success = false;
        }
    }

    return success;
}

// Create the renderer
bool createRenderer() {
    bool success = true;

    mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);
    if (mainRenderer != NULL) {
        // set renderer color

    } else {
        success = false;
        printf("Failed to create renderer. SDL Error:%s", SDL_GetError());
    }

    return success;
}

// setup IMG so we can use formats other than bitmap. In this case PNGs.
bool createIMG() {
    bool success = true;

    int imgFlags = IMG_INIT_PNG;
    // IMG_Init should return the same flags you gave it, so we check them bitwise to see if they're the same to judge success
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("IMG failed to initialize with IMG_Error: %s\n", IMG_GetError());
        success = false;
    }

    return success;
}

// setup TTF, which supports rendering text from fonts
bool createTTF() {
    bool success = true;

    if ( TTF_Init() == -1) {
        printf("Failed to initialize TTF with SDL_ttf Error: %s\n", TTF_GetError());
        success = false;
    }
    return success;
}

// set assets
bool setAssets() {
    strcpy(targeting.path, "assets\\targeting.png");
    return true;
}

// Explicitly close all assets and free resources used.
bool closeSDL() {
    SDL_DestroyRenderer(mainRenderer);
    SDL_DestroyWindow(mainWindow);
    SDL_FreeSurface(mainScreenSurf);

    SDL_DestroyTexture(shields.texture);
    SDL_DestroyTexture(ability1cd.texture);
    SDL_DestroyTexture(ability2cd.texture);
    SDL_DestroyTexture(ability3cd.texture);
    SDL_DestroyTexture(ability4cd.texture);
    SDL_DestroyTexture(targeting.texture);
    SDL_DestroyTexture(gameOver.texture);
    SDL_DestroyTexture(toRestart.texture);
    SDL_DestroyTexture(shieldsDownTexture);
    SDL_DestroyTexture(fpsTexture);
    SDL_DestroyTexture(pausedTexture);
    /*
    for (int i = 0; i < sizeof(assets)/sizeof(assets[0]); i++) {
        SDL_DestroyTexture(assets[i].texture);
    }
    */

    TTF_CloseFont(mainFont);

    TTF_Quit(); // Close TTF
    IMG_Quit(); // Close IMG
    SDL_Quit(); // Close SDL

    return true;
}

// Retrieves or generates a texture for a given asset.
SDL_Texture* getTexture(Asset* a) {
    SDL_Texture* loadedTexture = NULL;

    if (a->texture != NULL) {
        loadedTexture = a->texture;
    } else if (a->loadFailures < 3) { // just to avoid excessive and doomed attempts
        printf("Loading asset fresh from source: %s\n", a->path);
        SDL_Surface* loadedSurface = IMG_Load(a->path);
        if (loadedSurface != NULL) {
            SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0, 0));
            loadedTexture = SDL_CreateTextureFromSurface(mainRenderer, loadedSurface);
            a->texture = loadedTexture;
            a->w = loadedSurface->w;
            a->h = loadedSurface->h;
            SDL_FreeSurface(loadedSurface);
            a->loadFailures = 0; // reset fail counter
        } else {
            printf("Failed to create texture from path: %s. SDL Error: %s\n", a->path, SDL_GetError());
            a->loadFailures++;
        }
    } else if (a->loadFailures == 3) {
        printf("Aborted loading of asset at path: %s (Too many failed attempts).\n", a->path);
        a->loadFailures++; // just to avoid spamming the log.
    }

    return loadedTexture;
}

// Frees Asset of its texture.
void freeAsset(Asset* a) {
    SDL_DestroyTexture(a->texture);
}

void renderObject(Tactical* obj) {
    SDL_Rect textureBounds = { obj->pos.x - obj->stats->asset->w / 2 * obj->stats->assetScale, obj->pos.y - obj->stats->asset->h / 2 * obj->stats->assetScale, obj->stats->asset->w * obj->stats->assetScale, obj->stats->asset->h * obj->stats->assetScale };
    SDL_RenderCopyEx(mainRenderer, getTexture(obj->stats->asset), 0, &textureBounds, obj->heading, 0, SDL_FLIP_NONE);
}

// renders a text asset at the given location, only rendering a new texture if the content has changed
void renderTextAsset(Asset* a, char* text, int size, SDL_Color color, Point pos) {
    // render new if no texture exists or text has changed (I have requisitioned path for that task)
    if (a->texture == NULL || strcmp(a->path, text) != 0) {
        if (debug) printf("Rendering new text texture. Old text was %s, new text is %s.\n", a->path, text);
        TTF_Font* font = TTF_OpenFont("fonts\\Context_Ultra_Condensed.ttf", size);
        SDL_Surface* surf = TTF_RenderText_Solid(font, text, color);
        a->texture = SDL_CreateTextureFromSurface(mainRenderer, surf);
        a->w = surf->w;
        a->h = surf->h;
        strcpy(a->path, text);
        SDL_FreeSurface(surf);
        TTF_CloseFont(font);
    }
    SDL_Rect space = { pos.x, pos.y, a->w, a->h};
    SDL_RenderCopy(mainRenderer, a->texture, 0, &space);
}

// Render the word paused over the top of the screen.
void renderPaused() {
    if (pausedTexture == NULL) {
        TTF_Font *pausedFont = TTF_OpenFont("fonts\\Context_Ultra_Condensed.ttf", 47);
        SDL_Color textColor = {142,224,250};
        char str[7] = "PAUSED";
        SDL_Surface* pausedSurf = TTF_RenderText_Solid(pausedFont, str, textColor);
        pausedTexture = SDL_CreateTextureFromSurface(mainRenderer, pausedSurf);
        pausedWidth = pausedSurf->w;
        pausedHeight = pausedSurf->h;
        SDL_FreeSurface(pausedSurf);
        TTF_CloseFont(pausedFont);
    }
    SDL_Rect topLevel = { (getScreenWidth() - pausedWidth)/2, (getScreenHeight() - pausedHeight)/2, pausedWidth, pausedHeight };
    SDL_RenderCopy(mainRenderer, pausedTexture, 0, &topLevel);
}

// Render the current FPS in the top left corner on demand
void renderFPS(int currentFPS) {
    if (fpsTexture == NULL || currentFPS != lastFPS) {
        TTF_Font *fpsFont = TTF_OpenFont("fonts\\Context_Ultra_Condensed.ttf", 36);
        SDL_Color textColor = {255,255,0};
        char str[9];
        if (currentFPS > 9999) {
            currentFPS = 9999;
        }
        sprintf(str, "%d FPS", currentFPS);
        SDL_Surface* fpsSurf = TTF_RenderText_Solid(fpsFont, str, textColor);
        fpsTexture = SDL_CreateTextureFromSurface(mainRenderer, fpsSurf);
        fpsTextureWidth = fpsSurf->w;
        fpsTextureHeight = fpsSurf->h;
        SDL_FreeSurface(fpsSurf);
        TTF_CloseFont(fpsFont);
        lastFPS = currentFPS;
    }
    SDL_Rect fpsRect = { 0, 0, fpsTextureWidth, fpsTextureHeight };
    SDL_RenderCopy(mainRenderer, fpsTexture, 0, &fpsRect);
}

// Render shields down UI overlay
void renderShieldsDown() {
    if (shieldsDownTexture == NULL) {
        TTF_Font *shieldsDownFont = TTF_OpenFont("fonts\\Context_Ultra_Condensed.ttf", 58);
        SDL_Color textColor = {255,0,0};
        char str[24] = "S H I E L D S   D O W N";
        SDL_Surface* shieldsDownSurf = TTF_RenderText_Solid(shieldsDownFont, str, textColor);
        shieldsDownTexture = SDL_CreateTextureFromSurface(mainRenderer, shieldsDownSurf);
        shieldsDownWidth = shieldsDownSurf->w;
        shieldsDownHeight = shieldsDownSurf->h;
        SDL_FreeSurface(shieldsDownSurf);
        TTF_CloseFont(shieldsDownFont);
    }
    SDL_Rect sdRect = { getScreenWidth() / 2 - shieldsDownWidth / 2, getScreenHeight() / 2 - shieldsDownHeight / 2, shieldsDownWidth, shieldsDownHeight };
    SDL_RenderCopy(mainRenderer, shieldsDownTexture, 0, &sdRect);
}

// Render game over UI overlay
void renderGameOver() {
    if (gameOverTexture == NULL) {
        TTF_Font *gameOverFont = TTF_OpenFont("fonts\\Context_Ultra_Condensed.ttf", 75);
        SDL_Color textColor = {255,0,0};
        char str[17] = "G A M E  O V E R";
        SDL_Surface* gameOverSurf = TTF_RenderText_Solid(gameOverFont, str, textColor);
        gameOverTexture = SDL_CreateTextureFromSurface(mainRenderer, gameOverSurf);
        gameOverWidth = gameOverSurf->w;
        gameOverHeight = gameOverSurf->h;
        SDL_FreeSurface(gameOverSurf);
        TTF_CloseFont(gameOverFont);
    }
    SDL_Rect sdRect = { getScreenWidth() / 2 - gameOverWidth / 2, getScreenHeight() / 3 - gameOverHeight / 2, gameOverWidth, gameOverHeight };
    SDL_RenderCopy(mainRenderer, gameOverTexture, 0, &sdRect);
}

void renderRestartAdvice() {
    if (restartTexture == NULL) {
        TTF_Font *restartFont = TTF_OpenFont("fonts\\Context_Ultra_Condensed.ttf", 47);
        SDL_Color textColor = {255,0,0};
        char str[23] = "PRESS SPACE TO RESTART";
        SDL_Surface* restartSurf = TTF_RenderText_Solid(restartFont, str, textColor);
        restartTexture = SDL_CreateTextureFromSurface(mainRenderer, restartSurf);
        restartWidth = restartSurf->w;
        restartHeight = restartSurf->h;
        SDL_FreeSurface(restartSurf);
        TTF_CloseFont(restartFont);
    }
    SDL_Rect sdRect = { getScreenWidth() / 2 - restartWidth / 2, getScreenHeight() * 2 / 3 - restartHeight * 2 / 3, restartWidth, restartHeight };
    SDL_RenderCopy(mainRenderer, restartTexture, 0, &sdRect);
}

// Render UI element for Ability 1 cooldown for given object
void renderAbility1cd(Tactical *obj) {
    SDL_Color textColor = defaultTextColor;
    char strBuf[9];
    if (obj->ability1avail > getMissionClock()) {
        textColor.r = 255;
        textColor.g = 255;
        textColor.b = 0;
        sprintf(strBuf, "1: %.1fs", (obj->ability1avail - getMissionClock()) / 1000.0);
    } else {
        strcpy(strBuf, "1: RDY");
    }
    renderTextAsset(&ability1cd, strBuf, 24, textColor, (Point){obj->pos.x - 120, obj->pos.y - 40});
}

// Render UI element for Ability 2 cooldown for given object
void renderAbility2cd(Tactical *obj) {
    SDL_Color textColor = defaultTextColor;
    char strBuf[9];
    if (obj->ability2avail > getMissionClock()) {
        textColor.r = 255;
        textColor.g = 255;
        textColor.b = 0;
        sprintf(strBuf, "2: %.1fs", (obj->ability2avail - getMissionClock()) / 1000.0);
    } else {
        strcpy(strBuf, "2: RDY");
    }
    renderTextAsset(&ability2cd, strBuf, 24, textColor, (Point){obj->pos.x - 120, obj->pos.y - 20});
}

// Render UI element for Ability 3 cooldown for given object
void renderAbility3cd(Tactical *obj) {
    SDL_Color textColor = defaultTextColor;
    char strBuf[9];
    if (obj->ability3avail > getMissionClock()) {
        textColor.r = 255;
        textColor.g = 255;
        textColor.b = 0;
        sprintf(strBuf, "3: %.1fs", (obj->ability3avail - getMissionClock()) / 1000.0);
    } else {
        strcpy(strBuf, "3: RDY");
    }
    renderTextAsset(&ability3cd, strBuf, 24, textColor, (Point){obj->pos.x - 120, obj->pos.y});
}

// Render UI element for Ability 4 cooldown for given object
void renderAbility4cd(Tactical *obj) {
    SDL_Color textColor = defaultTextColor;
    char strBuf[9];
    if (obj->invincibleUntil > getMissionClock()) {
        sprintf(strBuf, "4: %.1fs", (obj->invincibleUntil - getMissionClock()) / 1000.0);
    } else if (obj->ability4avail > getMissionClock()) {
        textColor.r = 255;
        textColor.g = 255;
        textColor.b = 0;
        sprintf(strBuf, "4: %.1fs", (obj->ability4avail - getMissionClock()) / 1000.0);
    } else {
        strcpy(strBuf, "4: RDY");
    }
    renderTextAsset(&ability4cd, strBuf, 24, textColor, (Point){obj->pos.x - 120, obj->pos.y + 20});
}

// Render UI element for Shields status for given object
void renderShieldsStatus(Tactical *obj) {
    SDL_Color textColor = defaultTextColor;
    char strBuf[8];
    if (obj->hp < obj->lastFrameHp || obj->hp <= obj->stats->maxHP / 4) {
        textColor.r = 255;
        textColor.g = 255;
        textColor.b = 0;
    }
    if (obj->hp == 0) {
        textColor.r = 255;
        textColor.g = 0;
        textColor.b = 0;
    }
    if (obj->hp > 0) {
        sprintf(strBuf, "S: %d%%", obj->hp / (obj->stats->maxHP / 100));
    } else {
        sprintf(strBuf, "S: %.1fs", (obj->nextRegenCycle - getMissionClock()) / 1000.0);
    }
    renderTextAsset(&shields, strBuf, 24, textColor, (Point){obj->pos.x - 120, obj->pos.y + 40});
}

// Render targeting reticle UI element for player's current primary target
void renderTargetingReticle(Tactical *obj) {
    SDL_Texture* text = getTexture(&targeting);
    SDL_Rect targetRect = { obj->pos.x - targeting.w / 2, obj->pos.y - targeting.h / 2, targeting.w, targeting.h };
    SDL_RenderCopy(mainRenderer, text, 0, &targetRect);
}

// Renders a phaser beam from Point a to Point b.
void renderPhaser(Point a, Point b) {
    SDL_SetRenderDrawColor(mainRenderer, 244, 115, 58, 255);
    SDL_RenderDrawLine(mainRenderer, a.x, a.y, b.x, b.y);
}

// Renders a simple green line
void renderGreenLine(Point a, Point b) {
    SDL_SetRenderDrawColor(mainRenderer, 0, 255, 0, 255);
    SDL_RenderDrawLine(mainRenderer, a.x, a.y, b.x, b.y);
}

// Renders a simple red line
void renderRedLine(Point a, Point b) {
    SDL_SetRenderDrawColor(mainRenderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(mainRenderer, a.x, a.y, b.x, b.y);
}

// clear screen for new rendering.
void clearScreen() {
    SDL_RenderClear(mainRenderer);
}

// present screen as rendered.
void presentScreen() {
    SDL_SetRenderDrawColor(mainRenderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderPresent(mainRenderer);
}

// allocates and randomly assigns values to the starfield data set.
void initStarfield() {
    srand(pow(time(0),2)); // initialize RNG
    starfield = malloc(numOfStars * 6 * sizeof(int));
    for (int i = 0; i < numOfStars * 6; i = i + 6) {
        starfield[i] = rand() % getScreenWidth();
        starfield[i+1] = rand() % getScreenHeight();
        starfield[i+2] = 255 - (rand() % 90); // color randomization
        starfield[i+3] = 255 - (rand() % 50);
        starfield[i+4] = 255 - (rand() % 90);
        starfield[i+5] = 255 - (rand() % 80);
    }
}

// Render the starfield using a perhaps needlessly complicated algorithm.
// The random points rendered above are rendered at points on the screen
// As starfieldScroll increases, the stars on the bottom-most row of pixels
// will rotate back to the top, though in reverse order from how they were
// before.
void renderStarfield(int starfieldScroll) {
    for (int j = 0; j < numOfStars * 6; j = j + 6) {
        SDL_SetRenderDrawColor(mainRenderer, starfield[j+2], starfield[j+3], starfield[j+4], starfield[j+5]);
        SDL_RenderDrawPoint(mainRenderer, abs(starfield[j] - (((starfield[j+1]+starfieldScroll)/getScreenHeight())%2*getScreenWidth()))%getScreenWidth(), (starfield[j+1]+starfieldScroll)%getScreenHeight());
    }
}

// Frees the starfield to avoid memory leaks.
void freeStarfield() {
    free(starfield);
}
