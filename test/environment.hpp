#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP
// =====================================================================
// environment.hpp - the scenery behind the gameplay.
//
// All 3 levels share the SAME layout: sky, water surface, 3 water depth
// bands, sand floor and seaweed. Only the WEATHER changes:
//     Level 1 - sunny day  (sun in the sky)
//     Level 2 - night      (moon + stars)
//     Level 3 - rainy      (grey sky + falling rain)
// Keeping one drawing path with a per-level "mood" makes the levels feel
// like one world, and means a fix to the scenery fixes all 3 at once.
// =====================================================================
#include "utility.hpp"
#include "player.hpp"

#define MAX_CLOUDS   4
#define MAX_BIRDS    3
#define BIRD_FRAMES  3
#define MAX_WEEDS   26
#define MAX_RAIN   140
#define MAX_DRIFT   18   // slow floating specks, adds depth underwater

// ==== 1. STRUCTS ====
struct Drifter { double x, y, speed; };      // clouds and birds
struct Weed    { double x; int look; };
struct Rain    { double x, y, speed; };
struct Speck   { double x, y, speed; double size; };

Drifter clouds[MAX_CLOUDS];
Drifter birds[MAX_BIRDS];
Weed    weeds[MAX_WEEDS];
Rain    rain[MAX_RAIN];
Speck   specks[MAX_DRIFT];

int cloudSprite, weedSprite[3];
int birdFrame[BIRD_FRAMES];   // 3 images = flapping wings
int birdAnimTick = 0;

double worldW = 3000;   // width of the current level's world

// ==== 2. LOADING ====
void loadEnvironment() {
    cloudSprite = loadImg("Images/Background/cloud_01.png");

    // 3 bird images played in sequence make the wings flap. If you only
    // have bird_01.png, all 3 fall back to it and the bird simply glides.
    birdFrame[0] = loadImg("Images/Background/bird_01.png");
    birdFrame[1] = loadImg("Images/Background/bird_02.png", "Images/Background/bird_01.png");
    birdFrame[2] = loadImg("Images/Background/bird_03.png", "Images/Background/bird_01.png");

    weedSprite[0] = loadImg("Images/Background/seaweed_01.png");
    weedSprite[1] = loadImg("Images/Background/seaweed_02.png", "Images/Background/seaweed_01.png");
    weedSprite[2] = loadImg("Images/Background/seaweed_03.png", "Images/Background/seaweed_01.png");

    for (int i = 0; i < MAX_CLOUDS; i++)
        clouds[i] = { i * 460.0, SEA_Y + 120 + (i % 3) * 55.0, 0.25 + i * 0.08 };
    for (int i = 0; i < MAX_BIRDS; i++)
        birds[i] = { i * 620.0 + 250, SEA_Y + 170 + i * 45.0, -(0.9 + i * 0.25) };
    for (int i = 0; i < MAX_WEEDS; i++)
        weeds[i] = { i * 190.0 + randRange(0, 110), rand() % 3 };
    for (int i = 0; i < MAX_RAIN; i++)
        rain[i] = { randRange(0, SCREEN_W), randRange(SEA_Y, SCREEN_H), randRange(9, 15) };
    for (int i = 0; i < MAX_DRIFT; i++)
        specks[i] = { randRange(0, SCREEN_W), randRange(20, SEA_Y - 20), randRange(0.15, 0.5), randRange(1.5, 3.5) };
}

void resetEnvironment(double levelWidth) { worldW = levelWidth; }

// The view slides sideways to keep the player near the middle.
void followPlayer() {
    scrollX = clampD(player.x, 0, worldW);
}

// ==== 3. UNDERWATER ====
// Three depth bands: lightest near the surface, darkest at the floor.
// Each level tints them differently to match its weather.
void drawWaterBands() {
    int top[3], mid[3], deep[3];

    if (currentLevel == 2) {            // night - cold dark blues
        top[0]=10;  top[1]=34;  top[2]=76;
        mid[0]=6;   mid[1]=20;  mid[2]=50;
        deep[0]=2;  deep[1]=9;  deep[2]=26;
    } else if (currentLevel == 3) {     // rainy - murky grey-green
        top[0]=26;  top[1]=72;  top[2]=92;
        mid[0]=16;  mid[1]=50;  mid[2]=68;
        deep[0]=8;  deep[1]=28; deep[2]=42;
    } else {                            // sunny - bright blue
        top[0]=24;  top[1]=104; top[2]=158;
        mid[0]=14;  mid[1]=70;  mid[2]=115;
        deep[0]=8;  deep[1]=40; deep[2]=78;
    }

    iSetColor(deep[0], deep[1], deep[2]);
    iFilledRectangle(0, 0, SCREEN_W, BAND_H);
    iSetColor(mid[0], mid[1], mid[2]);
    iFilledRectangle(0, BAND_H, SCREEN_W, BAND_H);
    iSetColor(top[0], top[1], top[2]);
    iFilledRectangle(0, BAND_H * 2, SCREEN_W, SEA_Y - BAND_H * 2);
}

void drawSand() {
    if (currentLevel == 2)      iSetColor(96, 88, 70);
    else if (currentLevel == 3) iSetColor(150, 140, 110);
    else                         iSetColor(214, 196, 148);
    iFilledRectangle(0, 0, SCREEN_W, SAND_H);
}

// Seaweed sits at fixed spots on the floor, so it scrolls with the world.
void drawSeaweed() {
    for (int i = 0; i < MAX_WEEDS; i++) {
        double sx = toScreenX(weeds[i].x);
        if (sx < -80 || sx > SCREEN_W + 80) continue;
        iShowImage((int)sx - 34, (int)SAND_H - 12, 68, 92, weedSprite[weeds[i].look]);
    }
}

// Tiny floating specks - cheap way to make the water feel alive.
void drawSpecks() {
    glColor4f(1.0f, 1.0f, 1.0f, 0.20f);
    for (int i = 0; i < MAX_DRIFT; i++)
        iFilledCircle(specks[i].x, specks[i].y, specks[i].size, 8);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

// ==== 4. SKY ====
void drawSun() {
    double x = SCREEN_W - 260, y = SEA_Y + 230;
    glColor4f(1.0f, 0.93f, 0.55f, 0.22f);
    iFilledCircle(x, y, 78, 36);
    iSetColor(255, 238, 140);
    iFilledCircle(x, y, 48, 36);
    iSetColor(255, 250, 205);
    iFilledCircle(x, y, 38, 36);
}

void drawMoonAndStars() {
    // Stars: spread with a fixed pattern so they never jitter.
    iSetColor(215, 230, 255);
    for (int i = 0; i < 40; i++) {
        double sx = fmod(i * 197.0, (double)SCREEN_W);
        double sy = SEA_Y + 40 + fmod(i * 83.0, SCREEN_H - SEA_Y - 60);
        iFilledCircle(sx, sy, (i % 4 == 0) ? 2.2 : 1.3, 6);
    }

    double x = SCREEN_W - 280, y = SEA_Y + 225;
    glColor4f(0.55f, 0.72f, 1.0f, 0.16f);
    iFilledCircle(x, y, 74, 36);
    iSetColor(238, 244, 255);
    iFilledCircle(x, y, 44, 36);
    // A slightly offset darker circle carves out a crescent shape.
    iSetColor(8, 14, 34);
    iFilledCircle(x + 17, y + 9, 38, 36);
}

void drawRain() {
    iSetColor(175, 200, 225);
    for (int i = 0; i < MAX_RAIN; i++)
        iLine(rain[i].x, rain[i].y, rain[i].x - 5, rain[i].y - 17);
}

void drawSky() {
    if (currentLevel == 2)      iSetColor(8, 14, 34);     // night
    else if (currentLevel == 3) iSetColor(96, 106, 120);  // storm grey
    else                         iSetColor(126, 200, 240); // clear day
    iFilledRectangle(0, SEA_Y, SCREEN_W, SCREEN_H - SEA_Y);

    if (currentLevel == 1) drawSun();
    if (currentLevel == 2) drawMoonAndStars();

    // Clouds. At night they are dimmed; in the storm they are grey.
    if (currentLevel == 2)      glColor4f(0.42f, 0.46f, 0.60f, 0.75f);
    else if (currentLevel == 3) glColor4f(0.62f, 0.64f, 0.68f, 0.95f);
    for (int i = 0; i < MAX_CLOUDS; i++)
        iShowImage((int)clouds[i].x, (int)clouds[i].y, 120, 74, cloudSprite);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // Birds flap by cycling through their 3 frames.
    int frame = (birdAnimTick / 6) % BIRD_FRAMES;
    for (int i = 0; i < MAX_BIRDS; i++)
        iShowImage((int)birds[i].x, (int)birds[i].y, 54, 40, birdFrame[frame]);

    if (currentLevel == 3) drawRain();

    // A bright line marks the water surface in every level.
    iSetColor(190, 225, 245);
    iFilledRectangle(0, SEA_Y - 2, SCREEN_W, 3);
}

// ==== 5. UPDATE & DRAW ====
void updateEnvironment() {
    birdAnimTick++;

    for (int i = 0; i < MAX_CLOUDS; i++) {
        clouds[i].x += clouds[i].speed;                      // left -> right
        if (clouds[i].x > SCREEN_W + 120) clouds[i].x = -140;
    }
    for (int i = 0; i < MAX_BIRDS; i++) {
        birds[i].x += birds[i].speed;                         // right -> left
        if (birds[i].x < -70) birds[i].x = SCREEN_W + 60;
    }
    for (int i = 0; i < MAX_DRIFT; i++) {
        specks[i].x += specks[i].speed;
        if (specks[i].x > SCREEN_W) { specks[i].x = 0; specks[i].y = randRange(20, SEA_Y - 20); }
    }

    if (currentLevel != 3) return;
    for (int i = 0; i < MAX_RAIN; i++) {
        rain[i].y -= rain[i].speed;
        rain[i].x -= 2.2;
        if (rain[i].y < SEA_Y) {              // rain stops at the water
            rain[i].y = SCREEN_H + randRange(0, 60);
            rain[i].x = randRange(0, SCREEN_W);
        }
    }
}

void drawEnvironment() {
    drawWaterBands();
    drawSpecks();
    drawSeaweed();
    drawSand();
    drawSky();
}

#endif
