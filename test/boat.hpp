#ifndef BOAT_HPP
#define BOAT_HPP
// =====================================================================
// boat.hpp - the fisherman's boat and everything it drops.
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
const double BOAT_W = 150.0, BOAT_H = 150.0;
const double NET_W  = 100.0, NET_H  = 100.0;
const double HOOK_W = 40.0,  HOOK_H = 40.0;
const double SINK_SPEED = 2.6;

// Hook tug-of-war. The fisherman pulls steadily; each SPACE tap pushes
// back. Escape when your effort bar fills; reeled in when his fills.
const double PULL_PER_TICK  = 0.0045;   // fisherman's steady pull
const double PUSH_PER_TAP   = 0.075;    // how much one SPACE tap helps
const double EFFORT_DECAY   = 0.0020;   // effort slips if you stop tapping

// ==== 2. STRUCTS ====
struct Boat {
    double x, y;
    double speed;
    int    dropTimer;
};

struct Net {
    double x, y;
    bool   active;
};

// The hook is a small state machine. Having ONE state variable (rather
// than several loose bools) is what stops the old bugs: a stale hook
// left behind after a restart, a life lost twice from one hook, or an
// escape firing more than once.
enum HookState {
    HOOK_OFF,        // not in play at all
    HOOK_FALLING,    // sinking, looking for the player
    HOOK_CAUGHT,     // holding the player - tug-of-war running
    HOOK_ESCAPED,    // player won; hook rising back up, cannot re-catch
    HOOK_REELING     // player lost; hook rising with the fish
};

struct Hook {
    double x, y;
    HookState state;
    double effort;     // 0..1 the player's escape progress
    double pull;       // 0..1 the fisherman's progress
    int    cooldown;   // ticks before another hook may be dropped
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

// Puts the hook fully back to "not in play". Called on reset and
// whenever a hook finishes, so no stale state can survive a restart.
void clearHook() {
    hook.state = HOOK_OFF;
    hook.effort = 0;
    hook.pull = 0;
    hook.cooldown = 0;
    hook.y = SEA_Y;
    stopShaking();
    stopStruggle();
}

void resetBoat(int level) {
    boat.x = 0;
    boat.y = SEA_Y;
    boat.speed = 1.6 + level * 0.5;
    boat.dropTimer = 150;

    for (int i = 0; i < MAX_NETS; i++) nets[i].active = false;

    clearHook();

    powerupCount = 0;
    chest.active = false;
    chestCycleSecs = 25;
    speedSecsLeft = 0;

    netsOn = (level >= 2);
    hookOn = (level >= 3);
}

// ==== 5. POWER-UPS ====
void addPowerup(double x, double y, PowerKind kind) {
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
        if (!touching(player.x, player.y, player.size, p.x, p.y, 18)) continue;

        p.taken = true;
        playCollect();
        if (p.kind == PW_COIN) stats.score += 15;
        else if (p.kind == PW_LIFE) stats.lives++;
        else { speedSecsLeft = 10; player.speed = BASE_SPEED * 2.0; }
    }

    if (chest.active && touching(player.x, player.y, player.size, chest.x, chest.y, 26)) {
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

        // A net costs ONE life - never an instant game over. The net is
        // removed on the same tick it hits, so it cannot hit twice.
        if (playerCanMove() && player.safeTicks == 0 &&
            touching(player.x, player.y, player.size, n.x, n.y, NET_W * 0.34)) {
            n.active = false;
            loseLife(worldWidth);
        }
    }
}

// ==== 7. HOOK TUG-OF-WAR (level 3) ====
void dropHook(double x) {
    if (hook.state != HOOK_OFF || hook.cooldown > 0) return;   // one at a time
    hook.x = x;
    hook.y = SEA_Y;
    hook.state = HOOK_FALLING;
    hook.effort = 0;
    hook.pull = 0;
}

bool playerIsHooked() { return hook.state == HOOK_CAUGHT; }

// Called when SPACE is tapped while hooked. Only HOOK_CAUGHT responds,
// so taps during the escape animation do nothing.
void hookSpaceTap() {
    if (hook.state != HOOK_CAUGHT) return;
    hook.effort += PUSH_PER_TAP;
    if (hook.effort > 1.0) hook.effort = 1.0;
    spawnSwimBubble(player.x + randRange(-14, 14), player.y);
}

// The fish breaks free: shaking and sound stop, the hook rises away
// empty, and normal swimming is restored immediately.
void hookEscape() {
    hook.state = HOOK_ESCAPED;
    hook.effort = 0;
    hook.pull = 0;
    stopShaking();
    stopStruggle();
    player.safeTicks = 60;      // brief safety so it cannot re-catch at once
    playCollect();
}

// The fisherman wins: one life lost, then the normal respawn animation.
void hookReelIn(double worldWidth) {
    hook.state = HOOK_REELING;
    hook.effort = 0;
    hook.pull = 0;
    stopShaking();
    stopStruggle();
    loseLife(worldWidth);
}

void updateHook(double worldWidth) {
    if (hook.cooldown > 0) hook.cooldown--;
    if (!hookOn || hook.state == HOOK_OFF) return;

    if (hook.state == HOOK_FALLING) {
        hook.y -= SINK_SPEED;
        if (hook.y < FLOOR_Y) { clearHook(); hook.cooldown = 120; return; }

        if (playerCanMove() && player.safeTicks == 0 &&
            touching(player.x, player.y, player.size, hook.x, hook.y, HOOK_W * 0.5)) {
            hook.state = HOOK_CAUGHT;
            hook.effort = 0;
            hook.pull = 0;
            startShaking();
        }
        return;
    }

    if (hook.state == HOOK_CAUGHT) {
        // The trapped fish is held at the hook and shakes in place.
        player.x = hook.x;
        player.y = hook.y;
        updateShaking();

        hook.pull += PULL_PER_TICK;

        // Check the max BEFORE decay is applied. hookSpaceTap() already
        // clamps effort to exactly 1.0 the instant it reaches the max,
        // so checking here - before anything reduces it again - is what
        // guarantees the escape is detected on that same tick, every
        // time, even though the bar is clamped at its maximum value.
        if (hook.effort >= 1.0)    { hookEscape(); return; }
        if (hook.pull >= 1.0)      { hookReelIn(worldWidth); return; }

        hook.effort -= EFFORT_DECAY;          // stop tapping and you slip back
        if (hook.effort < 0) hook.effort = 0;
        return;
    }

    // ESCAPED or REELING: the hook simply rises back to the boat.
    hook.y += SINK_SPEED * 1.8;
    if (hook.y >= SEA_Y) {
        clearHook();
        hook.cooldown = 150;      // a pause before the next hook is dropped
    }
}

// The two bars shown above a hooked fish: green is your SPACE effort,
// red is the fisherman's pull. Both fill LEFT TO RIGHT, so reaching the
// max is reaching the bar's right edge - tap fast enough to fill yours
// before his and the fish escapes.
void drawHookBars() {
    if (hook.state != HOOK_CAUGHT) return;

    double w = 170, h = 22, gap = 8;
    double bx = toScreenX(player.x) - w / 2;
    double by = toScreenY(player.y) + player.size + 46;

    // Player Effort (green), fills left -> right.
    drawPanel(bx, by, w, h, 12, 18, 28, 220, 230, 240);
    iSetColor(70, 225, 115);
    iFilledRectangle(bx + 3, by + 3, (w - 6) * hook.effort, h - 6);

    // Boat Pull (red), fills left -> right, stacked below.
    double by2 = by - h - gap;
    drawPanel(bx, by2, w, h, 12, 18, 28, 220, 230, 240);
    iSetColor(225, 70, 55);
    iFilledRectangle(bx + 3, by2 + 3, (w - 6) * hook.pull, h - 6);

    drawText(bx, by + h + 8, "YOUR EFFORT - mash SPACE!", 130, 240, 160);
    drawText(bx, by2 - 20, "BOAT PULL", 240, 130, 115);
}

// ==== 8. BOAT ====
void updateBoat(double worldWidth) {
    boat.x += boat.speed;
    if (boat.x > worldWidth + BOAT_W) boat.x = -BOAT_W;

    boat.dropTimer--;
    if (boat.dropTimer <= 0) {
        boat.dropTimer = 150 + rand() % 180;
        int roll = rand() % 10;

        if (hookOn && roll < 3)      dropHook(boat.x);
        else if (netsOn && roll < 6) dropNet(boat.x);
        else if (roll == 6)          addPowerup(boat.x, SEA_Y + 40, PW_LIFE);
        else if (roll == 7 || roll == 8) addPowerup(boat.x, SEA_Y + 40, PW_SPEED);
        else                          addPowerup(boat.x, SEA_Y + 40, PW_COIN);
    }

    updateNets(worldWidth);
    updateHook(worldWidth);
    collectPowerups();
}

// ==== 9. PER-SECOND TIMERS ====
void spawnChest() {
    chest.x = scrollX + randRange(-CENTER_X + 100, CENTER_X - 100);
    chest.y = randRange(FLOOR_Y + 80, SEA_Y - 80);
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
    iShowImage((int)(toScreenX(boat.x) - BOAT_W / 2), (int)(SEA_Y - 15),
               (int)BOAT_W, (int)BOAT_H, boatSprite);
}

void drawNets() {
    for (int i = 0; i < MAX_NETS; i++) {
        if (!nets[i].active) continue;
        iShowImage((int)(toScreenX(nets[i].x) - NET_W / 2),
                   (int)(toScreenY(nets[i].y) - NET_H / 2),
                   (int)NET_W, (int)NET_H, netSprite);
    }
}

// Drawn AFTER the player (see levels.hpp) so the hook is clearly
// visible attached to the fish instead of being hidden behind it.
void drawHook() {
    if (hook.state == HOOK_OFF) return;

    // The line gets brighter/thicker as the boat's pull builds up, so
    // the pulling effort actually reads as tension rather than a plain
    // static line.
    if (hook.state == HOOK_CAUGHT) {
        int pullGlow = (int)(hook.pull * 255);
        iSetColor(235, 235 - pullGlow / 3, 220 - pullGlow / 2);
    } else {
        iSetColor(220, 220, 205);
    }
    iLine(toScreenX(hook.x), SEA_Y, toScreenX(hook.x), toScreenY(hook.y));

    iShowImage((int)(toScreenX(hook.x) - HOOK_W / 2),
               (int)(toScreenY(hook.y) - HOOK_H / 2),
               (int)HOOK_W, (int)HOOK_H, hookSprite);
}

void drawPowerups() {
    for (int i = 0; i < powerupCount; i++) {
        if (powerups[i].taken) continue;
        iShowImage((int)(toScreenX(powerups[i].x) - 20),
                   (int)(toScreenY(powerups[i].y) - 20), 40, 40, spriteFor(powerups[i].kind));
    }

    if (!chest.active) return;
    iShowImage((int)(toScreenX(chest.x) - 28), (int)(toScreenY(chest.y) - 26), 56, 52, chestSprite);
    char buf[16];
    sprintf_s(buf, "%ds", chest.secsLeft);
    drawText(toScreenX(chest.x) - 12, toScreenY(chest.y) + 36, buf, 255, 225, 90);
}

#endif
