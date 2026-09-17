#ifndef BOAT_HPP
#define BOAT_HPP

// ============================================================
// 1. INCLUDES & FORWARD DECLARATIONS
// ============================================================
#include "utility.hpp"
#include "player.hpp"
#include "menu.hpp" // for triggerGameOver()

void iSetColor(double r, double g, double b);
void iLine(double x1, double y1, double x2, double y2);

// ============================================================
// 2. CONSTANTS & CONFIGURATION
// ============================================================

// Boat and Net Dimensions
const double BOAT_WIDTH   = 130.0;
const double BOAT_HEIGHT  = 70.0;
const double NET_WIDTH    = 85.0;
const double NET_HEIGHT   = 85.0;

// Catch animation duration before Game Over (~50 frames ≈ 0.85s)
const int NET_CATCH_DELAY = 50;

// Inner collision box margins (fair hitbox matching visible net mesh)
const double NET_HITBOX_INSET_X = 12.0;
const double NET_HITBOX_INSET_Y = 12.0;

// Level 1: Boat and net disabled
const double BOAT_SPEED_L1   = 0.0;
const double NET_SPEED_L1    = 0.0;
const int    NET_COOLDOWN_L1 = 999999;

// Level 2: Moderate boat and net speed, relaxed cooldown
const double BOAT_SPEED_L2   = 1.8;
const double NET_SPEED_L2    = 2.2;
const int    NET_COOLDOWN_L2 = 270; // ~4.5 seconds

// Level 3: Faster boat and net, frequent attacks
const double BOAT_SPEED_L3   = 3.2;
const double NET_SPEED_L3    = 3.8;
const int    NET_COOLDOWN_L3 = 160; // ~2.6 seconds

// ============================================================
// 3. ENUMS & STRUCTS
// ============================================================

enum BoatNetState {
    BOAT_IDLE,
    NET_FALLING,
    NET_CAUGHT,
    NET_FINISHED
};

struct FishingNet {
    double x;           // World X (center of net)
    double y;           // World Y (center of net)
    double speed;       // Downward sink speed
    double width;
    double height;
    bool active;
    BoatNetState state;
    int catchTimer;     // Catch animation countdown before Game Over
};

struct FishingBoatObstacle {
    double x;           // World X position
    double y;           // World Y position (waterline)
    double speed;       // Horizontal movement speed
    int direction;      // 1 = moving right, -1 = moving left
    double width;
    double height;
    bool active;        // Enabled in Level 2+
    int cooldown;       // Ticks until next net drop
    int maxCooldown;    // Base cooldown for current level
    double worldWidth;  // Current level boundary for patrol
    FishingNet net;     // Dropped fishing net
};

// ============================================================
// 4. GLOBAL STATE
// ============================================================

FishingBoatObstacle boatObstacle;
int boatSpriteId       = 0;
int netOpenSpriteId    = 0;
int netClosedSpriteId  = 0;
bool isPlayerTrappedInBoatNet = false;

// ============================================================
// 5. FUNCTIONS
// ============================================================

// Forward declaration
bool checkBoatCollision();

// Load image assets for the boat and fishing net (with graceful fallbacks)
void loadBoat() {
    boatSpriteId = iLoadImage((char*)"Images/Boat/boat.png");
    if (boatSpriteId == 0) boatSpriteId = iLoadImage((char*)"Images/Boats/boat_01.png");
    if (boatSpriteId == 0) boatSpriteId = iLoadImage((char*)"Images/Boats/boat2.png");

    netOpenSpriteId = iLoadImage((char*)"Images/Boat/net_open.png");
    if (netOpenSpriteId == 0) netOpenSpriteId = iLoadImage((char*)"Images/Boats/net_open.png");
    if (netOpenSpriteId == 0) netOpenSpriteId = iLoadImage((char*)"Images/Boats/net_01.png");

    netClosedSpriteId = iLoadImage((char*)"Images/Boat/net_closed.png");
    if (netClosedSpriteId == 0) netClosedSpriteId = iLoadImage((char*)"Images/Boats/net_closed.png");
    if (netClosedSpriteId == 0) netClosedSpriteId = netOpenSpriteId;
}

// Reset boat position, timers, and net state for the given level
void resetBoat(int level = currentLevel, double worldWidth = 2600.0) {
    isPlayerTrappedInBoatNet = false;
    boatObstacle.worldWidth = worldWidth;

    // Level 1: Disabled
    if (level <= 1) {
        boatObstacle.active = false;
        boatObstacle.net.active = false;
        boatObstacle.net.state = BOAT_IDLE;
        return;
    }

    // Configure difficulty based on level
    double bSpeed = BOAT_SPEED_L2;
    double nSpeed = NET_SPEED_L2;
    int cooldown  = NET_COOLDOWN_L2;

    if (level >= 3) {
        bSpeed   = BOAT_SPEED_L3;
        nSpeed   = NET_SPEED_L3;
        cooldown = NET_COOLDOWN_L3;
    }

    boatObstacle.active      = true;
    boatObstacle.width       = BOAT_WIDTH;
    boatObstacle.height      = BOAT_HEIGHT;
    // Sits on the water surface (hull extends 10px below surface line)
    boatObstacle.y           = WATER_SURFACE_Y - 10.0;
    boatObstacle.x           = 150.0 + (rand() % 350);
    boatObstacle.speed       = bSpeed;
    boatObstacle.direction   = 1;
    boatObstacle.maxCooldown = cooldown;
    // Initial drop cooldown with random variance
    boatObstacle.cooldown    = cooldown / 2 + (rand() % (cooldown / 2));

    // Reset net
    boatObstacle.net.width      = NET_WIDTH;
    boatObstacle.net.height     = NET_HEIGHT;
    boatObstacle.net.speed      = nSpeed;
    boatObstacle.net.active     = false;
    boatObstacle.net.state      = BOAT_IDLE;
    boatObstacle.net.catchTimer = 0;
    boatObstacle.net.x          = boatObstacle.x + boatObstacle.width / 2.0;
    boatObstacle.net.y          = boatObstacle.y;
}

// Initialize boat (alias to resetBoat for clean API consistency)
void initBoat(int level = currentLevel, double worldWidth = 2600.0) {
    resetBoat(level, worldWidth);
}

// Checks AABB collision between player fish and the fishing net
bool checkBoatCollision() {
    if (!boatObstacle.active || !boatObstacle.net.active) return false;
    if (boatObstacle.net.state != NET_FALLING) return false;

    // Player bounding box (centered at player.x, player.y)
    double pRadius = player.size * 0.85;
    double pLeft   = player.x - pRadius;
    double pRight  = player.x + pRadius;
    double pBottom = player.y - pRadius;
    double pTop    = player.y + pRadius;

    // Net bounding box (centered at net.x, net.y with inset margins)
    double nHalfW  = (boatObstacle.net.width / 2.0) - NET_HITBOX_INSET_X;
    double nHalfH  = (boatObstacle.net.height / 2.0) - NET_HITBOX_INSET_Y;
    double nLeft   = boatObstacle.net.x - nHalfW;
    double nRight  = boatObstacle.net.x + nHalfW;
    double nBottom = boatObstacle.net.y - nHalfH;
    double nTop    = boatObstacle.net.y + nHalfH;

    // Standard AABB overlap test
    bool overlapX = (pLeft <= nRight) && (pRight >= nLeft);
    bool overlapY = (pBottom <= nTop) && (pTop >= nBottom);

    if (overlapX && overlapY) {
        // Net caught the player!
        boatObstacle.net.state = NET_CAUGHT;
        boatObstacle.net.catchTimer = NET_CATCH_DELAY;
        // Snap net center to player position
        boatObstacle.net.x = player.x;
        boatObstacle.net.y = player.y;
        isPlayerTrappedInBoatNet = true;
        return true;
    }

    return false;
}

// Updates boat movement, net descent, timers, and catch state
void updateBoat(double worldWidth = 2600.0) {
    if (!boatObstacle.active) return;
    boatObstacle.worldWidth = worldWidth;

    // 1. Boat horizontal patrol
    boatObstacle.x += boatObstacle.speed * boatObstacle.direction;

    // Reversal near level world edges
    if (boatObstacle.direction > 0 && boatObstacle.x > worldWidth - boatObstacle.width - 60.0) {
        boatObstacle.direction = -1;
    } else if (boatObstacle.direction < 0 && boatObstacle.x < 60.0) {
        boatObstacle.direction = 1;
    }

    // 2. Net state machine
    switch (boatObstacle.net.state) {
        case BOAT_IDLE: {
            if (boatObstacle.cooldown > 0) {
                boatObstacle.cooldown--;
            } else {
                // Drop net from boat
                boatObstacle.net.active = true;
                boatObstacle.net.state  = NET_FALLING;
                boatObstacle.net.x      = boatObstacle.x + boatObstacle.width / 2.0;
                boatObstacle.net.y      = boatObstacle.y - 10.0;
            }
            break;
        }

        case NET_FALLING: {
            // Sinks vertically into the water
            boatObstacle.net.y -= boatObstacle.net.speed;

            // Check collision with player
            checkBoatCollision();

            // If net missed and touched the sea floor
            if (boatObstacle.net.state == NET_FALLING &&
                boatObstacle.net.y <= WORLD_FLOOR_Y + boatObstacle.net.height / 2.0) {
                boatObstacle.net.state = NET_FINISHED;
            }
            break;
        }

        case NET_CAUGHT: {
            // Keep net locked onto the fish
            boatObstacle.net.x = player.x;
            boatObstacle.net.y = player.y;
            isPlayerTrappedInBoatNet = true;

            // Catch animation countdown
            if (boatObstacle.net.catchTimer > 0) {
                boatObstacle.net.catchTimer--;
            } else {
                // Delay finished: activate existing Game Over system!
                boatObstacle.net.state = NET_FINISHED;
                triggerGameOver();
            }
            break;
        }

        case NET_FINISHED: {
            // Deactivate net and reset cooldown with variance
            boatObstacle.net.active = false;
            boatObstacle.net.state  = BOAT_IDLE;
            boatObstacle.cooldown   = boatObstacle.maxCooldown + (rand() % 60 - 30);
            if (boatObstacle.cooldown < 60) boatObstacle.cooldown = 60;
            break;
        }
    }
}

// Renders the boat, connecting rope, and fishing net
void drawBoat() {
    if (!boatObstacle.active) return;

    // 1. Draw connecting rope if net is falling or caught
    if (boatObstacle.net.active &&
       (boatObstacle.net.state == NET_FALLING || boatObstacle.net.state == NET_CAUGHT)) {

        int ropeTopX = (int)worldToScreenX(boatObstacle.x + boatObstacle.width / 2.0);
        int ropeTopY = (int)worldToScreenY(boatObstacle.y + 15.0);

        int ropeBottomX = (int)worldToScreenX(boatObstacle.net.x);
        int ropeBottomY = (int)worldToScreenY(boatObstacle.net.y + boatObstacle.net.height / 2.0 - 8.0);

        // Twine rope lines
        iSetColor(55, 45, 35);
        iLine(ropeTopX, ropeTopY, ropeBottomX, ropeBottomY);
        iLine(ropeTopX + 1, ropeTopY, ropeBottomX + 1, ropeBottomY);
    }

    // 2. Draw Boat on the water surface
    int bScreenX = (int)worldToScreenX(boatObstacle.x);
    int bScreenY = (int)worldToScreenY(boatObstacle.y);

    if (bScreenX + (int)boatObstacle.width >= -80 && bScreenX <= SCREEN_WIDTH + 80) {
        iShowImage(bScreenX, bScreenY, (int)boatObstacle.width, (int)boatObstacle.height, boatSpriteId);
    }

    // 3. Draw Fishing Net (open-net while falling, closed-net when caught)
    if (boatObstacle.net.active &&
       (boatObstacle.net.state == NET_FALLING || boatObstacle.net.state == NET_CAUGHT)) {

        int nScreenX = (int)(worldToScreenX(boatObstacle.net.x) - boatObstacle.net.width / 2.0);
        int nScreenY = (int)(worldToScreenY(boatObstacle.net.y) - boatObstacle.net.height / 2.0);

        int currentSprite = (boatObstacle.net.state == NET_CAUGHT) ? netClosedSpriteId : netOpenSpriteId;
        iShowImage(nScreenX, nScreenY, (int)boatObstacle.net.width, (int)boatObstacle.net.height, currentSprite);
    }
}

// Helper to check if player should be immobilized by the net
inline bool isPlayerTrappedByNet() {
    return isPlayerTrappedInBoatNet;
}

#endif // BOAT_HPP
