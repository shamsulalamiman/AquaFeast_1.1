#ifndef LEVELS_HPP
#define LEVELS_HPP
// =====================================================================
// levels.hpp - what makes each level different, and the per-tick
// update / draw for gameplay.
//
// Every level runs the SAME code - a level is just a set of numbers.
//
// Each level has TWO HALVES. At 50% progress an alert appears, a
// powerful DANGER enemy arrives, and every enemy speeds up. Each level
// uses a different danger enemy, and both the count and the speed climb
// from level 1 to level 3.
// =====================================================================
#include "utility.hpp"
#include "player.hpp"
#include "entities.hpp"
#include "boat.hpp"
#include "environment.hpp"
#include "hud.hpp"
#include "sound.hpp"
#include "scores.hpp"

struct LevelInfo {
    double width;        // how wide the world is
    double goalSize;     // grow this big to win
    int    seconds;      // time limit
    int    preyCount;
    double preySpeed;
    int    predCount;    // predators in the FIRST half
    double predSpeed;
    double predSize;
    int    dangerCount;  // extra danger enemies in the SECOND half
    double dangerSpeed;
    double dangerSize;
    const char* dangerName;
};

LevelInfo levels[MAX_LEVELS + 1];   // index 0 unused, levels 1..3

void setupLevels() {
    //              width   goal secs prey pSpd prd pSpd pSize dng dSpd dSize name
    levels[1] = { 2600.0,  70.0, 120,  32, 1.6,   3, 2.2, 44.0,  1, 3.6, 54.0, "BARRACUDA" };
    levels[2] = { 3400.0,  96.0, 100,  30, 2.2,   5, 3.0, 50.0,  2, 4.5, 62.0, "NIGHT SHARK" };
    levels[3] = { 4200.0, 124.0,  85,  28, 2.9,   7, 3.8, 56.0,  3, 5.4, 70.0, "STORM HUNTER" };
}

void loadLevels() { setupLevels(); }

// ==== SECOND-HALF EVENT ====
bool dangerArrived = false;   // has the halfway event already fired?
int  alertTicks = 0;          // how long the on-screen alert still shows
int  respawnTick = 0;         // counts down to the next prey respawn

// Spawns the level's danger enemies and makes everything faster.
void triggerDangerEvent() {
    LevelInfo &L = levels[currentLevel];
    dangerArrived = true;
    alertTicks = 130;          // ~4 seconds of warning text
    playAlert();

    for (int i = 0; i < L.dangerCount; i++) {
        // Arrive from off to the side of the player, then close in.
        double x = clampD(player.x + (i % 2 ? 620.0 : -620.0), 100, L.width - 100);
        double y = randRange(FLOOR_Y + 100, SEA_Y - 100);
        addPredator(x, y, L.dangerSize, L.dangerSpeed, 2);
    }
    speedUpPredators(1.25);    // the whole ocean gets more aggressive
}

// The flashing banner shown when the danger enemy arrives.
void drawDangerAlert() {
    if (alertTicks <= 0) return;
    if ((alertTicks / 8) % 2 == 0) return;      // blink

    char msg[96];
    sprintf_s(msg, "!  %s INCOMING  !", levels[currentLevel].dangerName);

    double w = 560, h = 64;
    double x = SCREEN_W / 2.0 - w / 2, y = SEA_Y - 120;
    drawPanel(x, y, w, h, 60, 12, 16, 255, 120, 110);
    drawTextCentred(SCREEN_W / 2.0, y + 26, msg, 255, 225, 120, BIG_FONT);
}

// ==== SCORE ====
void saveRunScore() {
    if (scoreSaved) return;
    scoreSaved = true;
    submitScore(playerName, stats.score);
}

// ==== SPAWNING ====
void spawnForLevel(int n) {
    LevelInfo &L = levels[n];
    preyCount = 0;
    predCount = 0;

    for (int i = 0; i < L.preyCount; i++) {
        double x = randRange(150, L.width - 150);
        double y = randRange(FLOOR_Y + 80, SEA_Y - 80);
        // A few larger prey appear from level 2 on - worth more points,
        // but you must grow before you can eat them.
        double size = (n >= 2 && i % 4 == 0) ? randRange(34, 46) : randRange(17, 25);
        addPrey(x, y, size, L.preySpeed);
    }

    for (int i = 0; i < L.predCount; i++) {
        double x = randRange(250, L.width - 250);
        double y = randRange(FLOOR_Y + 110, SEA_Y - 110);
        addPredator(x, y, L.predSize, L.predSpeed, (n >= 2) ? 1 : 0);
    }
}

void startLevel(int n) {
    currentLevel = n;
    scoreSaved = false;
    dangerArrived = false;
    alertTicks = 0;
    respawnTick = 45;
    LevelInfo &L = levels[n];

    resetPlayer(L.width / 2.0, SEA_Y / 2.0);
    spawnForLevel(n);
    resetBoat(n);
    resetEnvironment(L.width);

    stats.level = n;
    stats.timeLeft = L.seconds;
    stats.progress = 0.0;
    isGameOver = false;
    isLevelWon = false;

    screen = SCR_PLAY;
    stopMenuMusic();
    startGameMusic();
}

// Restart the level currently being played, from the HUD button.
void restartLevel() {
    stats.score = 0;
    stats.lives = 3;
    startLevel(currentLevel);
}

// ==== PER-TICK UPDATE ====
void updateLevel() {
    LevelInfo &L = levels[currentLevel];

    if (alertTicks > 0) alertTicks--;

    // While hooked the fish cannot swim - the tug-of-war takes over,
    // and the other fish hold position so the moment stays fair.
    if (!playerIsHooked()) {
        updateEntities(L.width);
        updateJump();
    }

    updateRespawn();
    updateBoat(L.width);
    updateEnvironment();
    updateSwimBubbles();
    updateSeaBubbles();
    tickPlayerTimers();
    clampPlayer(L.width);
    followPlayer();

    // Keep the ocean stocked. Eaten fish come back a few seconds later,
    // so there is always enough food to reach the level's size goal.
    respawnTick--;
    if (respawnTick <= 0) {
        respawnTick = 45;
        if (countLivePrey() < L.preyCount)
            respawnOnePrey(L.width, randRange(17, 25) + player.size * 0.12, L.preySpeed);
    }

    stats.progress = player.size / L.goalSize;
    if (stats.progress > 1.0) stats.progress = 1.0;

    // Halfway point: the level's second, harder part begins.
    if (!dangerArrived && stats.progress >= 0.5) triggerDangerEvent();

    if (!isLevelWon && player.size >= L.goalSize) {
        isLevelWon = true;
        stats.score += stats.timeLeft * 6;          // time bonus
        if (currentLevel >= unlockedLevel && currentLevel < MAX_LEVELS)
            unlockedLevel = currentLevel + 1;        // unlock the next level
        saveRunScore();
        stopGameMusic();
        playWin();
    }
}

// Called once per second by a timer in iMain.cpp.
void tickLevelClock() {
    stats.timeLeft--;
    if (stats.timeLeft <= 0) {
        stats.timeLeft = 0;
        saveRunScore();
        triggerGameOver();
    }
    tickBoatClocks();
}

// ==== DRAW ====
void drawLevel() {
    drawEnvironment();
    drawSwimBubbles();
    drawBoat();
    drawNets();
    drawEntities();
    drawPowerups();
    drawPlayer();
    drawHook();          // after the player, so it is visibly attached
    drawHookBars();
    drawDangerAlert();
    drawHud();
}

#endif
