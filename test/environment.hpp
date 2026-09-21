#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP
// =====================================================================
// environment.hpp - the scenery behind the gameplay.
//
// All 3 levels share the SAME layout: sky, water surface, 3 water depth
// bands, sand floor and seaweed. Only the WEATHER changes:
//     Level 1 - sunny day  (sun image)
//     Level 2 - night      (moon image + stars)
//     Level 3 - rainy      (grey sky + falling rain)
// =====================================================================
#include "utility.hpp"
#include "player.hpp"

#define MAX_CLOUDS    5
#define MAX_BIRDS     3
#define BIRD_FRAMES   3
#define MAX_WEEDS    20    // enough to cover the whole sea floor
#define MAX_RAIN    160

// ==== 1. STRUCTS ====
struct Drifter { double x, y, speed; };
struct Weed    { double x; int look; double w, h; };
struct Rain    { double x, y, speed; };

Drifter clouds[MAX_CLOUDS];
Drifter birds[MAX_BIRDS];
Weed    weeds[MAX_WEEDS];
Rain    rain[MAX_RAIN];

int cloudSprite, weedSprite[3];
int birdFrame[BIRD_FRAMES];
int sunSprite, moonSprite;
int birdAnimTick = 0;

double worldW = 3000;      // width of the current level's world

// ==== 2. LOADING ====
void loadEnvironment() {
    cloudSprite = loadImg("Images/Background/cloud_01.png");
    sunSprite   = loadImg("Images/Background/sun.png");
    moonSprite  = loadImg("Images/Background/moon.png");

    // 3 bird pictures played in turn make the wings flap. Missing ones
    // fall back to bird_01, so the bird simply glides instead.
    birdFrame[0] = loadImg("Images/Background/bird_01.png");
    birdFrame[1] = loadImg("Images/Background/bird_02.png", "Images/Background/bird_01.png");
    birdFrame[2] = loadImg("Images/Background/bird_03.png", "Images/Background/bird_01.png");

    weedSprite[0] = loadImg("Images/Background/seaweed_01.png");
    weedSprite[1] = loadImg("Images/Background/seaweed_02.png", "Images/Background/seaweed_01.png");
    weedSprite[2] = loadImg("Images/Background/seaweed_03.png", "Images/Background/seaweed_01.png");

    for (int i = 0; i < MAX_CLOUDS; i++)
        clouds[i].x = i * 430.0, clouds[i].y = SEA_Y + 130 + (i % 3) * 60.0, clouds[i].speed = 0.25 + i * 0.07;
    for (int i = 0; i < MAX_BIRDS; i++)
        birds[i].x = i * 520.0 + 200, birds[i].y = SEA_Y + 185 + i * 42.0, birds[i].speed = -(0.9 + i * 0.22);

    // Seaweed is packed close together with varied size so the sand is
    // fully covered rather than dotted with a few plants.
    for (int i = 0; i < MAX_WEEDS; i++) {
        weeds[i].x = i * 92.0 + randRange(-24, 24);
        weeds[i].look = rand() % 3;
        weeds[i].w = randRange(58, 104);
        weeds[i].h = randRange(74, 132);
    }

    for (int i = 0; i < MAX_RAIN; i++)
        rain[i].x = randRange(0, SCREEN_W), rain[i].y = randRange(SEA_Y, SCREEN_H), rain[i].speed = randRange(9, 16);
}

void resetEnvironment(double levelWidth) { worldW = levelWidth; }

void followPlayer() { scrollX = clampD(player.x, 0, worldW); }

// ==== 3. UNDERWATER ====
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

// Seaweed sits at fixed spots on the floor, so it scrolls with the
// world. Drawing it in two passes (small behind, large in front) gives
// the sea bed a fuller, layered look.

// Seaweed sits at fixed spots on the floor, so it scrolls with the world.
void drawSeaweed() {
	for (int i = 0; i < MAX_WEEDS; i++) {
		double sx = toScreenX(weeds[i].x);
		//if (sx < -80 || sx > SCREEN_W + 80) continue;
		iShowImage((int)sx - 50, (int)SAND_H - 12, 50, 50, weedSprite[weeds[i].look]);
	}
}

// ==== 4. SKY ====
void drawSun() {
    iShowImage((int)(SCREEN_W - 330), (int)(SEA_Y + 150), 170, 170, sunSprite);
}

void drawMoonAndStars() {
    iSetColor(215, 230, 255);
    for (int i = 0; i < 46; i++) {
        double sx = fmod(i * 197.0, (double)SCREEN_W);
        double sy = SEA_Y + 40 + fmod(i * 83.0, SCREEN_H - SEA_Y - 60);
        iFilledCircle(sx, sy, (i % 4 == 0) ? 2.4 : 1.4, 6);
    }
	iShowImage((int)(SCREEN_W - 340), (int)(SEA_Y + 150), 150, 150, moonSprite);
}

void drawRain() {
    iSetColor(175, 200, 225);
    for (int i = 0; i < MAX_RAIN; i++)
        iLine(rain[i].x, rain[i].y, rain[i].x - 6, rain[i].y - 20);
}

void drawSky() {
    if (currentLevel == 2)      iSetColor(8, 14, 34);
    else if (currentLevel == 3) iSetColor(96, 106, 120);
    else                         iSetColor(126, 200, 240);
    iFilledRectangle(0, SEA_Y, SCREEN_W, SCREEN_H - SEA_Y);

    if (currentLevel == 1) drawSun();
    if (currentLevel == 2) drawMoonAndStars();

    // Clouds are dimmed at night and greyed in the storm.
    if (currentLevel == 2)      iSetColor(110, 120, 155);
    else if (currentLevel == 3) iSetColor(160, 164, 172);
    else                         iSetColor(255, 255, 255);
    for (int i = 0; i < MAX_CLOUDS; i++)
        iShowImage((int)clouds[i].x, (int)clouds[i].y, 100, 100, cloudSprite);
    iSetColor(255, 255, 255);

    // Birds flap by cycling through their 3 frames.
    int frame = (birdAnimTick / 6) % BIRD_FRAMES;
    if (currentLevel == 2) iSetColor(150, 160, 190);
    for (int i = 0; i < MAX_BIRDS; i++)
        iShowImage((int)birds[i].x, 850, 50, 50, birdFrame[frame]);
    iSetColor(255, 255, 255);

    if (currentLevel == 3) drawRain();

    iSetColor(190, 225, 245);
    iFilledRectangle(0, SEA_Y - 2, SCREEN_W, 3);
}

// ==== 5. UPDATE & DRAW ====
void updateEnvironment() {
    birdAnimTick++;

    for (int i = 0; i < MAX_CLOUDS; i++) {
        clouds[i].x += clouds[i].speed;                        // left -> right
        if (clouds[i].x > SCREEN_W + 150) clouds[i].x = -170;
    }
    for (int i = 0; i < MAX_BIRDS; i++) {
        birds[i].x += birds[i].speed;                           // right -> left
        if (birds[i].x < -80) birds[i].x = SCREEN_W + 70;
    }

    if (currentLevel != 3) return;
    for (int i = 0; i < MAX_RAIN; i++) {
        rain[i].y -= rain[i].speed;
        rain[i].x -= 2.4;
        if (rain[i].y < SEA_Y) {                 // rain stops at the water
            rain[i].y = SCREEN_H + randRange(0, 70);
            rain[i].x = randRange(0, SCREEN_W);
        }
    }
}

void drawEnvironment() {
    drawWaterBands();
    drawSeaBubbles();      // ambient bubbles rising from the deep
    drawSeaweed();
    drawSand();
    drawSky();
}

#endif
