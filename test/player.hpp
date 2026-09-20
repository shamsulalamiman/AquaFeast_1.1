#ifndef PLAYER_HPP
#define PLAYER_HPP
// =====================================================================
// player.hpp - the player's fish.
// Movement, growing, jumping, the trail of bubbles it leaves while
// swimming, and the "lost a life" respawn animation.
// =====================================================================
#include "utility.hpp"
#include "sound.hpp"

// ==== 1. TUNING ====
const double JUMP_UP     = 9.0;    // upward kick when Space is pressed
const double GRAVITY     = -0.5;   // pulls the jump back down each tick
const double MAX_FALL    = -12.0;  // stops the fall speeding up forever
const double START_SIZE  = 18.0;
const double BASE_SPEED  = 4.5;

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
};

Player player;
int playerSkin = 0;              // which of the 3 fish characters was chosen
int skinRight[3], skinLeft[3];   // sprites for each character

// ==== 3. SWIM BUBBLES ====
// Small bubbles that trail behind the fish while it swims sideways.
#define MAX_SWIM_BUBBLES 24
struct SwimBubble {
    double x, y;      // world position
    double vy;
    int    life;      // ticks left before it pops; 0 = unused slot
};
SwimBubble swimBubbles[MAX_SWIM_BUBBLES];
int swimBubbleSprite = 0;

void spawnSwimBubble(double x, double y) {
    for (int i = 0; i < MAX_SWIM_BUBBLES; i++) {
        if (swimBubbles[i].life > 0) continue;
        swimBubbles[i].x = x;
        swimBubbles[i].y = y + randRange(-6, 6);
        swimBubbles[i].vy = randRange(0.6, 1.4);
        swimBubbles[i].life = 40 + rand() % 25;
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

void drawSwimBubbles() {
    glColor4f(1.0f, 1.0f, 1.0f, 0.45f);
    for (int i = 0; i < MAX_SWIM_BUBBLES; i++) {
        if (swimBubbles[i].life <= 0) continue;
        int s = 10;
        iShowImage((int)toScreenX(swimBubbles[i].x) - s / 2,
                   (int)toScreenY(swimBubbles[i].y) - s / 2, s, s, swimBubbleSprite);
    }
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void clearSwimBubbles() {
    for (int i = 0; i < MAX_SWIM_BUBBLES; i++) swimBubbles[i].life = 0;
}

// ==== 4. LOADING ====
void loadPlayer() {
    skinRight[0] = loadImg("Images/Character/fish_character_01_right.png", "Images/Character/fish_character_01.png");
    skinLeft[0]  = loadImg("Images/Character/fish_character_01_left.png",  "Images/Character/fish_character_01.png");
    skinRight[1] = loadImg("Images/Character/fish_character_02_right.png", "Images/Character/fish_character_02.png");
    skinLeft[1]  = loadImg("Images/Character/fish_character_02_left.png",  "Images/Character/fish_character_02.png");
    skinRight[2] = loadImg("Images/Character/fish_character_03_right.png", "Images/Character/fish_character_03.png");
    skinLeft[2]  = loadImg("Images/Character/fish_character_03_left.png",  "Images/Character/fish_character_03.png");

    swimBubbleSprite = loadImg("Images/Background/bubble_01.png");
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
    clearSwimBubbles();
}

// ==== 5. MOVEMENT ====
// The fish is drawn centred on (x,y), so its top edge is at y + size.
// Resting just below SEA_Y keeps it fully underwater.
double restY() { return SEA_Y - player.size; }

void movePlayer(double dx, double dy) {
    if (player.respawning) return;

    player.x += dx * player.speed;
    player.y += dy * player.speed;
    if (dx != 0 || dy != 0) player.facing = facingOf(dx, dy);

    // Swimming sideways leaves a bubble trail and makes a soft swish.
    if (dx != 0) {
        playSwim();
        if (rand() % 5 == 0) {
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
    if (player.respawning) return;
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
    clearSwimBubbles();
}

void loseLife(double worldWidth) {
    if (player.safeTicks > 0 || player.respawning) return;

    stats.lives--;
    if (stats.lives <= 0) { triggerGameOver(); return; }

    playLose();
    startRespawn(worldWidth);
}

// The drop-in animation. The fish falls until it reaches the middle
// depth of the ocean, then normal play resumes.
void updateRespawn() {
    if (!player.respawning) return;

    double targetY = SEA_Y / 2.0;
    player.respawnVY += GRAVITY * 0.6;      // gentle fall
    player.y += player.respawnVY;

    // Bubbles stream off the falling fish so the drop reads clearly.
    if (rand() % 3 == 0) spawnSwimBubble(player.x, player.y + player.size);

    if (player.y <= targetY) {
        player.y = targetY;
        player.respawning = false;
        player.respawnVY = 0;
        player.safeTicks = 70;   // ~2s of safety so you are not hit instantly
    }
}

void tickPlayerTimers() {
    if (player.safeTicks > 0) player.safeTicks--;
    tickSwimCooldown();
}

// ==== 7. DRAWING ====
void drawPlayer() {
    // While briefly safe after a hit the fish blinks, so the player can
    // see they are not currently vulnerable.
    if (player.safeTicks > 0 && (player.safeTicks / 4) % 2 == 0) return;

    int sprite = (player.facing == FACE_LEFT) ? skinLeft[playerSkin] : skinRight[playerSkin];
    double s = player.size * 2.0;
    iShowImage((int)(toScreenX(player.x) - s / 2),
               (int)(toScreenY(player.y) - s / 2), (int)s, (int)s, sprite);
}

#endif
