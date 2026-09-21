#ifndef MENU_HPP
#define MENU_HPP
// =====================================================================
// menu.hpp - every screen that is NOT gameplay.
//
//   SPLASH -> MENU -> (Start) -> NAME -> CHARACTER -> MAP -> gameplay
//                  -> Instructions / Scores / Credits / Exit
//
// Every non-menu screen is drawn as a centred 500x400 POPUP over the
// living ocean background, so the whole game feels like one place.
//
// After finishing a level: Level Up popup -> the map reopens with the
// new level unlocked -> that level starts on its own. The name is only
// ever asked for once.
// =====================================================================
#include "utility.hpp"
#include "sound.hpp"
#include "scores.hpp"
#include "environment.hpp"
#include "entities.hpp"
#include "levels.hpp"

// ==== 1. MENU ITEMS ====
#define MENU_COUNT 5
const char* menuLabels[MENU_COUNT] = { "START", "INSTRUCTION", "SCORE", "CREDIT", "EXIT" };
int menuIndex = 0;

int splashImage, helpImage, creditsImage;
int btnNormal, btnBright;        // the two button image states
int menuFishIcon;                // the little marker beside the selected row

int splashTicks = 0;
const int SPLASH_LENGTH = 150;   // ~4.5 seconds at a 30ms tick

// True when both button pictures exist. Worked out once at load time.
bool buttonArtReady = false;

// Key edge-detectors.
bool kUp = false, kDown = false, kLeft = false, kRight = false;
bool kEnter = false, kBack = false, kSpace = false, kDelete = false;

// What the end-of-run popup should say.
enum EndPopup { END_NONE, END_WIN, END_OVER, END_LEVELUP };
EndPopup endPopup = END_NONE;

// Set when the Level Up popup is dismissed, so the map knows to open on
// the newly unlocked level and start it automatically.
bool autoStartNextLevel = false;

// ==== 2. MENU BACKGROUND FISH ====
// Separate from the gameplay fish so entering a level never disturbs
// them. Shows every fish type: all 3 prey looks AND both predators.
#define MENU_FISH 18
struct MenuFish {
    double x, y, dx, dy, size;
    int look;          // 0..2 prey looks, 3..4 predator kinds
    int animTick;
    bool faceLeft;
    int dartTicks;
};
MenuFish menuFish[MENU_FISH];

void setupMenuFish() {
    for (int i = 0; i < MENU_FISH; i++) {
        menuFish[i].x = randRange(0, SCREEN_W);
        menuFish[i].y = randRange(80, SEA_Y - 80);
        menuFish[i].dx = randRange(-1.4, 1.4);
        if (fabs(menuFish[i].dx) < 0.3) menuFish[i].dx = 0.7;   // never stuck still
        menuFish[i].dy = randRange(-0.4, 0.4);
        // Most are prey; every 5th is a bigger predator.
        bool isPred = (i % 5 == 4);
        menuFish[i].look = isPred ? (3 + rand() % 2) : (rand() % PREY_LOOKS);
        menuFish[i].size = isPred ? randRange(44, 58) : randRange(22, 38);
        menuFish[i].animTick = rand() % 60;
        menuFish[i].faceLeft = menuFish[i].dx < 0;
        menuFish[i].dartTicks = 0;
    }
}

void updateMenuFish() {
    for (int i = 0; i < MENU_FISH; i++) {
        MenuFish &f = menuFish[i];
        double speed = (f.dartTicks > 0) ? 3.4 : 1.0;

        f.x += f.dx * speed;
        f.y += f.dy * speed;
        f.animTick++;
        if (f.dartTicks > 0) f.dartTicks--;

        if (f.x < -90) f.x = SCREEN_W + 80;
        if (f.x > SCREEN_W + 90) f.x = -80;

        // Bounce off the top and bottom of the water.
        if (f.y < 80)         { f.y = 80;         f.dy = fabs(f.dy); }
        if (f.y > SEA_Y - 80) { f.y = SEA_Y - 80; f.dy = -fabs(f.dy); }

        f.faceLeft = f.dx < 0;
    }
}

// Picks the right picture for a menu fish, prey or predator.
int menuFishSprite(const MenuFish &f) {
    int frame = (f.animTick / 8) % SWIM_FRAMES;
    if (f.look < PREY_LOOKS)
        return preySprite[f.look][frame][f.faceLeft ? 1 : 0];
    return predSprite[f.look - 3][frame][f.faceLeft ? FACE_LEFT : FACE_RIGHT];
}

void drawMenuFish() {
    for (int i = 0; i < MENU_FISH; i++) {
        MenuFish &f = menuFish[i];
        double s = f.size * 2.0;
        iShowImage((int)(f.x - s / 2), (int)(f.y - s / 2), (int)s, (int)s, menuFishSprite(f));
    }
}

// Clicking a fish sends it darting off in a new direction. The test
// uses the fish's real drawn size, so big and small fish are both easy
// to hit - the old version missed most clicks on large fish.
void clickMenuFish(double mouseX, double mouseY) {
    double y = SCREEN_H - mouseY;        // mouse y comes from the top

    int best = -1;
    double bestDist = 1e9;
    for (int i = 0; i < MENU_FISH; i++) {
        double d = dist(mouseX, y, menuFish[i].x, menuFish[i].y);
        if (d < menuFish[i].size + 14 && d < bestDist) { bestDist = d; best = i; }
    }
    if (best < 0) return;

    MenuFish &f = menuFish[best];
    double angle = randRange(0, 6.283);
    f.dx = cos(angle) * 1.8;
    if (fabs(f.dx) < 0.4) f.dx = (f.dx < 0 ? -0.8 : 0.8);
    f.dy = sin(angle) * 0.8;
    f.dartTicks = 50;
    playButton();
}

// The menu's living ocean: the same scenery as level 1, plus the fish.
void drawMenuBackdrop() {
    int saved = currentLevel;
    currentLevel = 1;          // always the bright sunny ocean
    drawEnvironment();
    currentLevel = saved;
    drawMenuFish();
}

// ==== 3. LOADING ====
void loadMenuImages() {
    splashImage  = loadImg("Images/splash.png", "Images/backgroundImage.png");
    helpImage    = loadImg("Images/instruction.png");
    creditsImage = loadImg("Images/credit.png");

    // The two button states. If the art is missing, drawButton() falls
    // back to drawing plain coloured panels instead.
    btnNormal = loadImg("Images/Menu/button_normal.png");
    btnBright = loadImg("Images/Menu/button_selected.png");
    // Checked ONCE here, not while drawing - imageExists() opens the
    // file from disk, which must never happen every frame.
    buttonArtReady = imageExists("Images/Menu/button_normal.png") &&
                     imageExists("Images/Menu/button_selected.png");
    menuFishIcon = loadImg("Images/Menu/select_fish.png", "Images/Fish/prey_small_01.png");

    setupMenuFish();
}

// ==== 4. SHARED WIDGETS ====
// One button row. Uses the bright/normal images when they exist, and
// falls back to drawn panels when they do not.
void drawButton(double cx, double y, double w, double h, const char* label,
                bool selected, bool locked) {
    double x = cx - w / 2;

    if (buttonArtReady) {
        iShowImage((int)x, (int)y, (int)w, (int)h, selected ? btnBright : btnNormal);
    }
    else if (locked)   drawPanel(x, y, w, h, 40, 40, 48, 90, 90, 100);
    else if (selected) drawPanel(x, y, w, h, 40, 150, 210, 200, 240, 255);
    else               drawPanel(x, y, w, h, 18, 40, 62, 90, 130, 170);

    int r = 255, g = 255, b = 255;
    if (locked)             { r = 135; g = 135; b = 145; }
    else if (selected && !buttonArtReady) { r = 10; g = 20; b = 30; }
    else if (selected)       { r = 255; g = 240; b = 170; }

    drawTextCentred(cx, y + h / 2.0 - 5, label, r, g, b);

    // The fish marker swims in beside whichever row is selected.
    if (selected && !locked)
        iShowImage((int)(x - 52), (int)(y + h / 2.0 - 16), 44, 32, menuFishIcon);
}

// A row of small buttons inside a popup (e.g. Restart / Main Menu).
void drawPopupButton(double cx, double y, double w, const char* label, bool selected) {
    drawButton(cx, y, w, 44, label, selected, false);
}

// Main-menu-only button: always drawn in its plain (unselected) image
// state, with NO highlight box, border, or colour tint. The small fish
// icon beside the row is the ONLY thing that shows which one is
// selected - this is separate from drawButton() so the popups and
// level map (which still use the highlight box) are unaffected.
void drawMenuButton(double cx, double y, double w, double h, const char* label, bool selected) {
    double x = cx - w / 2;

    if (buttonArtReady) iShowImage((int)x, (int)y, (int)w, (int)h, btnNormal);
    else                 drawPanel(x, y, w, h, 18, 40, 62, 90, 130, 170);

    drawTextCentred(cx, y + h / 2.0 - 5, label, 255, 255, 255);

    if (selected)
        iShowImage((int)(x - 56), (int)(y + h / 2.0 - 16), 44, 32, menuFishIcon);
}

// Geometry for the 5 main-menu buttons, kept as one function so drawing
// and mouse-click detection can never disagree about where they are.
double menuButtonX() { return SCREEN_W / 2.0 - 360.0 / 2.0; }
double menuButtonY(int i) { return SCREEN_H - 290 - i * 76; }
const double MENU_BTN_W = 360.0, MENU_BTN_H = 56.0;

// Which main-menu button (if any) contains this screen point.
int menuButtonAt(double px, double py) {
    for (int i = 0; i < MENU_COUNT; i++)
        if (inBox(px, py, menuButtonX(), menuButtonY(i), MENU_BTN_W, MENU_BTN_H)) return i;
    return -1;
}

// ==== 5. SPLASH SCREEN ====
// A loading page with a progress bar and a fish that swims along it.
void drawSplash() {
    iShowImage(0, 0, SCREEN_W, SCREEN_H, splashImage);

    double barW = 640, barH = 26;
    double x = SCREEN_W / 2.0 - barW / 2, y = 180;
    double pct = clampD((double)splashTicks / SPLASH_LENGTH, 0, 1);

    drawPanel(x, y, barW, barH, 12, 18, 28, 190, 215, 235);
    iSetColor(70, 200, 230);
    iFilledRectangle(x + 2, y + 2, (barW - 4) * pct, barH - 4);

    // The fish swims along as the bar fills, and bobs while it goes.
    double fishX = x + (barW - 4) * pct - 20;
    double bob = sin(splashTicks * 0.22) * 6.0;
    iShowImage((int)fishX, (int)(y + barH - 8 + bob), 52, 38, menuFishIcon);

    char buf[48];
    sprintf_s(buf, "LOADING  %d%%", (int)(pct * 100));
    drawTextCentred(SCREEN_W / 2.0, y + barH + 34, buf, 235, 245, 255);
    drawTextCentred(SCREEN_W / 2.0, y - 38, "press ENTER to skip", 180, 200, 220);
}

void updateSplash() {
    splashTicks++;
    bool skip = tapped(isKeyPressed(KEY_ENTER) != 0, kEnter);
    if (splashTicks >= SPLASH_LENGTH || skip) {
        screen = SCR_MENU;
        startMenuMusic();
    }
}

// ==== 6. MAIN MENU ====
void drawMenu() {
    drawMenuBackdrop();

    drawTextCentred(SCREEN_W / 2.0, SCREEN_H - 150, "A Q U A F E A S T", 255, 240, 170, BIG_FONT);

    for (int i = 0; i < MENU_COUNT; i++)
        drawMenuButton(SCREEN_W / 2.0, menuButtonY(i), MENU_BTN_W, MENU_BTN_H, menuLabels[i], i == menuIndex);

    char buf[64];
    sprintf_s(buf, "BEST SCORE: %d", bestScore());
    drawTextCentred(SCREEN_W / 2.0, 150, buf, 255, 225, 120);

    // Mute button, bottom right, matching the HUD one.
    iShowImage(SCREEN_W - 80, 60, 44, 44, isMuted ? icoMuteOff : icoMuteOn);
    drawText(SCREEN_W - 150, 74, "M", 200, 220, 235);

    drawTextCentred(SCREEN_W / 2.0, 90,
        "UP/DOWN or W/S to move    ENTER to select    M to mute    click a fish", 195, 215, 235);
}

void openMenuChoice() {
    if (menuIndex == 0) {
        // The name is only asked for the first time.
        if (haveName) screen = SCR_MAP;
        else           screen = SCR_NAME;
    }
    else if (menuIndex == 1) screen = SCR_HELP;
    else if (menuIndex == 2) screen = SCR_SCORES;
    else if (menuIndex == 3) screen = SCR_CREDITS;
    else exit(0);
}

void updateMenu() {
    if (tapped(keyUp(), kUp))   { menuIndex = (menuIndex - 1 + MENU_COUNT) % MENU_COUNT; playButton(); }
    if (tapped(keyDown(), kDown)) { menuIndex = (menuIndex + 1) % MENU_COUNT; playButton(); }
    if (tapped(isKeyPressed(KEY_ENTER) != 0, kEnter)) { playButton(); openMenuChoice(); }
}

// Clicking the menu's mute button.
bool clickedMenuMute(double mouseX, double mouseY) {
    double y = SCREEN_H - mouseY;
    return inBox(mouseX, y, SCREEN_W - 80, 60, 44, 44);
}

// ==== 7. NICKNAME POPUP ====
void drawNamePopup() {
    drawMenuBackdrop();
    drawPopupFrame("ENTER YOUR NAME");

    double boxW = 380, x = SCREEN_W / 2.0 - boxW / 2, y = POPUP_Y + 200;
    drawPanel(x, y, boxW, 56, 10, 22, 34, 120, 190, 230);

    char shown[32];
    sprintf_s(shown, "%s%s", playerName, ((splashTicks / 12) % 2 == 0) ? "_" : " ");
    drawText(x + 20, y + 20, shown, 255, 255, 255, BIG_FONT);

    drawTextCentred(SCREEN_W / 2.0, y + 80, "one word, letters and numbers", 180, 200, 220);
    drawPopupButton(SCREEN_W / 2.0, POPUP_Y + 60, 200, "BACK", false);
    drawPopupHint("ENTER to continue    BACKSPACE to erase");
}

void updateNamePopup() {
    splashTicks++;   // reused purely to blink the cursor
    if (tapped(isKeyPressed(KEY_ENTER) != 0, kEnter)) {
        if (playerName[0] == '\0') return;   // a name is required
        haveName = true;
        playButton();
        screen = SCR_CHARACTER;
    }
}

// ==== 8. FISH SELECTION POPUP ====
int skinIndex = 0;
const char* skinNames[3] = { "REEF DARTER", "CORAL GLIDER", "DEEP RUNNER" };

void drawCharacterPopup() {
    drawMenuBackdrop();
    drawPopupFrame("CHOOSE YOUR FISH");

    double cardW = 130, gap = 16;
    double totalW = cardW * 3 + gap * 2;
    double startX = SCREEN_W / 2.0 - totalW / 2;
    double y = POPUP_Y + 150;

    for (int i = 0; i < 3; i++) {
        double x = startX + i * (cardW + gap);
        bool sel = (i == skinIndex);

        if (sel) drawPanel(x, y, cardW, 160, 22, 66, 96, 130, 225, 255);
        else      drawPanel(x, y, cardW, 160, 10, 24, 38, 70, 100, 130);

        iShowImage((int)(x + cardW / 2 - 50), (int)(y + 58), 100, 74, skinRight[i]);
        drawTextCentred(x + cardW / 2, y + 28, skinNames[i],
                        sel ? 255 : 170, sel ? 235 : 190, sel ? 140 : 200);
        if (sel) iShowImage((int)(x + cardW / 2 - 20), (int)(y + 168), 40, 30, menuFishIcon);
    }

    drawPopupButton(SCREEN_W / 2.0, POPUP_Y + 60, 200, "BACK", false);
    drawPopupHint("LEFT/RIGHT or A/D to choose    ENTER to confirm");
}

void updateCharacterPopup() {
    if (tapped(keyLeft(), kLeft))   { skinIndex = (skinIndex + 2) % 3; playButton(); }
    if (tapped(keyRight(), kRight)) { skinIndex = (skinIndex + 1) % 3; playButton(); }
    if (tapped(isKeyPressed(KEY_ENTER) != 0, kEnter)) {
        playerSkin = skinIndex;
        playButton();
        screen = SCR_MAP;
    }
}

// ==== 9. LEVEL MAP POPUP ====
int mapIndex = 0;
const char* levelNames[3] = { "SUNNY SHALLOWS", "MIDNIGHT DEEP", "STORM WATERS" };

void beginSelectedLevel() {
    stats.score = 0;
    stats.lives = 3;
    startLevel(mapIndex + 1);
}

void drawLevelMapPopup() {
    drawMenuBackdrop();
    drawPopupFrame("SELECT LEVEL");

    for (int i = 0; i < 3; i++) {
        bool locked = (i + 1) > unlockedLevel;
        double y = POPUP_Y + 250 - i * 62;

        char label[64];
        sprintf_s(label, "%d.  %s%s", i + 1, levelNames[i], locked ? "   (LOCKED)" : "");
        drawButton(SCREEN_W / 2.0 + 20, y, 340, 50, label, i == mapIndex, locked);

        // A small route dot beside each stop, filled once unlocked.
        double dotX = POPUP_X + 46;
        if (locked) iSetColor(90, 95, 105);
        else         iSetColor(90, 220, 150);
        iFilledCircle(dotX, y + 25, 9, 12);
        if (i < 2) {                       // dotted line down to the next stop
            iSetColor(70, 110, 140);
            for (int d = 1; d < 5; d++) iFilledCircle(dotX, y + 25 - d * 12, 2.2, 8);
        }
    }

    char who[64];
    sprintf_s(who, "PLAYER: %s", playerName);
    drawTextCentred(SCREEN_W / 2.0, POPUP_Y + 110, who, 170, 215, 245);

    drawPopupButton(SCREEN_W / 2.0, POPUP_Y + 50, 200, "BACK", false);
    drawPopupHint("UP/DOWN to move    ENTER to play");
}

void updateLevelMapPopup() {
    // Coming back from a Level Up popup: jump straight into the newly
    // unlocked level. Score and lives CARRY OVER here (unlike picking a
    // level by hand), so a full playthrough builds one running total.
    if (autoStartNextLevel) {
        autoStartNextLevel = false;
        mapIndex = unlockedLevel - 1;
        startLevel(mapIndex + 1);
        return;
    }

    if (tapped(keyUp(), kUp))     { mapIndex = (mapIndex + 2) % 3; playButton(); }
    if (tapped(keyDown(), kDown)) { mapIndex = (mapIndex + 1) % 3; playButton(); }
    if (tapped(isKeyPressed(KEY_ENTER) != 0, kEnter)) {
        if (mapIndex + 1 > unlockedLevel) return;   // locked - ignore
        playButton();
        beginSelectedLevel();
    }
}

// ==== 10. SCORE POPUP ====
void drawScorePopup() {
    drawMenuBackdrop();
    drawPopupFrame("HIGH SCORES");

    if (scoreCount == 0)
        drawTextCentred(SCREEN_W / 2.0, POPUP_Y + 220, "no scores yet - go play!", 190, 210, 230);

    // Only the top 7 fit neatly inside a 500x400 popup.
    int show = scoreCount < 7 ? scoreCount : 7;
    for (int i = 0; i < show; i++) {
        char row[80];
        sprintf_s(row, "%d.  %-14s %6d", i + 1, scoreTable[i].name, scoreTable[i].score);
        int bright = (i == 0) ? 255 : 210;
        drawText(POPUP_X + 60, POPUP_Y + 300 - i * 32, row, bright, bright, (i == 0) ? 120 : 230);
    }

    drawPopupButton(SCREEN_W / 2.0 - 110, POPUP_Y + 40, 190, "BACK", false);
    drawPopupButton(SCREEN_W / 2.0 + 110, POPUP_Y + 40, 190, "DELETE ALL", false);
    drawPopupHint("press D to delete all saved scores    BACKSPACE to go back");
}

void updateScorePopup() {
    if (tapped(isKeyPressed('d') || isKeyPressed('D'), kDelete)) {
        clearScores();
        playButton();
    }
}

// ==== 11. INSTRUCTION / CREDIT POPUPS ====
void drawHelpPopup() {
    drawMenuBackdrop();
    drawPopupFrame("INSTRUCTIONS");

    const char* lines[] = {
        "ARROWS or WASD  -  swim",
        "SPACE  -  jump above the water",
        "Eat fish SMALLER than you to grow",
        "Avoid anything BIGGER than you",
        "A warning icon marks real threats",
        "Halfway through, a danger fish arrives",
        "LEVEL 3: mash SPACE to escape the hook",
        "M  -  mute      R  -  restart"
    };
    for (int i = 0; i < 8; i++)
        drawText(POPUP_X + 40, POPUP_Y + 300 - i * 30, lines[i], 220, 232, 244);

    drawPopupButton(SCREEN_W / 2.0, POPUP_Y + 40, 200, "BACK", false);
    drawPopupHint("BACKSPACE to go back");
}

void drawCreditPopup() {
    drawMenuBackdrop();
    drawPopupFrame("CREDITS");

    const char* lines[] = {
        "AQUAFEAST",
        "",
        "A 2D arcade game built with iGraphics",
        "",
        "Course project",
        "AUST CSE"
    };
    for (int i = 0; i < 6; i++)
        drawTextCentred(SCREEN_W / 2.0, POPUP_Y + 290 - i * 34, lines[i],
                        i == 0 ? 255 : 215, i == 0 ? 235 : 228, i == 0 ? 140 : 240);

    drawPopupButton(SCREEN_W / 2.0, POPUP_Y + 40, 200, "BACK", false);
    drawPopupHint("BACKSPACE to go back");
}

// ==== 12. END-OF-RUN POPUPS ====
// Win, Game Over and Level Up all share one layout: the score achieved,
// plus Restart and Main Menu. They have NO back button on purpose - the
// run is finished, so there is nothing to go back to.
int endIndex = 0;   // 0 = Restart, 1 = Main Menu

void drawEndPopup() {
    const char* title = "GAME OVER";
    if (endPopup == END_WIN)          title = "YOU WIN!";
    else if (endPopup == END_LEVELUP) title = "LEVEL UP!";

    drawPopupFrame(title);

    char buf[64];
    sprintf_s(buf, "SCORE: %d", stats.score);
    drawTextCentred(SCREEN_W / 2.0, POPUP_Y + 270, buf, 255, 235, 150, BIG_FONT);

    if (endPopup == END_LEVELUP) {
        char msg[64];
        sprintf_s(msg, "LEVEL %d UNLOCKED", unlockedLevel);
        drawTextCentred(SCREEN_W / 2.0, POPUP_Y + 215, msg, 140, 235, 175);
        drawTextCentred(SCREEN_W / 2.0, POPUP_Y + 180, "ENTER to continue", 200, 220, 240);
    }
    else if (endPopup == END_WIN) {
        drawTextCentred(SCREEN_W / 2.0, POPUP_Y + 210, "You finished every level!", 140, 235, 175);
    }

    drawPopupButton(SCREEN_W / 2.0, POPUP_Y + 110, 250, "RESTART",   endIndex == 0);
    drawPopupButton(SCREEN_W / 2.0, POPUP_Y + 50,  250, "MAIN MENU", endIndex == 1);
    drawPopupHint("UP/DOWN to choose    ENTER to confirm");
}

void returnToMenu() {
    screen = SCR_MENU;
    isGameOver = false;
    isLevelWon = false;
    endPopup = END_NONE;
    menuIndex = 0;
    currentLevel = 1;   // the menu always shows the bright level-1 ocean
    scrollX = 0;
    stopGameMusic();
    startMenuMusic();
}

void updateEndPopup() {
    if (tapped(keyUp(), kUp))     { endIndex = (endIndex + 1) % 2; playButton(); }
    if (tapped(keyDown(), kDown)) { endIndex = (endIndex + 1) % 2; playButton(); }

    if (!tapped(isKeyPressed(KEY_ENTER) != 0, kEnter)) return;
    playButton();

    // Level Up: ENTER always moves on to the newly unlocked level via
    // the map, no matter which button is highlighted.
    if (endPopup == END_LEVELUP && endIndex == 0) {
        endPopup = END_NONE;
        isLevelWon = false;
        autoStartNextLevel = true;
        screen = SCR_MAP;
        return;
    }

    if (endIndex == 0) {          // Restart this level
        endPopup = END_NONE;
        restartLevel();
        return;
    }
    returnToMenu();               // Main Menu
}

// Decides which end popup to show once a run finishes.
void refreshEndPopup() {
    if (isGameOver)  { if (endPopup != END_OVER) { endPopup = END_OVER; endIndex = 0; } return; }
    if (!isLevelWon) { endPopup = END_NONE; return; }

    // Winning the last level is a full WIN; otherwise it is a LEVEL UP.
    EndPopup want = (currentLevel >= MAX_LEVELS) ? END_WIN : END_LEVELUP;
    if (endPopup != want) { endPopup = want; endIndex = 0; }
}

#endif
