#ifndef ENTITIES_HPP
#define ENTITIES_HPP
// =====================================================================
// entities.hpp - prey fish (food) and predators (danger).
// =====================================================================
#include "utility.hpp"
#include "player.hpp"
#include "sound.hpp"

#define MAX_PREY      45
#define MAX_PREDATORS 10
#define PREY_LOOKS     3

const double CHASE_RANGE = 190.0;  // predator notices you inside this
const int    WARN_TICKS = 22;     // warning shown before it charges

// ==== 1. STRUCTS ====
struct Prey {
	double x, y;
	double size;
	double speed;
	double dx, dy;      // swim direction
	int    turnTicks;   // ticks until it picks a new direction
	int    look;        // which of the 3 prey images
	Facing facing;
	bool   alive;
};

struct Predator {
	double x, y;
	double size;
	double speed;
	double dx, dy;
	int    turnTicks;
	int    kind;        // 0 or 1 - which predator artwork
	Facing facing;
	bool   alive;
	bool   chasing;
	int    warnTicks;
};

Prey preys[MAX_PREY];
int preyCount = 0;
Predator preds[MAX_PREDATORS];
int predCount = 0;

int preyRight[PREY_LOOKS], preyLeft[PREY_LOOKS];
int predSprite[2][4];     // [kind][facing]
int warnSprite;

// ==== 2. LOADING ====
void loadEntities() {
	preyRight[0] = loadImg("Images/Fish/prey_small_01.png");
	preyLeft[0] = loadImg("Images/Fish/prey_small_01_left.png", "Images/Fish/prey_small_01.png");
	preyRight[1] = loadImg("Images/Fish/prey_medium_01.png");
	preyLeft[1] = loadImg("Images/Fish/prey_medium_01_left.png", "Images/Fish/prey_medium_01.png");
	preyRight[2] = loadImg("Images/Fish/prey_small_02.png");
	preyLeft[2] = loadImg("Images/Fish/prey_small_02_left.png", "Images/Fish/prey_small_02.png");

	warnSprite = loadImg("Images/HUD/warning_icon.png");

	predSprite[0][FACE_RIGHT] = loadImg("Images/Predators/predator_01_right.png");
	predSprite[0][FACE_LEFT] = loadImg("Images/Predators/predator_01_left.png");
	predSprite[0][FACE_UP] = loadImg("Images/Predators/predator_01_up.png", "Images/Predators/predator_01_right.png");
	predSprite[0][FACE_DOWN] = loadImg("Images/Predators/predator_01_down.png", "Images/Predators/predator_01_right.png");

	predSprite[1][FACE_RIGHT] = loadImg("Images/Predators/predator_02_right.png");
	predSprite[1][FACE_LEFT] = loadImg("Images/Predators/predator_02_left.png");
	predSprite[1][FACE_UP] = loadImg("Images/Predators/predator_02_up.png", "Images/Predators/predator_02_right.png");
	predSprite[1][FACE_DOWN] = loadImg("Images/Predators/predator_02_down.png", "Images/Predators/predator_02_right.png");
}

// ==== 3. SPAWNING ====
void addPrey(double x, double y, double size, double speed) {
	int index = -1;

	// Find an empty slot (either at the end, or replacing a dead fish)
	if (preyCount < MAX_PREY) {
		index = preyCount++;
	}
	else {
		for (int i = 0; i < MAX_PREY; i++) {
			if (!preys[i].alive) {
				index = i;
				break;
			}
		}
	}

	if (index == -1) return; // Abort if completely full

	Prey f;
	f.x = x; f.y = y;
	f.size = size;
	f.speed = speed;
	f.look = rand() % PREY_LOOKS;
	randomDir(f.dx, f.dy, f.turnTicks);
	f.facing = facingOf(f.dx, f.dy);
	f.alive = true;

	preys[index] = f;
}

void addPredator(double x, double y, double size, double speed, int kind) {
	int index = -1;

	if (predCount < MAX_PREDATORS) {
		index = predCount++;
	}
	else {
		for (int i = 0; i < MAX_PREDATORS; i++) {
			if (!preds[i].alive) {
				index = i;
				break;
			}
		}
	}

	if (index == -1) return;

	Predator p;
	p.x = x; p.y = y;
	p.size = size;
	p.speed = speed;
	p.kind = kind;
	randomDir(p.dx, p.dy, p.turnTicks);
	p.facing = facingOf(p.dx, p.dy);
	p.alive = true;
	p.chasing = false;
	p.warnTicks = 0;

	preds[index] = p;
}

// ==== 4. MOVEMENT ====
void keepInWater(double &y, double &dy, double size) {
	if (y > SEA_Y - size)     { y = SEA_Y - size;     dy = -fabs(dy); }
	if (y < FLOOR_Y + size)   { y = FLOOR_Y + size;   dy = fabs(dy); }
}

void keepInWorld(double &x, double &dx, double size, double worldWidth) {
	if (x > worldWidth - size) { x = worldWidth - size; dx = -fabs(dx); }
	if (x < size)              { x = size;              dx = fabs(dx); }
}

void wander(double &x, double &y, double &dx, double &dy,
	int &turnTicks, double speed, double size, double worldWidth) {
	x += dx * speed;
	y += dy * speed;

	turnTicks--;
	if (turnTicks <= 0) randomDir(dx, dy, turnTicks);

	keepInWater(y, dy, size);
	keepInWorld(x, dx, size, worldWidth);
}

void updatePrey(double worldWidth) {
	for (int i = 0; i < MAX_PREY; i++) {
		Prey &f = preys[i];
		if (!f.alive) continue;
		wander(f.x, f.y, f.dx, f.dy, f.turnTicks, f.speed, f.size, worldWidth);
		f.facing = facingOf(f.dx, f.dy);
	}
}

void updatePredators(double worldWidth) {
	for (int i = 0; i < MAX_PREDATORS; i++) {
		Predator &p = preds[i];
		if (!p.alive) continue;

		// FIXED: Renamed 'near' to 'isNear'
		bool isNear = dist(p.x, p.y, player.x, player.y) < CHASE_RANGE;

		if (isNear && !p.chasing) {
			if (p.warnTicks == 0) p.warnTicks = WARN_TICKS;
			p.warnTicks--;
			if (p.warnTicks <= 0) p.chasing = true;
		}
		else if (!isNear) {
			p.chasing = false;
			p.warnTicks = 0;
		}

		if (p.chasing) {
			double dx = player.x - p.x, dy = player.y - p.y;
			double len = sqrt(dx * dx + dy * dy);
			if (len > 0.001) { dx /= len; dy /= len; }
			p.x += dx * p.speed;
			p.y += dy * p.speed;
			p.facing = facingOf(dx, dy);
			p.y = clampD(p.y, FLOOR_Y + p.size, SEA_Y - p.size);
			p.x = clampD(p.x, p.size, worldWidth - p.size);
		}
		else {
			wander(p.x, p.y, p.dx, p.dy, p.turnTicks, p.speed * 0.5, p.size, worldWidth);
			p.facing = facingOf(p.dx, p.dy);
		}
	}
}

// ==== 5. EATING ====
void eatPrey() {
	for (int i = 0; i < MAX_PREY; i++) {
		Prey &f = preys[i];
		if (!f.alive) continue;
		if (!touching(player.x, player.y, player.size, f.x, f.y, f.size)) continue;

		if (player.size > f.size) {
			f.alive = false;
			growPlayer(1.4);
			stats.score += (int)(f.size * 2);
			playEat();
		}
	}
}

void hitPredators(double worldWidth) {
	if (player.respawning) return;

	for (int i = 0; i < MAX_PREDATORS; i++) {
		Predator &p = preds[i];
		if (!p.alive) continue;
		if (!touching(player.x, player.y, player.size, p.x, p.y, p.size)) continue;

		if (player.size > p.size) {
			p.alive = false;
			growPlayer(3.0);
			stats.score += (int)(p.size * 3);
			playEat();
		}
		else {
			loseLife(worldWidth);
			return;   // one hit per tick at most
		}
	}
}

void updateEntities(double worldWidth) {
	updatePrey(worldWidth);
	updatePredators(worldWidth);
	eatPrey();
	hitPredators(worldWidth);
}

// ==== 6. DRAWING ====
void drawEntities() {
	for (int i = 0; i < MAX_PREY; i++) {
		Prey &f = preys[i];
		if (!f.alive) continue;
		double s = f.size * 2.0;
		double sx = toScreenX(f.x);
		if (sx < -60 || sx > SCREEN_W + 60) continue;   // skip off-screen
		int sprite = (f.facing == FACE_LEFT) ? preyLeft[f.look] : preyRight[f.look];
		iShowImage((int)(sx - s / 2), (int)(toScreenY(f.y) - s / 2), (int)s, (int)s, sprite);
	}

	for (int i = 0; i < MAX_PREDATORS; i++) {
		Predator &p = preds[i];
		if (!p.alive) continue;
		double s = p.size * 2.0;
		double sx = toScreenX(p.x);
		if (sx < -80 || sx > SCREEN_W + 80) continue;
		iShowImage((int)(sx - s / 2), (int)(toScreenY(p.y) - s / 2), (int)s, (int)s,
			predSprite[p.kind][p.facing]);

		if (p.warnTicks > 0)
			iShowImage((int)(sx - 14), (int)(toScreenY(p.y) + s / 2 + 6), 28, 28, warnSprite);
	}
}

#endif