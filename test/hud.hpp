#ifndef HUD_HPP
#define HUD_HPP
// =====================================================================
// hud.hpp - the status bar across the top of the screen.
//
// Shows exactly 5 things and nothing else:
//   [level icon] level    [life icon] lives    [timer icon] time
//   [coin icon] score     progress bar (icon + drawn shape)
// =====================================================================
#include "utility.hpp"
#include "player.hpp"

int icoLevel, icoLife, icoTime, icoScore;

void loadHud() {
    icoLevel = loadImg("Images/HUD/level.png");
    icoLife  = loadImg("Images/HUD/life.png");
    icoTime  = loadImg("Images/HUD/timer.png");
    icoScore = loadImg("Images/HUD/coin.png");
}

// Draws one "icon + text" pair and returns where the next one should
// start, so the items space themselves out automatically.
double drawHudItem(double x, int icon, const char* text) {
    double y = SCREEN_H - HUD_H + 16;
    iShowImage((int)x, (int)y, 34, 34, icon);
    drawText(x + 42, y + 10, text, 255, 255, 255);
    return x + 42 + 110;
}

void drawProgressBar(double x) {
    double y = SCREEN_H - HUD_H + 22;
    double w = 260, h = 22;

    drawPanel(x, y, w, h, 28, 34, 46, 150, 170, 200);

    // Filled portion: green while growing, gold once the goal is met.
    double fill = clampD(stats.progress, 0.0, 1.0) * (w - 4);
    if (stats.progress >= 1.0) iSetColor(255, 205, 70);
    else                        iSetColor(70, 200, 130);
    iFilledRectangle(x + 2, y + 2, fill, h - 4);

    drawText(x + w + 12, y + 5, "GROWTH", 210, 225, 240);
}

void drawHud() {
    // Bar background
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

    // Player name sits on the far right, out of the way.
    if (playerName[0] != '\0')
        drawText(SCREEN_W - 230, SCREEN_H - HUD_H + 26, playerName, 150, 200, 235);
}

#endif
