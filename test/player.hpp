#ifndef PLAYER_H
#define PLAYER_H
// ==== 1. INCLUDES ====
#include "utility.hpp"

// ==== 2. CONSTANTS ====
const double JUMP_LAUNCH_SPEED   = 9.0;
const double JUMP_GRAVITY        = -0.5;
const double JUMP_MAX_FALL_SPEED = -12.0;

// ==== 3. STRUCT ====
struct PlayerFish {
    double x, y;
    double size;
    double speed;
    FacingDirection facing;
    bool isJumping;
    double vy;
    int spriteRight, spriteLeft, spriteUp, spriteDown, spriteJump;
};

// ==== 4. GLOBAL OBJECT ====
PlayerFish player;
int playerInvulnerableTicks = 0;
double playerBaseSpeed = 4.0;

// Character selection variables
int selectedCharacter = -1;
int playerCharacterSprites[3];
int playerCharacterRight[3];
int playerCharacterLeft[3];

// ==== 5. FUNCTIONS ====
void loadPlayerCharacters() {
    playerCharacterRight[0] = iLoadImage("Images/Character/fish_character_01_right.png");
    if (playerCharacterRight[0] == 0) playerCharacterRight[0] = iLoadImage("Images/Character/fish_character_01.png");
    playerCharacterLeft[0]  = iLoadImage("Images/Character/fish_character_01_left.png");

    playerCharacterRight[1] = iLoadImage("Images/Character/fish_character_02_right.png");
    if (playerCharacterRight[1] == 0) playerCharacterRight[1] = iLoadImage("Images/Character/fish_character_02.png");
    playerCharacterLeft[1]  = iLoadImage("Images/Character/fish_character_02_left.png");

    playerCharacterRight[2] = iLoadImage("Images/Character/fish_character_03_right.png");
    if (playerCharacterRight[2] == 0) playerCharacterRight[2] = iLoadImage("Images/Character/fish_character_03.png");
    playerCharacterLeft[2]  = iLoadImage("Images/Character/fish_character_03_left.png");

    for (int i = 0; i < 3; i++) {
        playerCharacterSprites[i] = playerCharacterRight[i];
    }
}

void loadPlayer() {
    player.spriteRight = iLoadImage("Images/Player/fish_right.png");
    player.spriteLeft  = iLoadImage("Images/Player/fish_left.png");
    player.spriteUp    = iLoadImage("Images/Player/fish_up.png");
    player.spriteDown  = iLoadImage("Images/Player/fish_down.png");
    player.spriteJump  = iLoadImage("Images/Player/fish_jump.png");
    loadPlayerCharacters();
    player.size = 18.0;
    player.speed = playerBaseSpeed;
    player.facing = FACE_RIGHT;
    player.isJumping = false;
    player.vy = 0;
}

void resetPlayer(double startX, double startY) {
    player.x = startX;
    player.y = startY;
    player.size = 18.0;
    player.isJumping = false;
    player.vy = 0;
    player.speed = playerBaseSpeed;
    player.facing = FACE_RIGHT;
    playerInvulnerableTicks = 0;
}

void movePlayer(double dx, double dy) {
    player.x += dx * player.speed;
    player.y += dy * player.speed;
    if (dx != 0 || dy != 0) player.facing = facingFromDelta(dx, dy);
}

// The fish sprite is drawn size*2 tall, CENTERED on (x,y) - so its visual
// top edge sits at (y + size), not y. Clamping to (WATER_SURFACE_Y - size)
// keeps that top edge exactly on the line instead of poking above it.
double restingY() { return WATER_SURFACE_Y - player.size; }

void clampPlayerToWater() {
    if (!player.isJumping && player.y > restingY()) player.y = restingY();
}

void startPlayerJump() {
    if (!player.isJumping && player.y >= restingY() - 1) {
        player.isJumping = true;
        player.vy = JUMP_LAUNCH_SPEED;
    }
}

void updatePlayerJump() {
    if (!player.isJumping) return;
    player.y += player.vy;
    player.vy += JUMP_GRAVITY;
    if (player.vy < JUMP_MAX_FALL_SPEED) player.vy = JUMP_MAX_FALL_SPEED;

    if (player.y <= restingY()) {
        player.y = restingY();
        player.isJumping = false;
        player.vy = 0;
    }
}

void growPlayer(double amount) {
    player.size += amount;
}

void drawPlayer() {
    int sprite = player.spriteRight;
    if (selectedCharacter >= 0 && selectedCharacter < 3) {
        if (player.facing == FACE_LEFT) {
            sprite = playerCharacterLeft[selectedCharacter];
        } else {
            sprite = playerCharacterRight[selectedCharacter];
        }
    } else {
        if (player.isJumping)                 sprite = player.spriteJump;
        else if (player.facing == FACE_LEFT)  sprite = player.spriteLeft;
        else if (player.facing == FACE_UP)    sprite = player.spriteUp;
        else if (player.facing == FACE_DOWN)  sprite = player.spriteDown;
    }

    double drawSize = player.size * 2.0;
    int screenX = (int)(worldToScreenX(player.x) - drawSize / 2);
    int screenY = (int)(worldToScreenY(player.y) - drawSize / 2);
    iShowImage(screenX, screenY, (int)drawSize, (int)drawSize, sprite);
}

#endif
