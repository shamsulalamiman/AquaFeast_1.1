#ifndef LEVELS_H
#define LEVELS_H

// ==== 1. INCLUDES ====
#include "utility.hpp"
#include "player.hpp"
#include "entities.hpp"
#include "boats.hpp"
#include "environment.hpp"
#include "menu.hpp" // for playWinSound()

// ==== 2. STRUCT ====
struct LevelConfig {
	double worldWidth;
	double targetSize;
	int    timeLimitSeconds;
	bool   netsEnabled;
	bool   hooksEnabled;
};

// ==== 3. GLOBAL ARRAY ====
LevelConfig levelConfigs[MAX_LEVELS + 1];

// ==== 4. LEVEL 2 OCEAN CURRENTS (DYNAMIC & ENHANCED DIFFICULTY) ====
struct CurrentZone {
	double minX, maxX;
	double minY, maxY;
	double pushX, pushY;
};

#define MAX_CURRENTS 3
CurrentZone level2Currents[MAX_CURRENTS];
int level2CurrentCount = 0;
int currentFlipTimer = 0; // Timer to reverse current flows periodically

void setupLevel2Currents() {
	level2CurrentCount = 0;
	currentFlipTimer = 0;

	// Zone 1: Mid-water current pushing EAST (Right)
	level2Currents[0] = { 400.0, 1200.0, 150.0, 300.0, 1.4, 0.0 };

	// Zone 2: Deep ocean current pushing WEST (Left)
	level2Currents[1] = { 1400.0, 2200.0, 40.0, 180.0, -1.8, 0.0 };

	level2CurrentCount = 2;
}

void applyOceanCurrents() {
	if (currentLevel != 2) return;

	// High Difficulty: Reverse current directions every ~4 seconds (240 ticks)
	currentFlipTimer++;
	if (currentFlipTimer >= 240) {
		for (int i = 0; i < level2CurrentCount; i++) {
			level2Currents[i].pushX *= -1.0;
		}
		currentFlipTimer = 0;
	}

	// 1. Push Player
	for (int i = 0; i < level2CurrentCount; i++) {
		CurrentZone &z = level2Currents[i];
		if (player.x >= z.minX && player.x <= z.maxX &&
			player.y >= z.minY && player.y <= z.maxY) {
			player.x += z.pushX;
			player.y += z.pushY;
		}
	}

	// 2. Push Prey Fish
	for (int p = 0; p < preyCount; p++) {
		if (!preyList[p].alive) continue;
		for (int i = 0; i < level2CurrentCount; i++) {
			CurrentZone &z = level2Currents[i];
			if (preyList[p].x >= z.minX && preyList[p].x <= z.maxX &&
				preyList[p].y >= z.minY && preyList[p].y <= z.maxY) {
				preyList[p].x += z.pushX;
				preyList[p].y += z.pushY;
			}
		}
	}

	// 3. High Difficulty: Push Predators with 1.8x speed boost in currents
	for (int pr = 0; pr < predatorCount; pr++) {
		if (!predators[pr].alive) continue;
		for (int i = 0; i < level2CurrentCount; i++) {
			CurrentZone &z = level2Currents[i];
			if (predators[pr].x >= z.minX && predators[pr].x <= z.maxX &&
				predators[pr].y >= z.minY && predators[pr].y <= z.maxY) {
				predators[pr].x += z.pushX * 1.8;
				predators[pr].y += z.pushY * 1.8;
			}
		}
	}
}

// Visual indicators removed as requested
void drawCurrentIndicators() {
	return;
}

// ==== 5. CONFIGS & FUNCTIONS ====
void setupLevelConfigs() {
	levelConfigs[1] = { 2000.0, 50, 120, false, false };
	// High Difficulty Level 2: Target size 70, 75s timer, fishing nets enabled
	levelConfigs[2] = { 2600.0, 50, 75, true, false };
	levelConfigs[3] = { 3200.0, 80, 80, true, true };
}

void loadLevels() {
	setupLevelConfigs();
}

void levelInitialize(int levelNumber) {
	currentLevel = levelNumber;
	LevelConfig cfg = levelConfigs[levelNumber];
	resetPlayer(cfg.worldWidth / 2.0, WATER_SURFACE_Y - 150.0);
	spawnEntitiesForLevel(levelNumber, cfg.worldWidth);
	resetPowerups();
	resetBoats();
	resetEnvironment(cfg.worldWidth);

	if (levelNumber == 2) {
		setupLevel2Currents();
	}

	stats.level = levelNumber;
	stats.timeRemaining = cfg.timeLimitSeconds;
	stats.progress = 0.0f;
	isGameOver = false;
	isLevelComplete = false;
}

void levelUpdate() {
	LevelConfig cfg = levelConfigs[currentLevel];

	updateEntities();
	applyOceanCurrents();
	updatePowerups();
	updateBoats(cfg.worldWidth, cfg.netsEnabled, cfg.hooksEnabled);
	updateEnvironmentAnimation();
	updateBubbles();
	followPlayer();
	updatePlayerJump();
	clampPlayerToWater();

	stats.progress = (float)(player.size / cfg.targetSize);
	if (stats.progress > 1.0f) stats.progress = 1.0f;

	if (!isLevelComplete && player.size >= cfg.targetSize) {
		isLevelComplete = true;
		stats.score += stats.timeRemaining * 5;
		playWinSound();
		stopGameplayMusic();
	}
}

void tickLevelClock() {
	stats.timeRemaining--;
	if (stats.timeRemaining <= 0) {
		stats.timeRemaining = 0;
		triggerGameOver();
	}
	tickChestClock();
	tickSpeedUpClock();
	tickNetSlowClock();
}

void levelDraw() {
	drawEnvironment();
	drawCurrentIndicators();
	drawBubbles();
	drawBoats();
	drawEntities();
	drawPowerups();
	drawPlayer();
	drawHud();
}

#endif