#ifndef BOAT_HPP
#define BOAT_HPP
// =====================================================================
// boat.hpp - the fisherman's boat and everything it drops.
//
// This replaces the old pair of files (boats.hpp AND Boat.hpp), which
// both ran at the same time and fought over the same job.
//
//   Level 1 : boat sails past, drops power-ups only. Nothing harmful.
//   Level 2 : boat also drops NETS. Touching one costs a life.
//   Level 3 : boat drops a HOOK. Getting hooked starts a tug-of-war -
//             mash SPACE to escape before the fisherman reels you in.
// =====================================================================
#include "utility.hpp"
#include "player.hpp"
#include "sound.hpp"

// ==== 1. SIZES & TUNING ====
const double BOAT_W = 150.0, BOAT_H = 80.0;
const double NET_W  = 95.0,  NET_H  = 95.0;
const double HOOK_W = 46.0,  HOOK_H = 70.0;
const double SINK_SPEED = 2.4;

// Hook tug-of-war: the fisherman pulls steadily, each SPACE tap pushes
// back. Escape when your effort bar fills; get reeled in when his does.
const double PULL_PER_TICK   = 0.0055;  // fisherman's steady pulling force
const double PUSH_PER_TAP    = 0.055;   // how much one SPACE tap helps
const double STRUGGLE_DECAY  = 0.0022;  // effort slips if you stop tapping

// ==== 2. STRUCTS ====
struct Boat {
    double x, y;
    double speed;
    int    dropTimer;   // ticks until the next thing is dropped
};

struct Net {
    double x, y;
    bool   active;
};

// The hook has 3 stages: falling, holding the player (tug-of-war), and
// being pulled back up empty after a successful escape.
enum HookState { HOOK_OFF, HOOK_FALLING, HOOK_CAUGHT, HOOK_RETURNING };

struct Hook {
    double x, y;
    HookState state;
    double struggle;   // 0..1 - the player's escape progress
    double pull;       // 0..1 - the fisherman's progress
    int    shakeTick;  // drives the trapped fish's wobble
};

enum PowerKind { PW_COIN, PW_LIFE, PW_SPEED };
struct Powerup { double x, y; PowerKind kind; bool taken; };
struct Chest   { double x, y; bool active; int secsLeft; };

// ==== 3. STATE ====
#define MAX_NETS     6
#define MAX_POWERUPS 14

Boat boat;
Net  nets[MAX_NETS];
Hook hook;
Powerup powerups[MAX_POWERUPS];
int powerupCount = 0;
Chest chest;

int chestCycleSecs = 25;
int speedSecsLeft  = 0;

int boatSprite, netSprite, hookSprite;
int coinSprite, lifeSprite, speedSprite, chestSprite;

// Set per level by initBoat().
bool netsOn = false;
bool hookOn = false;

// ==== 4. LOADING / RESET ====
void loadBoat() {
    boatSprite  = loadImg("Images/Boats/boat_01.png");
    netSprite   = loadImg("Images/Boats/net_open.png", "Images/Boats/net_01.png");
    hookSprite  = loadImg("Images/Boats/hook_01.png");
    coinSprite  = loadImg("Images/Powerups/coin_01.png");
    lifeSprite  = loadImg("Images/Powerups/extra_life_01.png");
    speedSprite = loadImg("Images/Powerups/speed_up_01.png");
    chestSprite = loadImg("Images/Powerups/treasure_chest_01.png");
}

void resetBoat(int level) {
    boat.x = 0;
    boat.y = SEA_Y;
    boat.speed = 1.6 + level * 0.5;
    boat.dropTimer = 120;

    for (int i = 0; i < MAX_NETS; i++) nets[i].active = false;

    hook.state = HOOK_OFF;
    hook.struggle = 0;
    hook.pull = 0;
    hook.shakeTick = 0;

    powerupCount = 0;
    chest.active = false;
    chestCycleSecs = 25;
    speedSecsLeft = 0;

    netsOn = (level >= 2);
    hookOn = (level >= 3);
}

// ==== 5. POWER-UPS ====
void addPowerup(double x, double y, PowerKind kind) {
    // Reuse a collected slot first, so a long level never runs out.
    for (int i = 0; i < powerupCount; i++) {
        if (!powerups[i].taken) continue;
        powerups[i].x = x; powerups[i].y = y;
        powerups[i].kind = kind; powerups[i].taken = false;
        return;
    }
    if (powerupCount >= MAX_POWERUPS) return;
    Powerup p = { x, y, kind, false };
    powerups[powerupCount++] = p;
}

int spriteFor(PowerKind k) {
    if (k == PW_COIN) return coinSprite;
    if (k == PW_LIFE) return lifeSprite;
    return speedSprite;
}

void collectPowerups() {
    for (int i = 0; i < powerupCount; i++) {
        Powerup &p = powerups[i];
        if (p.taken) continue;
        if (!touching(player.x, player.y, player.size, p.x, p.y, 14)) continue;

        p.taken = true;
        playCollect();
        if (p.kind == PW_COIN) stats.score += 15;
        else if (p.kind == PW_LIFE) stats.lives++;
        else { speedSecsLeft = 10; player.speed = BASE_SPEED * 2.0; }
    }

    if (chest.active && touching(player.x, player.y, player.size, chest.x, chest.y, 20)) {
        stats.score += 120;
        chest.active = false;
        playCollect();
    }
}

// ==== 6. NETS (level 2+) ====
void dropNet(double x) {
    for (int i = 0; i < MAX_NETS; i++) {
        if (nets[i].active) continue;
        nets[i].x = x;
        nets[i].y = SEA_Y;
        nets[i].active = true;
        return;
    }
}

void updateNets(double worldWidth) {
    for (int i = 0; i < MAX_NETS; i++) {
        Net &n = nets[i];
        if (!n.active) continue;

        n.y -= SINK_SPEED;
        if (n.y < FLOOR_Y) { n.active = false; continue; }

        // A net costs ONE life - never an instant game over.
        if (!player.respawning && player.safeTicks == 0 &&
            touching(player.x, player.y, player.size, n.x, n.y, NET_W * 0.35)) {
            n.active = false;
            loseLife(worldWidth);
        }
    }
}

// ==== 7. HOOK TUG-OF-WAR (level 3) ====
void dropHook(double x) {
    if (hook.state != HOOK_OFF) return;   // only one hook at a time
    hook.x = x;
    hook.y = SEA_Y;
    hook.state = HOOK_FALLING;
    hook.struggle = 0;
    hook.pull = 0;
    hook.shakeTick = 0;
}

bool playerIsHooked() { return hook.state == HOOK_CAUGHT; }

// Called whenever SPACE is tapped while hooked.
void hookSpaceTap() {
    if (hook.state != HOOK_CAUGHT) return;
    hook.struggle += PUSH_PER_TAP;
    if (hook.struggle > 1.0) hook.struggle = 1.0;
    spawnSwimBubble(player.x + randRange(-12, 12), player.y);
}

void updateHook(double worldWidth) {
    if (!hookOn || hook.state == HOOK_OFF) return;

    if (hook.state == HOOK_FALLING) {
        hook.y -= SINK_SPEED;
        if (hook.y < FLOOR_Y) { hook.state = HOOK_OFF; return; }

        if (!player.respawning && player.safeTicks == 0 &&
            touching(player.x, player.y, player.size, hook.x, hook.y, HOOK_W * 0.5)) {
            hook.state = HOOK_CAUGHT;
            hook.struggle = 0;
            hook.pull = 0;
        }
        return;
    }

    if (hook.state == HOOK_CAUGHT) {
        // The trapped fish is dragged to the hook and shakes in place.
        hook.shakeTick++;
        player.x = hook.x + sin(hook.shakeTick * 0.55) * 7.0;
        player.y = hook.y;

        hook.pull += PULL_PER_TICK;
        hook.struggle -= STRUGGLE_DECAY;      // stop tapping and you slip back
        if (hook.struggle < 0) hook.struggle = 0;

        // Escaped - the hook goes back up empty.
        if (hook.struggle >= 1.0) {
            hook.state = HOOK_RETURNING;
            player.safeTicks = 50;
            playCollect();
            return;
        }
        // Reeled in - costs one life, then the normal respawn happens.
        if (hook.pull >= 1.0) {
            hook.state = HOOK_OFF;
            loseLife(worldWidth);
        }
        return;
    }

    if (hook.state == HOOK_RETURNING) {
        hook.y += SINK_SPEED * 1.6;
        if (hook.y >= SEA_Y) hook.state = HOOK_OFF;
    }
}

// The two-colour struggle bar shown next to a hooked fish. Green is
// your tapping, red is the fisherman pulling.
void drawHookBar() {
    if (hook.state != HOOK_CAUGHT) return;

    double bx = toScreenX(player.x) + player.size + 18;
    double by = toScreenY(player.y) - 40;
    double w = 26, h = 80;

    drawPanel(bx, by, w, h, 15, 20, 30, 220, 220, 220);

    // Fisherman's pull grows down from the top (red).
    iSetColor(220, 60, 50);
    iFilledRectangle(bx + 2, by + h - 2 - (h - 4) * hook.pull, w - 4, (h - 4) * hook.pull);

    // Your effort grows up from the bottom (green).
    iSetColor(70, 220, 110);
    iFilledRectangle(bx + 2, by + 2, w - 4, (h - 4) * hook.struggle);

    drawText(bx - 16, by + h + 10, "SPACE!", 255, 240, 120);
}

// ==== 8. BOAT ====
void initBoat(int level) { resetBoat(level); }

void updateBoat(double worldWidth) {
    boat.x += boat.speed;
    if (boat.x > worldWidth + BOAT_W) boat.x = -BOAT_W;

    boat.dropTimer--;
    if (boat.dropTimer <= 0) {
        boat.dropTimer = 150 + rand() % 180;
        int roll = rand() % 10;

        if (hookOn && roll < 3)      dropHook(boat.x);
        else if (netsOn && roll < 6) dropNet(boat.x);
        else if (roll == 6)          addPowerup(boat.x, SEA_Y + 34, PW_LIFE);
        else if (roll == 7 || roll == 8) addPowerup(boat.x, SEA_Y + 34, PW_SPEED);
        else                          addPowerup(boat.x, SEA_Y + 34, PW_COIN);
    }

    updateNets(worldWidth);
    updateHook(worldWidth);
    collectPowerups();
}

// ==== 9. PER-SECOND TIMERS ====
void spawnChest() {
    chest.x = scrollX + randRange(-CENTER_X + 80, CENTER_X - 80);
    chest.y = randRange(FLOOR_Y + 60, SEA_Y - 60);
    chest.active = true;
    chest.secsLeft = 10;
}

void tickBoatClocks() {
    if (chest.active) {
        chest.secsLeft--;
        if (chest.secsLeft <= 0) { chest.active = false; chestCycleSecs = 25; }
    }
    else {
        chestCycleSecs--;
        if (chestCycleSecs <= 0) spawnChest();
    }

    if (speedSecsLeft > 0) {
        speedSecsLeft--;
        if (speedSecsLeft == 0) player.speed = BASE_SPEED;
    }
}

// ==== 10. DRAWING ====
void drawBoat() {
    iShowImage((int)(toScreenX(boat.x) - BOAT_W / 2), (int)(SEA_Y - 6),
               (int)BOAT_W, (int)BOAT_H, boatSprite);
}

void drawNetsAndHook() {
    for (int i = 0; i < MAX_NETS; i++) {
        if (!nets[i].active) continue;
        iShowImage((int)(toScreenX(nets[i].x) - NET_W / 2),
                   (int)(toScreenY(nets[i].y) - NET_H / 2),
                   (int)NET_W, (int)NET_H, netSprite);
    }

    if (hook.state == HOOK_OFF) return;

    // The fishing line, drawn from the boat down to the hook.
    iSetColor(235, 235, 220);
    iLine(toScreenX(hook.x), SEA_Y, toScreenX(hook.x), toScreenY(hook.y));
    iShowImage((int)(toScreenX(hook.x) - HOOK_W / 2),
               (int)(toScreenY(hook.y) - HOOK_H / 2),
               (int)HOOK_W, (int)HOOK_H, hookSprite);
}

void drawPowerups() {
    for (int i = 0; i < powerupCount; i++) {
        if (powerups[i].taken) continue;
        iShowImage((int)(toScreenX(powerups[i].x) - 16),
                   (int)(toScreenY(powerups[i].y) - 16), 32, 32, spriteFor(powerups[i].kind));
    }

    if (!chest.active) return;
    iShowImage((int)(toScreenX(chest.x) - 22), (int)(toScreenY(chest.y) - 20), 44, 40, chestSprite);
    char buf[16];
    sprintf_s(buf, "%ds", chest.secsLeft);
    drawText(toScreenX(chest.x) - 10, toScreenY(chest.y) + 30, buf, 255, 225, 90);
}

#endif
