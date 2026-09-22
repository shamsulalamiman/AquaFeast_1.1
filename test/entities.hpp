#ifndef ENTITIES_HPP
#define ENTITIES_HPP
// =====================================================================
// entities.hpp - prey fish (food) and predators (danger).
//
// Every level uses the SAME movement code. Levels only change the
// NUMBERS (how many, how fast, how big) - never the logic.
//
// Every swimmer only ever shows its LEFT or RIGHT picture, even while
// moving up/down - sideOnly() below picks the facing from the
// horizontal direction only, so no up/down artwork is ever needed.
//
// Fish animate by cycling through a few sprite frames, so they look
// like they are really swimming rather than sliding along.
// =====================================================================
#include "utility.hpp"
#include "player.hpp"
#include "sound.hpp"

#define MAX_PREY      60
#define MAX_PREDATORS 26
#define PREY_LOOKS     3
#define SWIM_FRAMES    3    // how many pictures make one swimming cycle

const double CHASE_RANGE = 240.0;  // predator notices you inside this
const int    WARN_TICKS  = 22;     // warning shown before it charges

// ==== 1. STRUCTS ====
struct Prey {
    double x, y;
    double size;
    double speed;
    double dx, dy;      // swim direction
    int    turnTicks;   // ticks until it picks a new direction
    int    look;        // which of the 3 prey types
    int    animTick;    // drives the swimming animation
    Facing facing;       // always FACE_RIGHT or FACE_LEFT - see sideOnly()
    bool   alive;
};

struct Predator {
    double x, y;
    double size;
    double speed;
    double dx, dy;
    int    turnTicks;
    int    kind;        // 0,1 = normal predators. 2 = the level's DANGER enemy
    int    animTick;
    Facing facing;       // always FACE_RIGHT or FACE_LEFT - see sideOnly()
    bool   alive;
    bool   chasing;
    int    warnTicks;
};

Prey preys[MAX_PREY];
int preyCount = 0;
Predator preds[MAX_PREDATORS];
int predCount = 0;

// Sprites. [look/kind][frame][0=right, 1=left] - no up/down slot at all.
int preySprite[PREY_LOOKS][SWIM_FRAMES][2];
int predSprite[3][SWIM_FRAMES][2];
int warnSprite, dangerSprite;

// ==== 2. LOADING ====
// Loads one fish's animation frames. Frame 1 is the base picture you
// already have (e.g. prey_small_01.png); frames 2 and 3 are optional
// extras named _f2 / _f3. Any missing frame falls back to frame 1, so
// the fish simply glides instead of flapping - it never breaks. Used
// for BOTH prey and predators, since both only ever need right/left.
void loadSwimFrames(int out[SWIM_FRAMES][2], const char* baseRight, const char* baseLeft) {
    char path[160], stem[160];

    // If there is no left-facing picture at all, every left frame falls
    // back to the right-facing one rather than to a blank texture.
    const char* leftBase = imageExists(baseLeft) ? baseLeft : baseRight;

    for (int f = 0; f < SWIM_FRAMES; f++) {
        if (f == 0) {
            out[f][0] = loadImg(baseRight);
            out[f][1] = loadImg(leftBase);
            continue;
        }
        // Build "name_f2.png" from "name.png"
        strcpy_s(stem, baseRight);
        char* dot = strrchr(stem, '.');
        if (dot) *dot = '\0';
        sprintf_s(path, "%s_f%d.png", stem, f + 1);
        out[f][0] = loadImg(path, baseRight);

        strcpy_s(stem, leftBase);
        dot = strrchr(stem, '.');
        if (dot) *dot = '\0';
        sprintf_s(path, "%s_f%d.png", stem, f + 1);
        out[f][1] = loadImg(path, leftBase);
    }
}

void loadEntities() {
    loadSwimFrames(preySprite[0], "Images/Fish/prey_small_01.png",  "Images/Fish/prey_small_01_left.png");
    loadSwimFrames(preySprite[1], "Images/Fish/prey_medium_01.png", "Images/Fish/prey_medium_01_left.png");
    loadSwimFrames(preySprite[2], "Images/Fish/prey_small_02.png",  "Images/Fish/prey_small_02_left.png");

    warnSprite   = loadImg("Images/HUD/warning_icon.png");
    dangerSprite = loadImg("Images/HUD/danger_icon.png", "Images/HUD/warning_icon.png");

    loadSwimFrames(predSprite[0], "Images/Predators/predator_01_right.png", "Images/Predators/predator_01_left.png");
    loadSwimFrames(predSprite[1], "Images/Predators/predator_02_right.png", "Images/Predators/predator_02_left.png");
    // Kind 2 is the "danger" enemy that appears halfway through a level.
    loadSwimFrames(predSprite[2], "Images/Predators/danger_01_right.png", "Images/Predators/danger_01_left.png");
}

// Picks left/right purely from the horizontal direction, ignoring any
// vertical movement entirely - this is what keeps every fish showing
// only its left/right picture, even while swimming straight up or down.
// dx near 0 (moving mostly vertically) keeps whatever it was already
// facing, instead of flickering.
Facing sideOnly(double dx, Facing prevFacing) {
    if (dx > 0.05)  return FACE_RIGHT;
    if (dx < -0.05) return FACE_LEFT;
    return prevFacing;
}

// ==== 3. SPAWNING ====
void addPrey(double x, double y, double size, double speed) {
    if (preyCount >= MAX_PREY) return;
    Prey f;
    f.x = x; f.y = y;
    f.size = size;
    f.speed = speed;
    f.look = rand() % PREY_LOOKS;
    f.animTick = rand() % 60;      // stagger so they don't flap in unison
    randomDir(f.dx, f.dy, f.turnTicks);
    f.facing = (f.dx >= 0) ? FACE_RIGHT : FACE_LEFT;
    f.alive = true;
    preys[preyCount++] = f;
}

void addPredator(double x, double y, double size, double speed, int kind) {
    if (predCount >= MAX_PREDATORS) return;
    Predator p;
    p.x = x; p.y = y;
    p.size = size;
    p.speed = speed;
    p.kind = kind;
    p.animTick = rand() % 60;
    randomDir(p.dx, p.dy, p.turnTicks);
    p.facing = (p.dx >= 0) ? FACE_RIGHT : FACE_LEFT;
    p.alive = true;
    p.chasing = false;
    p.warnTicks = 0;
    preds[predCount++] = p;
}

// ==== 4. MOVEMENT (identical for every level) ====
void keepInWater(double &y, double &dy, double size) {
    if (y > SEA_Y - size)   { y = SEA_Y - size;   dy = -fabs(dy); }
    if (y < FLOOR_Y + size) { y = FLOOR_Y + size; dy =  fabs(dy); }
}

void keepInWorld(double &x, double &dx, double size, double worldWidth) {
    if (x > worldWidth - size) { x = worldWidth - size; dx = -fabs(dx); }
    if (x < size)              { x = size;              dx =  fabs(dx); }
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
    for (int i = 0; i < preyCount; i++) {
        Prey &f = preys[i];
        if (!f.alive) continue;
        wander(f.x, f.y, f.dx, f.dy, f.turnTicks, f.speed, f.size, worldWidth);
        f.facing = sideOnly(f.dx, f.facing);
        f.animTick++;
    }
}

// True when this predator can actually hurt the player. If the player
// has grown bigger, it is food - so no warning is shown for it.
bool isDangerous(const Predator &p) { return p.size >= player.size; }

void updatePredators(double worldWidth) {
    for (int i = 0; i < predCount; i++) {
        Predator &p = preds[i];
        if (!p.alive) continue;
        p.animTick++;

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
            p.facing = sideOnly(dx, p.facing);
            p.y = clampD(p.y, FLOOR_Y + p.size, SEA_Y - p.size);
            p.x = clampD(p.x, p.size, worldWidth - p.size);
        }
        else {
            wander(p.x, p.y, p.dx, p.dy, p.turnTicks, p.speed * 0.5, p.size, worldWidth);
            p.facing = sideOnly(p.dx, p.facing);
        }
    }
}

// Makes every predator faster - used when the level's second half
// begins and the danger enemy shows up.
void speedUpPredators(double multiplier) {
    for (int i = 0; i < predCount; i++) preds[i].speed *= multiplier;
}

int countLivePrey() {
    int n = 0;
    for (int i = 0; i < preyCount; i++) if (preys[i].alive) n++;
    return n;
}

// Brings an eaten fish back somewhere else in the level. Without this
// the ocean would slowly empty out and there would not be enough food
// left to reach the bigger size goals of levels 2 and 3.
void respawnOnePrey(double worldWidth, double size, double speed) {
    for (int i = 0; i < preyCount; i++) {
        if (preys[i].alive) continue;
        Prey &f = preys[i];
        // Come back well away from the player so nothing pops up in
        // their face.
        double x = randRange(100, worldWidth - 100);
        if (fabs(x - player.x) < 420)
            x = (x < player.x) ? clampD(player.x - 520, 100, worldWidth - 100)
                               : clampD(player.x + 520, 100, worldWidth - 100);
        f.x = x;
        f.y = randRange(FLOOR_Y + 80, SEA_Y - 80);
        f.size = size;
        f.speed = speed;
        f.look = rand() % PREY_LOOKS;
        f.animTick = rand() % 60;
        randomDir(f.dx, f.dy, f.turnTicks);
        f.facing = sideOnly(f.dx, f.facing);
        f.alive = true;
        return;
    }
}

// ==== 5. EATING ====
void eatPrey() {
    for (int i = 0; i < preyCount; i++) {
        Prey &f = preys[i];
        if (!f.alive) continue;
        if (!touching(player.x, player.y, player.size, f.x, f.y, f.size)) continue;

        if (player.size > f.size) {
            f.alive = false;
            growPlayer(1.6);
            stats.score += (int)(f.size * 2);
            spawnSwimBubble(f.x, f.y);      // little puff where it was eaten
            playEat();
        }
    }
}

void hitPredators(double worldWidth) {
    if (!playerCanMove()) return;   // safe while respawning or hooked

    for (int i = 0; i < predCount; i++) {
        Predator &p = preds[i];
        if (!p.alive) continue;
        if (!touching(player.x, player.y, player.size, p.x, p.y, p.size)) continue;

        if (player.size > p.size) {
            p.alive = false;
            growPlayer(3.4);
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
// Cycles through the frames at a steady pace to animate the swim.
int frameOf(int animTick) { return (animTick / 8) % SWIM_FRAMES; }

void drawEntities() {
    for (int i = 0; i < preyCount; i++) {
        Prey &f = preys[i];
        if (!f.alive) continue;
        double sx = toScreenX(f.x);
        if (sx < -80 || sx > SCREEN_W + 80) continue;   // skip off-screen
        double s = f.size * 2.0;
        int dir = (f.facing == FACE_LEFT) ? 1 : 0;
        iShowImage((int)(sx - s / 2), (int)(toScreenY(f.y) - s / 2), (int)s, (int)s,
                   preySprite[f.look][frameOf(f.animTick)][dir]);
    }

    for (int i = 0; i < predCount; i++) {
        Predator &p = preds[i];
        if (!p.alive) continue;
        double sx = toScreenX(p.x);
        if (sx < -120 || sx > SCREEN_W + 120) continue;
        double s = p.size * 2.0;
        int dir = (p.facing == FACE_LEFT) ? 1 : 0;
        iShowImage((int)(sx - s / 2), (int)(toScreenY(p.y) - s / 2), (int)s, (int)s,
                   predSprite[p.kind][frameOf(p.animTick)][dir]);

        // The warning only appears for enemies that can actually eat
        // you. Once you outgrow one, it stops being marked as a threat.
        if (p.warnTicks > 0 && isDangerous(p)) {
            int icon = (p.kind == 2) ? dangerSprite : warnSprite;
            iShowImage((int)(sx - 16), (int)(toScreenY(p.y) + s / 2 + 8), 32, 32, icon);
        }
    }
}

#endif
