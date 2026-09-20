#ifndef LEVELS_HPP
#define LEVELS_HPP
// =====================================================================
// levels.hpp - what makes each level different, and the per-tick
// update / draw for gameplay.
//
// Every level runs the SAME code. A level is just a set of numbers.
// Difficulty rises sharply: bigger goal, less time, faster and more
// dangerous fish, and an extra hazard each time.
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
    int    preyCount;    // how much food there is
    double preySpeed;
    int    predCount;    // how many predators
    double predSpeed;
    double predSize;
};

LevelInfo levels[MAX_LEVELS + 1];   // index 0 unused, levels 1..3

void setupLevels() {
    //                width  goal  secs prey pSpd preds pdSpd pdSize
    levels[1] = {  2600.0,  46.0, 120,  30, 1.5,    3,  2.1,  30.0 };
    levels[2] = {  3400.0,  62.0, 100,  26, 2.1,    5,  2.9,  34.0 };
    levels[3] = {  4200.0,  80.0,  85,  22, 2.7,    7,  3.6,  38.0 };
}

void loadLevels() { setupLevels(); }

// ==== SPAWNING ====
// Prey get scarcer and faster each level while the goal grows, so later
// levels genuinely demand better play rather than just more waiting.
void spawnForLevel(int n) {
    LevelInfo &L = levels[n];
    preyCount = 0;
    predCount = 0;

    for (int i = 0; i < L.preyCount; i++) {
        double x = randRange(120, L.width - 120);
        double y = randRange(FLOOR_Y + 60, SEA_Y - 60);
        // A few larger prey appear from level 2 on - worth more points,
        // but you must grow before you can eat them.
        double size = (n >= 2 && i % 4 == 0) ? randRange(22, 30) : randRange(11, 16);
        addPrey(x, y, size, L.preySpeed);
    }

    for (int i = 0; i < L.predCount; i++) {
        double x = randRange(200, L.width - 200);
        double y = randRange(FLOOR_Y + 80, SEA_Y - 80);
        addPredator(x, y, L.predSize, L.predSpeed, (n >= 2) ? 1 : 0);
    }
}

// Saves the run's score exactly once, no matter how the run ended
// (won, ran out of time, or lost every life).
void saveRunScore() {
    if (scoreSaved) return;
    scoreSaved = true;
    submitScore(playerName, stats.score);
}

void startLevel(int n) {
    currentLevel = n;
    scoreSaved = false;
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

// ==== PER-TICK UPDATE ====
void updateLevel() {
    LevelInfo &L = levels[currentLevel];

    // While hooked the fish cannot swim - the tug-of-war takes over.
    if (!playerIsHooked()) {
        updateEntities(L.width);
        updateJump();
    }

    updateRespawn();
    updateBoat(L.width);
    updateEnvironment();
    updateSwimBubbles();
    tickPlayerTimers();
    clampPlayer(L.width);
    followPlayer();

    stats.progress = player.size / L.goalSize;
    if (stats.progress > 1.0) stats.progress = 1.0;

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
    drawNetsAndHook();
    drawEntities();
    drawPowerups();
    drawPlayer();
    drawHookBar();
    drawHud();
}

#endif
