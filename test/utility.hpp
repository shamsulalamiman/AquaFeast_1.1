#ifndef UTILITY_H
#define UTILITY_H
// ==== 1. INCLUDES ====
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>

// ==== iGRAPHICS FORWARD DECLARATIONS ====
unsigned int iLoadImage(char filename[]);
void iShowImage(int x, int y, int width, int height, unsigned int texture);


// ==== 2. CONSTANTS ====
#define SCREEN_WIDTH   1920
#define SCREEN_HEIGHT  1080
#define MAX_LEVELS     3

#define KEY_ENTER      13
#define KEY_BACKSPACE  8
#define KEY_RESTART    'r'
#define KEY_MUTE       'm'

#define GAME_FONT GLUT_BITMAP_9_BY_15

const double HUD_HEIGHT = 50.0;

// 3/4 ocean, 1/4 sky, measured up from below the HUD - a fraction instead
// of a hand-picked pixel number, so it can't drift out of sync if the
// screen size ever changes.
const double OCEAN_FRACTION = 0.75;
const double WATER_SURFACE_Y = (SCREEN_HEIGHT - HUD_HEIGHT) * OCEAN_FRACTION;
const double WORLD_FLOOR_Y = 0.0;

// ==== 3. STRUCTS ====
struct GameStats {
    int score = 0;
    int lives = 3;
    int timeRemaining = 120;
    int level = 1;
    float progress = 0.0f;
};

enum GameState { MENU, CHARACTER_SELECT, PLAYING };
enum MenuScreen { MENU_HOME, MENU_LEVEL_SELECT, MENU_INSTRUCTIONS, MENU_CREDITS, MENU_CHARACTER_SELECT };
enum FacingDirection { FACE_RIGHT, FACE_LEFT, FACE_UP, FACE_DOWN };

// ==== 4. GLOBAL STATE ====
GameState gameState = MENU;
bool isInMenu = true;
bool isPlaying = false;
bool isGameOver = false;
bool isLevelComplete = false;
bool isMuted = false;
int  currentLevel = 1;

MenuScreen currentMenuScreen = MENU_HOME;
bool requestLevelStart = false;

GameStats stats;
const double GROWTH_PER_FISH = 1.5;

//======================================================================
// SCROLLING (used to be called "the camera" - renamed because there's
// no actual camera object, just one number).
//
// The world is wider than the screen, so as the player swims left or
// right we shift everything by the same amount to keep them roughly
// centered. scrollX is that shift amount - it's the ONLY thing that
// changes; height never scrolls at all, so there's nothing to track
// vertically. worldToScreenX/Y below do the actual shifting.
//======================================================================
double scrollX = 0;
const double SCREEN_CENTER_X = SCREEN_WIDTH / 2.0;

inline double worldToScreenX(double worldX) { return worldX - scrollX + SCREEN_CENTER_X; }

// Height never scrolls, so this is really just worldY - it only exists
// so every draw call can call BOTH worldToScreenX and worldToScreenY
// the same way, instead of X being "converted" and Y being raw.
inline double worldToScreenY(double worldY) { return worldY; }

// ==== 5. SMALL HELPER FUNCTIONS ====
// Each one does ONE thing, so they stay short and easy to scan.

inline void drawPixelTitle(double x, double y, const char* text, int r, int g, int b) {
    iSetColor(0, 0, 0);
    iText((int)x + 2, (int)y - 2, (char*)text, GAME_FONT); // shadow copy
    iSetColor(r, g, b);
    iText((int)x, (int)y, (char*)text, GAME_FONT);
}

inline double distanceBetween(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2, dy = y1 - y2;
    return sqrt(dx * dx + dy * dy);
}

inline bool circlesTouch(double x1, double y1, double r1, double x2, double y2, double r2) {
    return distanceBetween(x1, y1, x2, y2) < (r1 + r2);
}

inline bool pointInRect(double px, double py, double rx, double ry, double rw, double rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

inline FacingDirection facingFromDelta(double dx, double dy) {
    if (fabs(dx) > fabs(dy)) return dx > 0 ? FACE_RIGHT : FACE_LEFT;
    return dy > 0 ? FACE_UP : FACE_DOWN;
}

// Picks a fresh random direction + how many ticks to keep it - shared by
// every random-wander entity (prey AND predators) instead of copy-pasted
// in each file.
inline void pickNewDirection(double &dirX, double &dirY, int &ticksUntilTurn) {
    double angle = (rand() % 360) * 3.14159265 / 180.0;
    dirX = cos(angle);
    dirY = sin(angle);
    ticksUntilTurn = 30 + (rand() % 60);
}

// One shared "was this just pressed" check, used by every debounced key
// or click (menus, restart, mute). previousState is updated in place, so
// the caller just keeps one bool per key across frames.
inline bool wasJustPressed(bool isPressedNow, bool &previousState) {
    bool justPressed = isPressedNow && !previousState;
    previousState = isPressedNow;
    return justPressed;
}

#endif
