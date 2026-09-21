#ifndef UTILITY_HPP
#define UTILITY_HPP
// =====================================================================
// utility.hpp - things EVERY other file needs.
// Screen size, game state flags, small maths helpers, safe image
// loading, and the shared popup-window look.
// =====================================================================

// <cmath>   : sqrt, sin, cos, fabs
// <cstdio>  : sprintf_s, fopen (used by imageExists)
// <cstdlib> : rand, srand
// <cstring> : strcpy_s, strlen (player names)
// <ctime>   : time() to seed the random numbers
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

// ==== 1. SCREEN LAYOUT ====
#define SCREEN_W 1900
#define SCREEN_H 1000
#define MAX_LEVELS 3

#define KEY_ENTER      13
#define KEY_BACKSPACE  8
#define KEY_SPACE      ' '
#define GAME_FONT      GLUT_BITMAP_9_BY_15
#define BIG_FONT       GLUT_BITMAP_TIMES_ROMAN_24

const double HUD_H = 72.0;                 // HUD bar height (top of screen)
const double PLAY_H = SCREEN_H - HUD_H;    // everything below the HUD

// The water line: 3/4 ocean, 1/4 sky. Using a fraction (not a fixed
// pixel number) means changing SCREEN_H can never break the layout.
const double SEA_Y = PLAY_H * 0.75;        // y of the water surface
const double FLOOR_Y = 0.0;                // ocean floor
const double SAND_H = 58.0;                // sandy strip at the bottom
const double BAND_H = SEA_Y / 3.0;         // one of the 3 water depth bands

// Every popup window in the game is this size, centred on screen.
const double POPUP_W = 500.0;
const double POPUP_H = 400.0;
const double POPUP_X = (SCREEN_W - POPUP_W) / 2.0;
const double POPUP_Y = (SCREEN_H - POPUP_H) / 2.0;

// ==== 2. GAME STATE ====
enum Screen {
    SCR_SPLASH,      // loading page
    SCR_MENU,        // main menu
    SCR_NAME,        // type your nickname
    SCR_CHARACTER,   // pick your fish
    SCR_MAP,         // level map
    SCR_PLAY,        // gameplay
    SCR_SCORES,
    SCR_HELP,
    SCR_CREDITS
};

Screen screen = SCR_SPLASH;
bool isGameOver = false;
bool isLevelWon = false;
bool scoreSaved = false;   // a run's score is recorded only once
bool isMuted = false;
int  currentLevel = 1;
int  unlockedLevel = 1;    // highest level the player may enter

// Set once the player has typed a name, so we never ask again.
bool haveName = false;

// ==== 3. PLAYER STATS (shown on the HUD) ====
struct Stats {
    int score;
    int lives;
    int timeLeft;
    int level;
    double progress;   // 0..1, how close to this level's target size
};
Stats stats;

char playerName[24] = "";

// ==== 4. SIDE-SCROLLING ====
// The world is wider than the screen. scrollX is how far we have slid
// sideways to keep the player near the middle. Height never scrolls.
double scrollX = 0;
const double CENTER_X = SCREEN_W / 2.0;

inline double toScreenX(double worldX) { return worldX - scrollX + CENTER_X; }
inline double toScreenY(double worldY) { return worldY; }

// ==== 5. SMALL HELPERS ====
inline double dist(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2, dy = y1 - y2;
    return sqrt(dx * dx + dy * dy);
}

inline bool touching(double x1, double y1, double r1,
                     double x2, double y2, double r2) {
    return dist(x1, y1, x2, y2) < (r1 + r2);
}

inline bool inBox(double px, double py, double bx, double by, double bw, double bh) {
    return px >= bx && px <= bx + bw && py >= by && py <= by + bh;
}

inline double clampD(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

enum Facing { FACE_RIGHT, FACE_LEFT, FACE_UP, FACE_DOWN };

inline Facing facingOf(double dx, double dy) {
    if (fabs(dx) > fabs(dy)) return dx > 0 ? FACE_RIGHT : FACE_LEFT;
    return dy > 0 ? FACE_UP : FACE_DOWN;
}

inline void randomDir(double &dx, double &dy, int &turnTicks) {
    double angle = (rand() % 360) * 3.14159265 / 180.0;
    dx = cos(angle);
    dy = sin(angle);
    turnTicks = 30 + (rand() % 60);
}

inline double randRange(double lo, double hi) {
    return lo + (rand() / (double)RAND_MAX) * (hi - lo);
}

// "Was this key just pressed?" - true only on the first frame it goes
// down, so one tap = one menu move instead of racing through the list.
inline bool tapped(bool downNow, bool &wasDown) {
    bool result = downNow && !wasDown;
    wasDown = downNow;
    return result;
}

// ==== 6. CONTROLS (Arrow keys AND WASD everywhere) ====
inline bool keyUp()    { return isSpecialKeyPressed(GLUT_KEY_UP)    || isKeyPressed('w') || isKeyPressed('W'); }
inline bool keyDown()  { return isSpecialKeyPressed(GLUT_KEY_DOWN)  || isKeyPressed('s') || isKeyPressed('S'); }
inline bool keyLeft()  { return isSpecialKeyPressed(GLUT_KEY_LEFT)  || isKeyPressed('a') || isKeyPressed('A'); }
inline bool keyRight() { return isSpecialKeyPressed(GLUT_KEY_RIGHT) || isKeyPressed('d') || isKeyPressed('D'); }

// ==== 7. SAFE IMAGE LOADING ====
// IMPORTANT: iGraphics' iLoadImage() does NOT return 0 when a file is
// missing - it still makes a texture id, just with no picture in it.
// So the only reliable way to know a file exists is to try opening it.
// loadImg() falls back to a second path when the first is missing,
// which keeps the game running even if some art is not drawn yet.
inline bool imageExists(const char* path) {
    FILE* f = NULL;
    fopen_s(&f, path, "rb");
    if (f) { fclose(f); return true; }
    return false;
}

inline int loadImg(const char* path, const char* fallback = 0) {
    if (imageExists(path)) return iLoadImage((char*)path);
    if (fallback && imageExists(fallback)) return iLoadImage((char*)fallback);
    return iLoadImage((char*)path); // still returns an (empty) id - never crashes
}

// ==== 8. SHARED DRAWING HELPERS ====
inline void drawText(double x, double y, const char* s, int r, int g, int b, void* font = GAME_FONT) {
    iSetColor(0, 0, 0);
    iText((int)x + 2, (int)y - 2, (char*)s, font);
    iSetColor(r, g, b);
    iText((int)x, (int)y, (char*)s, font);
}

// Roughly how wide a piece of text is, so we can centre it. The bitmap
// fonts are fixed-width, so character-count x width is accurate enough.
inline double textW(const char* s, bool big = false) {
    return strlen(s) * (big ? 14.0 : 9.0);
}

inline void drawTextCentred(double cx, double y, const char* s, int r, int g, int b, void* font = GAME_FONT) {
    drawText(cx - textW(s, font == BIG_FONT) / 2.0, y, s, r, g, b, font);
}

inline void drawPanel(double x, double y, double w, double h,
                      int fillR, int fillG, int fillB,
                      int lineR, int lineG, int lineB) {
    iSetColor(fillR, fillG, fillB);
    iFilledRectangle(x, y, w, h);
    iSetColor(lineR, lineG, lineB);
    iRectangle(x, y, w, h);
}

// ==== 9. THE POPUP WINDOW LOOK ====
// Every popup in the game is drawn by this one function, so they all
// match: dark veil over the scene, a 500x400 panel, a title bar and a
// title. The caller then draws its own contents inside.
inline void drawPopupFrame(const char* title) {
    // Veil the whole screen so the popup stands out.
    iSetColor(4, 10, 18);
    for (int i = 0; i < SCREEN_H; i += 4)      // gaps let the scene show through
        iFilledRectangle(0, i, SCREEN_W, 2);

    drawPanel(POPUP_X, POPUP_Y, POPUP_W, POPUP_H, 14, 30, 48, 90, 170, 215);
    // Inner highlight line, gives the panel a little depth.
    iSetColor(40, 80, 110);
    iRectangle(POPUP_X + 4, POPUP_Y + 4, POPUP_W - 8, POPUP_H - 8);

    // Title bar across the top of the popup.
    iSetColor(26, 62, 92);
    iFilledRectangle(POPUP_X + 4, POPUP_Y + POPUP_H - 52, POPUP_W - 8, 48);
    drawTextCentred(SCREEN_W / 2.0, POPUP_Y + POPUP_H - 36, title, 255, 235, 165);
}

// A small hint line pinned just under a popup.
inline void drawPopupHint(const char* text) {
    drawTextCentred(SCREEN_W / 2.0, POPUP_Y - 70, text, 190, 212, 232);
}

#endif
