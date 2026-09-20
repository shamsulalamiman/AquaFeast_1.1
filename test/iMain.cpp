// =====================================================================
// iMain.cpp - the starting point of AquaFeast.
//
// This file does 3 jobs and nothing else:
//   1. load everything once, at startup
//   2. iDraw()      - decide WHICH screen to draw
//   3. fixedUpdate() - decide WHICH screen to update
// All the real work lives in the .hpp files.
// =====================================================================
#include "iGraphics.h"

// winmm.lib provides mciSendString(), used by sound.hpp to play mp3s.
#pragma comment(lib, "winmm.lib")

#include "utility.hpp"
#include "sound.hpp"
#include "scores.hpp"
#include "player.hpp"
#include "entities.hpp"
#include "boat.hpp"
#include "environment.hpp"
#include "hud.hpp"
#include "levels.hpp"
#include "menu.hpp"

// ==== 1. LOAD EVERYTHING ONCE ====
// Never call iLoadImage() while drawing - loading from disk every frame
// makes the game stutter and input feel broken.
void loadEverything() {
    loadPlayer();
    loadEntities();
    loadBoat();
    loadEnvironment();
    loadHud();
    loadMenuImages();
    loadLevels();
    loadSounds();
    loadScores();
}

// ==== 2. DRAW ====
void iDraw() {
    iClear();

    switch (screen) {
        case SCR_SPLASH:    drawSplash();          break;
        case SCR_MENU:      drawMenu();            break;
        case SCR_NAME:      drawNameEntry();       break;
        case SCR_CHARACTER: drawCharacterSelect(); break;
        case SCR_MAP:       drawLevelMap();        break;
        case SCR_SCORES:    drawScores();          break;
        case SCR_HELP:      drawHelp();            break;
        case SCR_CREDITS:   drawCredits();         break;

        case SCR_PLAY:
            drawLevel();
            if (isGameOver)      drawGameOver();
            else if (isLevelWon) drawLevelWon();
            break;
    }
}

// ==== 3. GAMEPLAY INPUT ====
void handleGameKeys() {
    // While hooked, the arrow keys do nothing - only SPACE matters.
    if (playerIsHooked()) {
        if (tapped(isKeyPressed(KEY_SPACE) != 0, kSpace)) hookSpaceTap();
        return;
    }
    kSpace = isKeyPressed(KEY_SPACE) != 0;   // keep the tap detector in sync

    if (isSpecialKeyPressed(GLUT_KEY_LEFT))  movePlayer(-1, 0);
    if (isSpecialKeyPressed(GLUT_KEY_RIGHT)) movePlayer(1, 0);
    if (isSpecialKeyPressed(GLUT_KEY_UP))    movePlayer(0, 1);
    if (isSpecialKeyPressed(GLUT_KEY_DOWN))  movePlayer(0, -1);
    if (isKeyPressed(KEY_SPACE))             startJump();
}

// What BACKSPACE does depends on which screen you are on.
void handleBackKey() {
    if (!tapped(isKeyPressed(KEY_BACKSPACE) != 0, kBack)) return;

    switch (screen) {
        case SCR_NAME:
            // Erase the last letter, or go back if the name is empty.
            if (playerName[0] != '\0') playerName[strlen(playerName) - 1] = '\0';
            else screen = SCR_MENU;
            break;
        case SCR_CHARACTER: screen = SCR_NAME; break;
        case SCR_MAP:       screen = SCR_CHARACTER; break;
        case SCR_SCORES:
        case SCR_HELP:
        case SCR_CREDITS:   screen = SCR_MENU; playButton(); break;
        case SCR_PLAY:      returnToMenu(); break;
        default: break;
    }
}

// ==== 4. TYPING THE NICKNAME ====
// This version of iGraphics has NO iKeyboard() callback - the only way
// to read the keyboard is isKeyPressed(). So to type a name we check
// each letter/digit key ourselves and remember which were already held,
// so holding a key types one letter instead of hundreds.
bool letterHeld[128] = { false };

void addNameChar(char c) {
    int len = (int)strlen(playerName);
    if (len >= 14) return;           // keep names short enough to display
    playerName[len] = c;
    playerName[len + 1] = '\0';
}

void readNameTyping() {
    for (char c = 'a'; c <= 'z'; c++)
        if (tapped(isKeyPressed(c) != 0, letterHeld[(int)c]))
            addNameChar(c - 32);     // store uppercase so names look tidy

    for (char c = 'A'; c <= 'Z'; c++)
        if (tapped(isKeyPressed(c) != 0, letterHeld[(int)c]))
            addNameChar(c);

    for (char c = '0'; c <= '9'; c++)
        if (tapped(isKeyPressed(c) != 0, letterHeld[(int)c]))
            addNameChar(c);
}

// ==== 5. UPDATE ====
void fixedUpdate() {
    handleBackKey();

    switch (screen) {
        case SCR_SPLASH:    updateSplash();          return;
        case SCR_MENU:      updateMenuFish(); updateEnvironment(); updateMenu(); return;
        case SCR_NAME:      updateMenuFish(); updateEnvironment(); readNameTyping(); updateNameEntry(); return;
        case SCR_CHARACTER: updateMenuFish(); updateEnvironment(); updateCharacterSelect(); return;
        case SCR_MAP:       updateMenuFish(); updateEnvironment(); updateLevelMap(); return;
        case SCR_SCORES:
        case SCR_HELP:
        case SCR_CREDITS:   updateMenuFish(); updateEnvironment(); return;
        case SCR_PLAY:      break;
    }

    // --- gameplay ---
    // Losing every life ends the run too, so the score is saved here as
    // well as on a win or a timeout.
    if (isGameOver) { saveRunScore(); return; }

    if (isLevelWon) {
        // ENTER moves on to the next level once one is finished.
        if (tapped(isKeyPressed(KEY_ENTER) != 0, kEnter) && currentLevel < MAX_LEVELS)
            startLevel(currentLevel + 1);
        return;
    }

    handleGameKeys();
    updateLevel();
}

// ==== 6. MOUSE ====
// The only click in the game: scaring a fish on the menu background.
void iMouse(int button, int state, int mx, int my) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;
    if (screen == SCR_PLAY || screen == SCR_SPLASH) return;
    clickMenuFish(mx, my);
}

void iMouseMove(int mx, int my) {}
void iPassiveMouseMove(int mx, int my) {}

// ==== 7. ONE-SECOND CLOCK ====
void tickClock() {
    if (screen == SCR_PLAY && !isGameOver && !isLevelWon) tickLevelClock();
}

// ==== 8. START ====
int main() {
    srand((unsigned int)time(NULL));

    iSetTimer(1000, tickClock);
    iInitialize(SCREEN_W, SCREEN_H, "AquaFeast", /*keyboardSamplingRate=*/30);

    // iGraphics does not turn on transparency by default. Without this,
    // bubbles and the menu's dark veil would draw as solid blocks.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    loadEverything();

    iStart();
    return 0;
}
