// =====================================================================
// iMain.cpp - the starting point of AquaFeast.
//
// This file does 3 jobs and nothing else:
//   1. load everything once, at startup
//   2. iDraw()       - decide WHICH screen to draw
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

bool kMute = false, kRestart = false;

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
        case SCR_SPLASH:    drawSplash();           break;
        case SCR_MENU:      drawMenu();             break;
        case SCR_NAME:      drawNamePopup();        break;
        case SCR_CHARACTER: drawCharacterPopup();   break;
        case SCR_MAP:       drawLevelMapPopup();    break;
        case SCR_SCORES:    drawScorePopup();       break;
        case SCR_HELP:      drawHelpPopup();        break;
        case SCR_CREDITS:   drawCreditPopup();      break;

        case SCR_PLAY:
            drawLevel();
            if (endPopup != END_NONE) drawEndPopup();
            break;
    }
}

// ==== 3. GAMEPLAY INPUT ====
void handleGameKeys() {
    // SPACE always goes through the SAME tap detector, whether hooked or
    // not - one press does one thing (one jump, or one tug-of-war tap),
    // instead of jumping using a held-key check while the hook used a
    // tap check. This also stops holding Space from auto-bouncing the
    // fish the instant it lands.
    bool spaceTapped = tapped(isKeyPressed(KEY_SPACE) != 0, kSpace);

    // While hooked, swimming is disabled - only SPACE matters.
    if (playerIsHooked()) {
        if (spaceTapped) hookSpaceTap();
        return;
    }

    if (keyLeft())  movePlayer(-1, 0);
    if (keyRight()) movePlayer(1, 0);
    if (keyUp())    movePlayer(0, 1);
    if (keyDown())  movePlayer(0, -1);
    if (spaceTapped) startJump();
}

// Mute (M) and Restart (R) work during play, from the keyboard or from
// the HUD buttons (see iMouse below).
void handleGlobalKeys() {
    if (tapped(isKeyPressed('m') || isKeyPressed('M'), kMute)) toggleMute();

    if (screen != SCR_PLAY || endPopup != END_NONE) return;
    if (tapped(isKeyPressed('r') || isKeyPressed('R'), kRestart)) restartLevel();
}

// What BACKSPACE does depends on which screen you are on. It is the
// "Back" button for every popup that has one.
void handleBackKey() {
    if (!tapped(isKeyPressed(KEY_BACKSPACE) != 0, kBack)) return;

    switch (screen) {
        case SCR_NAME:
            // Erase the last letter, or go back if the name is empty.
            if (playerName[0] != '\0') playerName[strlen(playerName) - 1] = '\0';
            else screen = SCR_MENU;
            break;
        case SCR_CHARACTER: screen = SCR_NAME; break;
        case SCR_MAP:       screen = SCR_MENU; playButton(); break;
        case SCR_SCORES:
        case SCR_HELP:
        case SCR_CREDITS:   screen = SCR_MENU; playButton(); break;
        // Win / Game Over / Level Up have NO back button, so backspace
        // only leaves a level while it is still being played.
        case SCR_PLAY:      if (endPopup == END_NONE) returnToMenu(); break;
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
// The menu background keeps living on every non-gameplay screen, so the
// popups always sit over a moving ocean.
void updateMenuScene() {
    updateMenuFish();
    updateEnvironment();
    updateSeaBubbles();
}

void fixedUpdate() {
    handleGlobalKeys();
    handleBackKey();

    switch (screen) {
        case SCR_SPLASH:    updateSplash(); return;
        case SCR_MENU:      updateMenuScene(); updateMenu(); return;
        case SCR_NAME:      updateMenuScene(); readNameTyping(); updateNamePopup(); return;
        case SCR_CHARACTER: updateMenuScene(); updateCharacterPopup(); return;
        case SCR_MAP:       updateMenuScene(); updateLevelMapPopup(); return;
        case SCR_SCORES:    updateMenuScene(); updateScorePopup(); return;
        case SCR_HELP:
        case SCR_CREDITS:   updateMenuScene(); return;
        case SCR_PLAY:      break;
    }

    // --- gameplay ---
    refreshEndPopup();
    if (endPopup != END_NONE) {
        // A finished run saves its score once, then waits on the popup.
        if (isGameOver) saveRunScore();
        updateEndPopup();
        return;
    }

    handleGameKeys();
    updateLevel();
}

// ==== 6. MOUSE ====
void iMouse(int button, int state, int mx, int my) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

    if (screen == SCR_PLAY) {
        if (endPopup != END_NONE) return;      // popup is keyboard-driven
        HudButton b = hudButtonAt(mx, my);
        if (b == HUD_MUTE) toggleMute();
        else if (b == HUD_RESTART) restartLevel();
        return;
    }

    if (screen == SCR_SPLASH) return;

    // On the menu: the mute button first, then the 5 menu buttons
    // themselves, then the fish (in that order, so a fish sitting over
    // a button never steals the click).
    if (screen == SCR_MENU) {
        int hit = menuButtonAt(mx, SCREEN_H - my);   // mouse y is from the top
        if (hit >= 0) {
            menuIndex = hit;
            playButton();
            openMenuChoice();
            return;
        }
    }
}

void iMouseMove(int mx, int my) {}
void iPassiveMouseMove(int mx, int my) {}

// ==== 7. ONE-SECOND CLOCK ====
void tickClock() {
    if (screen == SCR_PLAY && endPopup == END_NONE) tickLevelClock();
}

// ==== 8. START ====
int main() {
    srand((unsigned int)time(NULL));

    iSetTimer(1000, tickClock);
    iInitialize(SCREEN_W, SCREEN_H, "AquaFeast", /*keyboardSamplingRate=*/30);

    loadEverything();

    iStart();
    return 0;
}
