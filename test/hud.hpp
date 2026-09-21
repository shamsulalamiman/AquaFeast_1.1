#ifndef HUD_HPP
#define HUD_HPP
// =====================================================================
// hud.hpp - the status bar across the top of the screen.
//
//   [level] [lives] [time] [score]   progress bar with a fish marker
//                                    ... and Mute + Restart buttons
// The two buttons are clickable; iMain.cpp asks hudButtonAt() which one
// (if any) the mouse landed on.
// =====================================================================
#include "utility.hpp"
#include "player.hpp"

int icoLevel, icoLife, icoTime, icoScore;
int icoMuteOn, icoMuteOff, icoRestart, icoProgressFish;

// Button positions, kept as constants so drawing and clicking can never
// disagree about where they are.
const double BTN_SIZE = 40.0;
const double BTN_Y = SCREEN_H - HUD_H + 16;
const double MUTE_X = SCREEN_W - 110;
const double RESTART_X = SCREEN_W - 56;

enum HudButton { HUD_NONE, HUD_MUTE, HUD_RESTART };

void loadHud() {
    icoLevel = loadImg("Images/HUD/level.png");
    icoLife  = loadImg("Images/HUD/life.png");
    icoTime  = loadImg("Images/HUD/timer.png");
    icoScore = loadImg("Images/HUD/coin.png");

    icoMuteOn  = loadImg("Images/HUD/mute_on.png",  "Images/HUD/coin.png");
    icoMuteOff = loadImg("Images/HUD/mute_off.png", "Images/HUD/coin.png");
    icoRestart = loadImg("Images/HUD/restart.png",  "Images/HUD/level.png");
    // The little fish that rides along the progress bar.
    icoProgressFish = loadImg("Images/HUD/progress_fish.png", "Images/Fish/prey_small_01.png");
}

// Draws one "icon + text" pair and returns where the next should start.
double drawHudItem(double x, int icon, const char* text) {
    double y = SCREEN_H - HUD_H + 18;
    iShowImage((int)x, (int)y, 38, 38, icon);
    drawText(x + 48, y + 12, text, 255, 255, 255);
    return x + 48 + 120;
}

// The progress bar fills as the fish grows, and a small fish icon rides
// along the top showing exactly how far through the level you are.
void drawProgressBar(double x) {
    double y = SCREEN_H - HUD_H + 24;
    double w = 300, h = 24;

    drawPanel(x, y, w, h, 28, 34, 46, 150, 170, 200);

    double pct = clampD(stats.progress, 0.0, 1.0);
    double fill = pct * (w - 4);

    // Green while growing, gold once the level goal is reached.
    if (pct >= 1.0) iSetColor(255, 205, 70);
    else             iSetColor(70, 200, 130);
    iFilledRectangle(x + 2, y + 2, fill, h - 4);

    // Halfway marker - where the level's danger enemy arrives.
    iSetColor(255, 255, 255);
    iLine(x + w / 2, y, x + w / 2, y + h);

    // The fish marker rides the end of the filled part.
    iShowImage((int)(x + fill - 16), (int)(y + h - 6), 30, 30, icoProgressFish);

    char pctText[16];
    sprintf_s(pctText, "%d%%", (int)(pct * 100));
    drawText(x + w + 14, y + 6, pctText, 210, 225, 240);
}

void drawHud() {
    iSetColor(16, 22, 34);
    iFilledRectangle(0, SCREEN_H - HUD_H, SCREEN_W, HUD_H);
    iSetColor(60, 90, 130);
    iLine(0, SCREEN_H - HUD_H, SCREEN_W, SCREEN_H - HUD_H);

    char buf[48];
    double x = 26;

    sprintf_s(buf, "%d", stats.level);
    x = drawHudItem(x, icoLevel, buf);

    sprintf_s(buf, "%d", stats.lives);
    x = drawHudItem(x, icoLife, buf);

    sprintf_s(buf, "%ds", stats.timeLeft);
    x = drawHudItem(x, icoTime, buf);

    sprintf_s(buf, "%d", stats.score);
    x = drawHudItem(x, icoScore, buf);

    drawProgressBar(x + 20);

    // Mute and Restart buttons on the far right.
    iShowImage((int)MUTE_X, (int)BTN_Y, (int)BTN_SIZE, (int)BTN_SIZE,
               isMuted ? icoMuteOff : icoMuteOn);
    iShowImage((int)RESTART_X, (int)BTN_Y, (int)BTN_SIZE, (int)BTN_SIZE, icoRestart);
}

// Which HUD button is under this mouse point (mouse y is measured from
// the top of the window, so it is flipped first).
HudButton hudButtonAt(int mouseX, int mouseY) {
    double y = SCREEN_H - mouseY;
    if (inBox(mouseX, y, MUTE_X, BTN_Y, BTN_SIZE, BTN_SIZE))    return HUD_MUTE;
    if (inBox(mouseX, y, RESTART_X, BTN_Y, BTN_SIZE, BTN_SIZE)) return HUD_RESTART;
    return HUD_NONE;
}

#endif
