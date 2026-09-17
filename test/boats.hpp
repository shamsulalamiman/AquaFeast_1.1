#ifndef BOATS_H
#define BOATS_H
// ==== 1. INCLUDES ====
#include "utility.hpp"
#include "player.hpp"
#include "menu.hpp" // for playCollectSound(), triggerGameOver()

// ==== 2. CONSTANTS ====
#define MAX_BOATS     5
#define MAX_HAZARDS   20
#define MAX_POWERUPS  20
const double HAZARD_SINK_SPEED = 2.0;

// ==== 3. STRUCTS ====
struct Boat {
    double x, y, speed;
    int spriteId;
    int ticksUntilNextDrop;
};

enum HazardType { HZ_HOOK, HZ_NET };
struct BoatHazard { double x, y, size; HazardType type; bool active; };

enum PowerupType { PU_COIN, PU_EXTRA_LIFE, PU_SPEED_UP };
struct Powerup { double x, y, size; PowerupType type; bool collected; };
struct TreasureChest { double x, y, size; bool active; int secondsRemaining; };

// ==== 4. GLOBAL ARRAYS / STATE ====
Boat boats[MAX_BOATS];
int boatCount = 0;
BoatHazard hazards[MAX_HAZARDS];
int hazardCount = 0;
Powerup powerups[MAX_POWERUPS];
int powerupCount = 0;
TreasureChest chest;

int boatSprite, hookSprite, netSprite;
int coinSprite, extraLifeSprite, speedUpSprite, chestSprite;
int netSlowSecondsLeft = 0;
int chestCycleSecondsLeft = 30;
int speedUpSecondsLeft = 0;

// ==== 5. FUNCTIONS ====
void loadBoats() {
    boatSprite = iLoadImage("Images/Boats/boat_01.png");
    hookSprite = iLoadImage("Images/Boats/hook_01.png");
    netSprite  = iLoadImage("Images/Boats/net_01.png");
    coinSprite      = iLoadImage("Images/Powerups/coin_01.png");
    extraLifeSprite = iLoadImage("Images/Powerups/extra_life_01.png");
    speedUpSprite   = iLoadImage("Images/Powerups/speed_up_01.png");
    chestSprite     = iLoadImage("Images/Powerups/treasure_chest_01.png");
}

void resetBoats() {
    boatCount = 0;
    hazardCount = 0;
    netSlowSecondsLeft = 0;

    Boat b = { 0, WATER_SURFACE_Y, 2, boatSprite, 100 + (rand() % 150) };
    boats[boatCount++] = b;
}

void resetPowerups() {
    powerupCount = 0;
    chest.active = false;
    chest.secondsRemaining = 0;
    chestCycleSecondsLeft = 30;
    speedUpSecondsLeft = 0;
}

int spriteForPowerupType(PowerupType type) {
    if (type == PU_COIN)       return coinSprite;
    if (type == PU_EXTRA_LIFE) return extraLifeSprite;
    return speedUpSprite;
}

void spawnPowerup(double x, double y, PowerupType type) {
    if (powerupCount >= MAX_POWERUPS) return;
    Powerup p = { x, y, 10.0, type, false };
    powerups[powerupCount++] = p;
}

void dropHazard(double x, double y, HazardType type) {
    if (hazardCount >= MAX_HAZARDS) return;
    BoatHazard h = { x, y, 8.0, type, true };
    hazards[hazardCount++] = h;
}

void maybeSpawnTreasureChest() {
    if (chest.active) return;
    chest.x = scrollX + (rand() % SCREEN_WIDTH) - SCREEN_CENTER_X;
    chest.y = WORLD_FLOOR_Y + (rand() % (int)(WATER_SURFACE_Y - WORLD_FLOOR_Y));
    chest.size = 14.0;
    chest.active = true;
    chest.secondsRemaining = 10;
}

void tickChestClock() {
    if (chest.active) {
        chest.secondsRemaining--;
        if (chest.secondsRemaining <= 0) chest.active = false;
    }
    chestCycleSecondsLeft--;
    if (chestCycleSecondsLeft <= 0) {
        chestCycleSecondsLeft = 30;
        maybeSpawnTreasureChest();
    }
}

void tickSpeedUpClock() {
    if (speedUpSecondsLeft <= 0) return;
    speedUpSecondsLeft--;
    if (speedUpSecondsLeft == 0) player.speed = playerBaseSpeed;
}

void tickNetSlowClock() {
    if (netSlowSecondsLeft <= 0) return;
    netSlowSecondsLeft--;
    if (netSlowSecondsLeft == 0) player.speed = playerBaseSpeed;
}

// Moves the boats and, once in a while, drops something. Split out from
// updateBoats() so each function does one clear job.
void moveBoatsAndDrop(double worldWidth, bool allowNets, bool allowHooks) {
    for (int i = 0; i < boatCount; i++) {
        boats[i].y = WATER_SURFACE_Y;
        boats[i].x += boats[i].speed;
        if (boats[i].x > worldWidth + 100) boats[i].x = -100;

        boats[i].ticksUntilNextDrop--;
        if (boats[i].ticksUntilNextDrop <= 0) {
            boats[i].ticksUntilNextDrop = 100 + (rand() % 150);
            int roll = rand() % 10;
            if (roll < 4)                    spawnPowerup(boats[i].x, WATER_SURFACE_Y + 30, PU_COIN);
            else if (roll == 4)              spawnPowerup(boats[i].x, WATER_SURFACE_Y + 30, PU_EXTRA_LIFE);
            else if (roll == 5 || roll == 6) spawnPowerup(boats[i].x, WATER_SURFACE_Y + 30, PU_SPEED_UP);
            else if (roll == 7 && allowNets)  dropHazard(boats[i].x, WATER_SURFACE_Y, HZ_NET);
            else if (roll == 8 && allowHooks) dropHazard(boats[i].x, WATER_SURFACE_Y, HZ_HOOK);
            else                                spawnPowerup(boats[i].x, WATER_SURFACE_Y + 30, PU_COIN);
        }
    }
}

// Sinks dropped hazards and checks if the player hit one.
void updateHazards() {
    for (int i = 0; i < hazardCount; i++) {
        if (!hazards[i].active) continue;
        hazards[i].y -= HAZARD_SINK_SPEED;
        if (hazards[i].y < WORLD_FLOOR_Y) { hazards[i].active = false; continue; }

        if (circlesTouch(player.x, player.y, player.size, hazards[i].x, hazards[i].y, hazards[i].size)) {
            if (hazards[i].type == HZ_HOOK) {
                stats.lives -= 1;
                if (stats.lives <= 0) triggerGameOver();
            } else {
                netSlowSecondsLeft = 5;
                player.speed = playerBaseSpeed * 0.5;
            }
            hazards[i].active = false;
        }
    }
}

void updateBoats(double worldWidth, bool allowNets, bool allowHooks) {
    moveBoatsAndDrop(worldWidth, allowNets, allowHooks);
    updateHazards();
}

void updatePowerups() {
    for (int i = 0; i < powerupCount; i++) {
        if (powerups[i].collected) continue;
        if (!circlesTouch(player.x, player.y, player.size, powerups[i].x, powerups[i].y, powerups[i].size)) continue;

        powerups[i].collected = true;
        playCollectSound();
        if (powerups[i].type == PU_COIN) stats.score += 10;
        else if (powerups[i].type == PU_EXTRA_LIFE) stats.lives += 1;
        else { speedUpSecondsLeft = 10; player.speed = playerBaseSpeed * 2.0; }
    }

    if (chest.active && circlesTouch(player.x, player.y, player.size, chest.x, chest.y, chest.size)) {
        stats.score += 100;
        chest.active = false;
        playCollectSound();
    }
}

void drawBoats() {
    for (int i = 0; i < boatCount; i++)
        iShowImage((int)worldToScreenX(boats[i].x), (int)worldToScreenY(boats[i].y), 100, 100, boats[i].spriteId);

    for (int i = 0; i < hazardCount; i++) {
        if (!hazards[i].active) continue;
        double s = hazards[i].size * 2.0;
        int sprite = (hazards[i].type == HZ_HOOK) ? hookSprite : netSprite;
        iShowImage((int)(worldToScreenX(hazards[i].x) - s / 2), (int)(worldToScreenY(hazards[i].y) - s / 2), (int)s, (int)s, sprite);
    }
}

void drawPowerups() {
    for (int i = 0; i < powerupCount; i++) {
        if (powerups[i].collected) continue;
        double s = powerups[i].size * 2.0;
        iShowImage((int)(worldToScreenX(powerups[i].x) - s / 2), (int)(worldToScreenY(powerups[i].y) - s / 2),
                   (int)s, (int)s, spriteForPowerupType(powerups[i].type));
    }

    if (chest.active) {
        double s = chest.size * 2.0;
        iShowImage((int)(worldToScreenX(chest.x) - s / 2), (int)(worldToScreenY(chest.y) - s / 2), (int)s, (int)s, chestSprite);
        iSetColor(255, 255, 0);
        char timerText[16];
        sprintf_s(timerText, "%d s", chest.secondsRemaining);
        iText((int)(worldToScreenX(chest.x) - 10), (int)(worldToScreenY(chest.y) + s / 2 + 6), timerText, GAME_FONT);
    }
}

#endif
