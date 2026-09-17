// iMain.cpp
// ==== 1. INCLUDES ====
#include "iGraphics.h"
// windows.h + mmsystem.h: only needed so main() can link winmm.lib for
// mciSendString() (the sound-playing calls live in menu.hpp).
#include <mmsystem.h>
#include <windows.h>
#pragma comment(lib, "winmm.lib")

#include "boats.hpp"
#include "Boat.hpp"
#include "entities.hpp"
#include "environment.hpp"
#include "levels.hpp"
#include "menu.hpp"
#include "player.hpp"
#include "utility.hpp"

// ==== 2/3/4. STATE ====
int bgImage;
int gameOverImage, winImage; // loaded once below, not every frame
bool wasBackspace = false, wasRestart = false, wasMute = false;

// ==== 5. FUNCTIONS ====

void loadAllImages() {
  bgImage = iLoadImage("Images/backgroundImage.png");
  gameOverImage = iLoadImage("Images/gameover.png");
  winImage = iLoadImage("Images/win.png");
  loadPlayer();
  loadEntities();
  loadBoats();
  loadBoat();
  loadEnvironment();
  loadMenuImages();
  loadLevels();
  loadSounds();
}

// Same "loaded once, not every frame" fix as instructions/credits in
// menu.hpp - these used to call iLoadImage() inside iDraw() itself.
void drawGameOverScreen() {
  iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, gameOverImage);
  iSetColor(255, 255, 255);
  iText(SCREEN_WIDTH / 2 - 170, SCREEN_HEIGHT / 2 - 90,
        "Press BACKSPACE to return to the menu", GAME_FONT);
}

void drawLevelCompleteScreen() {
  iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, winImage);
  iSetColor(255, 255, 255);
  iText(SCREEN_WIDTH / 2 - 170, SCREEN_HEIGHT / 2 - 90,
        "Press BACKSPACE to return to the menu", GAME_FONT);
}

void iDraw() {
  iClear();

  if (isInMenu) {
    drawCurrentMenu();
    return;
  }
  if (!isPlaying)
    return;

  levelDraw();
  drawBoat();
  if (isGameOver)
    drawGameOverScreen();
  else if (isLevelComplete)
    drawLevelCompleteScreen();
}

// Reads arrow keys + Space and moves the player - kept out of
// fixedUpdate() so that function stays short and easy to scan.
void handlePlayerInput() {
  // If player is caught in fishing net, freeze movement during catch animation
  if (isPlayerTrappedByNet())
    return;

  if (isSpecialKeyPressed(GLUT_KEY_LEFT))
    movePlayer(-1, 0);
  if (isSpecialKeyPressed(GLUT_KEY_RIGHT))
    movePlayer(1, 0);
  if (isSpecialKeyPressed(GLUT_KEY_UP))
    movePlayer(0, 1);
  if (isSpecialKeyPressed(GLUT_KEY_DOWN))
    movePlayer(0, -1);
  if (isKeyPressed(' '))
    startPlayerJump();
}

void fixedUpdate() {
  if (isPlaying && wasJustPressed(isKeyPressed(KEY_BACKSPACE), wasBackspace)) {
    returnToHomeMenu();
    return;
  }

  if (isInMenu) {
    updateMenuNavigation();
    if (requestLevelStart) {
      requestLevelStart = false;
      levelInitialize(currentLevel);
      initBoat(currentLevel, levelConfigs[currentLevel].worldWidth);
    }
    return;
  }

  // Restart (R) and Mute (M) work anytime during a level - the mouse
  // versions of these same actions are handled in iMouse() below.
  if (wasJustPressed(isKeyPressed(KEY_RESTART), wasRestart)) {
    levelInitialize(currentLevel);
    resetBoat(currentLevel, levelConfigs[currentLevel].worldWidth);
    return;
  }
  if (wasJustPressed(isKeyPressed(KEY_MUTE), wasMute))
    toggleMute();

  if (isGameOver || isLevelComplete)
    return;

  handlePlayerInput();
  levelUpdate();
  updateBoat(levelConfigs[currentLevel].worldWidth);
}

void tickGameClock() {
  if (isPlaying && !isGameOver && !isLevelComplete)
    tickLevelClock();
}

// The Restart/Mute HUD icons are mouse-clickable during gameplay -
// checkHudClick() (environment.hpp) does the hit-testing, this just
// decides what to do about it (kept here since it needs levels.hpp,
// which environment.hpp can't include without a circular dependency).
void iMouse(int button, int state, int mouseX, int mouseY) {
  if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
    return;

  if (isInMenu) {
    if (currentMenuScreen == MENU_CHARACTER_SELECT || gameState == CHARACTER_SELECT) {
      handleCharacterSelectMouse(mouseX, mouseY);
    }
    return;
  }

  if (!isPlaying)
    return;

  HudClick click = checkHudClick(mouseX, mouseY);
  if (click == CLICK_RESTART) {
    levelInitialize(currentLevel);
    resetBoat(currentLevel, levelConfigs[currentLevel].worldWidth);
  } else if (click == CLICK_MUTE) {
    toggleMute();
  }
}
void iMouseMove(int mouseX, int mouseY) {}
void iPassiveMouseMove(int mouseX, int mouseY) {}

int main() {
  srand((unsigned int)time(NULL));

  iSetTimer(1000, tickGameClock);
  iInitialize(SCREEN_WIDTH, SCREEN_HEIGHT, "AquaFeast",
              /*keyboardSamplingRate=*/30);

  // iGraphics.h doesn't enable alpha blending by default - without this
  // the bubble effect would draw fully solid instead of see-through.
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  loadAllImages();
  homeMenuInitialize();

  iStart();
  return 0;
}
