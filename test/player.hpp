#ifndef PLAYER_HPP
#define PLAYER_HPP
// =====================================================================
// player.hpp - the player's fish.
// Movement, growing, jumping, the bubble trail it leaves while
// swimming, the "lost a life" respawn animation, and the shaking it
// does while caught on the level-3 hook.
// =====================================================================
#include "utility.hpp"
#include "sound.hpp"

// ==== 1. TUNING ====
// Sizes are scaled for the 1900x1000 window - a fish's "size" is its
// radius, so it is drawn at size*2 pixels across.
const double JUMP_UP     = 9.0;    // upward kick when Space is pressed
const double GRAVITY     = -0.5;   // pulls the jump back down each tick
const double MAX_FALL    = -12.0;  // stops the fall speeding up forever
const double START_SIZE  = 28.0;
const double BASE_SPEED  = 5.2;

// ==== 2. STRUCT ====
struct Player {
    double x, y;
    double size;        // radius: used for BOTH collision and draw size
    double speed;
    Facing facing;

    bool   jumping;
    double vy;

    // Respawn animation: after losing a life the fish drops in from the
    // top of the screen, then play resumes. While dropping the player
    // cannot be controlled or hurt.
    bool   respawning;
    double respawnVY;

    int    safeTicks;   // brief safety window after being hit

    // Shaking while caught on the hook (level 3).
    bool   shaking;
    int    shakeTick;
};

Player player;
int playerSkin = 0;              // which of the 3 fish characters was chosen
int skinRight[3], skinLeft[3];   // sprites for each character

// ==== 3. BUBBLES ====
// Two separate systems, as they behave differently:
//   swimBubbles - small, short-lived, made BY the player when moving
//   seaBubbles  - ambient, rise from the deep floor to the surface
#define MAX_SWIM_BUBBLES 30
#define MAX_SEA_BUBBLES  26

struct Bubble {
    double x, y;      // world position
    double vy;
    double size;
    int    life;      // ticks left; 0 = unused slot (swim bubbles only)
};

Bubble swimBubbles[MAX_SWIM_BUBBLES];
Bubble seaBubbles[MAX_SEA_BUBBLES];
int bubbleSprite = 0;

void spawnSwimBubble(double x, double y) {
    for (int i = 0; i < MAX_SWIM_BUBBLES; i++) {
        if (swimBubbles[i].life > 0) continue;
        swimBubbles[i].x = x;
        swimBubbles[i].y = y + randRange(-7, 7);
        swimBubbles[i].vy = randRange(0.7, 1.5);
        swimBubbles[i].size = randRange(8, 15);
        swimBubbles[i].life = 45 + rand() % 30;
        return;
    }
}

void updateSwimBubbles() {
    for (int i = 0; i < MAX_SWIM_BUBBLES; i++) {
        if (swimBubbles[i].life <= 0) continue;
        swimBubbles[i].y += swimBubbles[i].vy;
        swimBubbles[i].life--;
        if (swimBubbles[i].y > SEA_Y) swimBubbles[i].life = 0;
    }
}

void clearSwimBubbles() {
    for (int i = 0; i < MAX_SWIM_BUBBLES; i++) swimBubbles[i].life = 0;
}

// Ambient bubbles: always present, always rising, looping back to the
// sea floor when they reach the surface.
void resetSeaBubble(Bubble &b, bool anywhere) {
    b.x = scrollX + randRange(-CENTER_X, CENTER_X);
    b.y = anywhere ? randRange(FLOOR_Y, SEA_Y) : randRange(FLOOR_Y, FLOOR_Y + 60);
    b.vy = randRange(0.35, 0.95);
    b.size = randRange(7, 18);
    b.life = 1;   // ambient bubbles never expire
}

void setupSeaBubbles() {
    for (int i = 0; i < MAX_SEA_BUBBLES; i++) resetSeaBubble(seaBubbles[i], true);
}

void updateSeaBubbles() {
    for (int i = 0; i < MAX_SEA_BUBBLES; i++) {
        seaBubbles[i].y += seaBubbles[i].vy;
        // Gentle side-to-side wobble as they rise.
        seaBubbles[i].x += sin(seaBubbles[i].y * 0.03) * 0.35;
        if (seaBubbles[i].y > SEA_Y) resetSeaBubble(seaBubbles[i], false);
    }
}

void drawOneBubble(const Bubble &b, int alphaPct) {
    // iSetColor dims the image instead of true transparency - close
    // enough for bubbles and it keeps everything using one colour call.
    int v = 90 + alphaPct;
    iSetColor(v, v, v);
    iShowImage((int)(toScreenX(b.x) - b.size / 2),
               (int)(toScreenY(b.y) - b.size / 2), (int)b.size, (int)b.size, bubbleSprite);
    iSetColor(255, 255, 255);
}

void drawSeaBubbles() {
    for (int i = 0; i < MAX_SEA_BUBBLES; i++) drawOneBubble(seaBubbles[i], 60);
}

void drawSwimBubbles() {
    for (int i = 0; i < MAX_SWIM_BUBBLES; i++) {
        if (swimBubbles[i].life <= 0) continue;
        drawOneBubble(swimBubbles[i], 120);
    }
}

// ==== 4. LOADING ====
void loadPlayer() {
    skinRight[0] = loadImg("Images/Character/fish_character_01_right.png", "Images/Character/fish_character_01.png");
    skinLeft[0]  = loadImg("Images/Character/fish_character_01_left.png",  "Images/Character/fish_character_01.png");
    skinRight[1] = loadImg("Images/Character/fish_character_02_right.png", "Images/Character/fish_character_02.png");
    skinLeft[1]  = loadImg("Images/Character/fish_character_02_left.png",  "Images/Character/fish_character_02.png");
    skinRight[2] = loadImg("Images/Character/fish_character_03_right.png", "Images/Character/fish_character_03.png");
    skinLeft[2]  = loadImg("Images/Character/fish_character_03_left.png",  "Images/Character/fish_character_03.png");

    bubbleSprite = loadImg("Images/Background/bubble_01.png");
    setupSeaBubbles();
}

void resetPlayer(double startX, double startY) {
    player.x = startX;
    player.y = startY;
    player.size = START_SIZE;
    player.speed = BASE_SPEED;
    player.facing = FACE_RIGHT;
    player.jumping = false;
    player.vy = 0;
    player.respawning = false;
    player.respawnVY = 0;
    player.safeTicks = 0;
    player.shaking = false;
    player.shakeTick = 0;
    clearSwimBubbles();
}

// ==== 5. MOVEMENT ====
// The fish is drawn centred on (x,y), so its top edge is at y + size.
// Resting just below SEA_Y keeps it fully underwater.
double restY() { return SEA_Y - player.size; }

// Movement is blocked while shaking on the hook or dropping in after a
// life loss - both are moments the player is not in control.
bool playerCanMove() { return !player.respawning && !player.shaking; }

void movePlayer(double dx, double dy) {
    if (!playerCanMove()) return;

    player.x += dx * player.speed;
    player.y += dy * player.speed;
    if (dx != 0 || dy != 0) player.facing = facingOf(dx, dy);

    // Swimming sideways leaves a bubble trail and makes a soft swish.
    if (dx != 0) {
        playSwim();
        if (rand() % 4 == 0) {
            double tailX = player.x - (dx > 0 ? player.size : -player.size);
            spawnSwimBubble(tailX, player.y);
        }
    }
}

void clampPlayer(double worldWidth) {
    player.x = clampD(player.x, 0, worldWidth);
    if (player.y < FLOOR_Y + player.size) player.y = FLOOR_Y + player.size;
    if (!player.jumping && !player.respawning && player.y > restY()) player.y = restY();
}

void startJump() {
    if (!playerCanMove()) return;
    if (!player.jumping && player.y >= restY() - 1) {
        player.jumping = true;
        player.vy = JUMP_UP;
    }
}

void updateJump() {
    if (!player.jumping) return;
    player.y += player.vy;
    player.vy += GRAVITY;
    if (player.vy < MAX_FALL) player.vy = MAX_FALL;

    if (player.y <= restY()) {
        player.y = restY();
        player.jumping = false;
        player.vy = 0;
    }
}

void growPlayer(double amount) { player.size += amount; }

// ==== 6. LOSING A LIFE ====
// Losing a life is never an instant game over unless it was the LAST
// life. The fish keeps the size it had, drops in from the top of the
// gameplay window, and carries on from the middle of the ocean.
void startRespawn(double worldWidth) {
    player.respawning = true;
    player.respawnVY = 0;
    player.x = worldWidth / 2.0;
    player.y = SEA_Y - 10;      // start just under the surface
    player.jumping = false;
    player.vy = 0;
    player.shaking = false;
    clearSwimBubbles();
}

void loseLife(double worldWidth) {
    if (player.safeTicks > 0 || player.respawning) return;

    stats.lives--;
    if (stats.lives <= 0) { triggerGameOver(); return; }

    playLose();
    startRespawn(worldWidth);
}

void updateRespawn() {
    if (!player.respawning) return;

    double targetY = SEA_Y / 2.0;
    player.respawnVY += GRAVITY * 0.6;      // gentle fall
    player.y += player.respawnVY;

    if (rand() % 3 == 0) spawnSwimBubble(player.x, player.y + player.size);

    if (player.y <= targetY) {
        player.y = targetY;
        player.respawning = false;
        player.respawnVY = 0;
        player.safeTicks = 70;   // ~2s of safety so you are not hit instantly
    }
}

// ==== 7. HOOK SHAKING (level 3) ====
void startShaking() {
    player.shaking = true;
    player.shakeTick = 0;
}

void stopShaking() {
    player.shaking = false;
    player.shakeTick = 0;
}

// Wobbles the fish in place and plays the struggling sound.
void updateShaking() {
    if (!player.shaking) return;
    player.shakeTick++;
    playStruggle();
    if (rand() % 4 == 0) spawnSwimBubble(player.x + randRange(-14, 14), player.y);
}

// How far the fish is offset by its current shake, in pixels.
double shakeOffset() {
    if (!player.shaking) return 0.0;
    return sin(player.shakeTick * 0.62) * 9.0;
}

void tickPlayerTimers() {
    if (player.safeTicks > 0) player.safeTicks--;
    tickSoundCooldowns();
}

// ==== 8. DRAWING ====
void drawPlayer() {
    // While briefly safe after a hit the fish blinks, so the player can
    // see they are not currently vulnerable.
    if (player.safeTicks > 0 && (player.safeTicks / 4) % 2 == 0) return;

    int sprite = (player.facing == FACE_LEFT) ? skinLeft[playerSkin] : skinRight[playerSkin];
    double s = player.size * 2.0;
    iShowImage((int)(toScreenX(player.x) + shakeOffset() - s / 2),
               (int)(toScreenY(player.y) - s / 2), (int)s, (int)s, sprite);
}

#endif
