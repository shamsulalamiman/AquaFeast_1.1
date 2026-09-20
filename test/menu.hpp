#ifndef MENU_HPP
#define MENU_HPP
// =====================================================================
// menu.hpp - every screen that is NOT gameplay.
//
//   SPLASH -> MENU -> (Start) -> NAME -> CHARACTER -> MAP -> gameplay
//                  -> Instructions / Scores / Credits / Exit
//
// The menu has a living background: the same ocean and fish from the
// game swim behind the buttons. They cannot be steered, but clicking a
// fish makes it change place rapidly.
// =====================================================================
#include "utility.hpp"
#include "sound.hpp"
#include "scores.hpp"
#include "environment.hpp"
#include "entities.hpp"
#include "levels.hpp"

// ==== 1. MENU ITEMS ====
#define MENU_COUNT 5
const char* menuLabels[MENU_COUNT] = { "START", "INSTRUCTIONS", "SCORES", "CREDITS", "EXIT" };
int menuIndex = 0;

int splashImage, helpImage, creditsImage, winImage, loseImage;
int splashTicks = 0;               // how long the splash has been showing
const int SPLASH_LENGTH = 150;     // ~4.5 seconds at a 30ms tick

// Key edge-detectors (one per key we care about).
bool kUp = false, kDown = false, kLeft = false, kRight = false;
bool kEnter = false, kBack = false, kSpace = false;

// ==== 2. MENU BACKGROUND FISH ====
// These are separate from the gameplay fish so entering a level never
// disturbs them. They just swim in screen space, looping around.
#define MENU_FISH 14
struct MenuFish {
	double x, y, dx, dy, size;
	int look;
	bool faceLeft;
	int dartTicks;      // >0 while darting away from a mouse click
};
MenuFish menuFish[MENU_FISH];

void setupMenuFish() {
	for (int i = 0; i < MENU_FISH; i++) {
		menuFish[i].x = randRange(0, SCREEN_W);
		menuFish[i].y = randRange(60, SEA_Y - 60);
		menuFish[i].dx = randRange(-1.4, 1.4);
		menuFish[i].dy = randRange(-0.4, 0.4);
		menuFish[i].size = randRange(16, 34);
		menuFish[i].look = rand() % PREY_LOOKS;
		menuFish[i].faceLeft = menuFish[i].dx < 0;
		menuFish[i].dartTicks = 0;
	}
}

void updateMenuFish() {
	for (int i = 0; i < MENU_FISH; i++) {
		MenuFish &f = menuFish[i];

		// Increased darting speed significantly for rapid movement
		double speed = (f.dartTicks > 0) ? 8.0 : 1.0;

		f.x += f.dx * speed;
		f.y += f.dy * speed;
		if (f.dartTicks > 0) f.dartTicks--;

		// Loop around the screen edges.
		if (f.x < -60) f.x = SCREEN_W + 50;
		if (f.x > SCREEN_W + 60) f.x = -50;
		f.y = clampD(f.y, 50, SEA_Y - 50);
		if (f.y <= 50 || f.y >= SEA_Y - 50) f.dy = -f.dy;

		f.faceLeft = f.dx < 0;
	}
}

// Clicking a fish teleports it rapidly to a new place and makes it dart.
void clickMenuFish(double mx, double my) {
	double y = SCREEN_H - my;   // mouse y is measured from the top
	for (int i = 0; i < MENU_FISH; i++) {
		MenuFish &f = menuFish[i];
		if (!touching(mx, y, 4, f.x, f.y, f.size + 10)) continue;

		// Rapidly change place (Teleport to a new random location)
		f.x = randRange(100, SCREEN_W - 100);
		f.y = randRange(100, SEA_Y - 100);

		// Pick a new random angle to burst out from the new location
		double angle = randRange(0, 6.283);
		f.dx = cos(angle) * 2.0;
		f.dy = sin(angle) * 1.0;
		f.dartTicks = 25; // Brief but rapid speed boost

		playButton();
		return;
	}
}

void drawMenuFish() {
	for (int i = 0; i < MENU_FISH; i++) {
		MenuFish &f = menuFish[i];
		double s = f.size * 2.0;
		int sprite = f.faceLeft ? preyLeft[f.look] : preyRight[f.look];
		iShowImage((int)(f.x - s / 2), (int)(f.y - s / 2), (int)s, (int)s, sprite);
	}
}

// The menu's living ocean: same scenery as level 1, plus the fish.
void drawMenuBackdrop() {
	int saved = currentLevel;
	currentLevel = 1;          // always show the bright sunny ocean
	drawEnvironment();
	currentLevel = saved;
	drawMenuFish();

	// Dark veil so the buttons stay readable over the busy scene.
	glColor4f(0.0f, 0.02f, 0.06f, 0.45f);
	iFilledRectangle(0, 0, SCREEN_W, SCREEN_H);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

// ==== 3. LOADING ====
void loadMenuImages() {
	splashImage = loadImg("Images/splash.png", "Images/backgroundImage.png");
	helpImage = loadImg("Images/instruction.png");
	creditsImage = loadImg("Images/credit.png");
	winImage = loadImg("Images/win.png");
	loseImage = loadImg("Images/gameover.png");
	setupMenuFish();
}

// ==== 4. SHARED WIDGETS ====
// One button row, highlighted when selected. Used by the menu and map.
void drawButton(double cx, double y, double w, const char* label, bool selected, bool locked) {
	double x = cx - w / 2;
	if (locked)        drawPanel(x, y, w, 52, 40, 40, 48, 90, 90, 100);
	else if (selected) drawPanel(x, y, w, 52, 40, 150, 210, 200, 240, 255);
	else               drawPanel(x, y, w, 52, 18, 40, 62, 90, 130, 170);

	int r = locked ? 130 : 255, g = locked ? 130 : 255, b = locked ? 140 : 255;
	if (selected && !locked) { r = 10; g = 20; b = 30; }

	iSetColor(r, g, b);
	iText((int)(cx - strlen(label) * 4.5), (int)(y + 20), (char*)label, GAME_FONT);
}

void drawHint(const char* text) {
	drawText(SCREEN_W / 2.0 - strlen(text) * 4.5, 50, text, 190, 210, 230);
}

void drawTitle(const char* text) {
	drawText(SCREEN_W / 2.0 - strlen(text) * 7.0, SCREEN_H - 170, text, 255, 240, 170, BIG_FONT);
}

// ==== 5. SPLASH SCREEN ====
// A loading page shown once at startup, over a full-screen background.
void drawSplash() {
	iShowImage(0, 0, SCREEN_W, SCREEN_H, splashImage);

	double barW = 520, barH = 22;
	double x = SCREEN_W / 2.0 - barW / 2, y = 150;
	double pct = (double)splashTicks / SPLASH_LENGTH;

	drawPanel(x, y, barW, barH, 12, 18, 28, 190, 215, 235);
	iSetColor(70, 200, 230);
	iFilledRectangle(x + 2, y + 2, (barW - 4) * clampD(pct, 0, 1), barH - 4);

	char buf[48];
	sprintf_s(buf, "LOADING  %d%%", (int)(pct * 100));
	drawText(SCREEN_W / 2.0 - 52, y + barH + 22, buf, 235, 245, 255);
	drawText(SCREEN_W / 2.0 - 96, y - 34, "press ENTER to skip", 170, 190, 210);
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
	drawTitle("A Q U A F E A S T");

	for (int i = 0; i < MENU_COUNT; i++)
		drawButton(SCREEN_W / 2.0, SCREEN_H - 300 - i * 68, 340, menuLabels[i], i == menuIndex, false);

	char buf[64];
	sprintf_s(buf, "BEST SCORE: %d", bestScore());
	drawText(SCREEN_W / 2.0 - 70, 110, buf, 255, 225, 120);
	drawHint("UP / DOWN to move    ENTER to select    click a fish to scare it");
}

void openMenuChoice() {
	if (menuIndex == 0) { playerName[0] = '\0'; screen = SCR_NAME; }
	else if (menuIndex == 1) screen = SCR_HELP;
	else if (menuIndex == 2) screen = SCR_SCORES;
	else if (menuIndex == 3) screen = SCR_CREDITS;
	else exit(0);
}

void updateMenu() {
	if (tapped(isSpecialKeyPressed(GLUT_KEY_UP) != 0, kUp)) {
		menuIndex = (menuIndex - 1 + MENU_COUNT) % MENU_COUNT;
		playButton();
	}
	if (tapped(isSpecialKeyPressed(GLUT_KEY_DOWN) != 0, kDown)) {
		menuIndex = (menuIndex + 1) % MENU_COUNT;
		playButton();
	}
	if (tapped(isKeyPressed(KEY_ENTER) != 0, kEnter)) {
		playButton();
		openMenuChoice();
	}
}

// ==== 7. NICKNAME ENTRY ====
// Letters and digits are collected in iKeyboard() (see iMain.cpp).
void drawNameEntry() {
	drawMenuBackdrop();
	drawTitle("ENTER YOUR NICKNAME");

	double boxW = 520, x = SCREEN_W / 2.0 - boxW / 2, y = SCREEN_H / 2.0;
	drawPanel(x, y, boxW, 60, 12, 26, 40, 120, 190, 230);

	// A blinking cursor after whatever has been typed so far.
	char shown[32];
	sprintf_s(shown, "%s%s", playerName, ((splashTicks / 12) % 2 == 0) ? "_" : " ");
	drawText(x + 24, y + 22, shown, 255, 255, 255, BIG_FONT);

	drawText(SCREEN_W / 2.0 - 190, y - 60, "one word, letters and numbers only", 180, 200, 220);
	drawHint("ENTER to continue    BACKSPACE to erase / go back");
}

void updateNameEntry() {
	splashTicks++;   // reused purely to blink the cursor

	if (tapped(isKeyPressed(KEY_ENTER) != 0, kEnter)) {
		if (playerName[0] == '\0') return;   // a name is required
		playButton();
		screen = SCR_CHARACTER;
	}
}

// ==== 8. CHARACTER SELECT ====
int skinIndex = 0;
const char* skinNames[3] = { "REEF DARTER", "CORAL GLIDER", "DEEP RUNNER" };

void drawCharacterSelect() {
	drawMenuBackdrop();
	drawTitle("CHOOSE YOUR FISH");

	double cardW = 300, gap = 60;
	double totalW = cardW * 3 + gap * 2;
	double startX = SCREEN_W / 2.0 - totalW / 2;
	double y = SCREEN_H / 2.0 - 120;

	for (int i = 0; i < 3; i++) {
		double x = startX + i * (cardW + gap);
		bool sel = (i == skinIndex);

		if (sel) drawPanel(x, y, cardW, 300, 20, 60, 90, 120, 220, 255);
		else     drawPanel(x, y, cardW, 300, 12, 26, 40, 70, 100, 130);

		iShowImage((int)(x + cardW / 2 - 90), (int)(y + 110), 180, 130, skinRight[i]);
		drawText(x + cardW / 2 - strlen(skinNames[i]) * 4.5, y + 60,
			skinNames[i], sel ? 255 : 170, sel ? 235 : 190, sel ? 140 : 200);

		if (sel) drawText(x + cardW / 2 - 30, y + 26, "SELECTED", 120, 240, 160);
	}

	drawHint("LEFT / RIGHT to choose    ENTER to confirm    BACKSPACE to go back");
}

void updateCharacterSelect() {
	if (tapped(isSpecialKeyPressed(GLUT_KEY_LEFT) != 0, kLeft)) {
		skinIndex = (skinIndex + 2) % 3;
		playButton();
	}
	if (tapped(isSpecialKeyPressed(GLUT_KEY_RIGHT) != 0, kRight)) {
		skinIndex = (skinIndex + 1) % 3;
		playButton();
	}
	if (tapped(isKeyPressed(KEY_ENTER) != 0, kEnter)) {
		playerSkin = skinIndex;
		playButton();
		screen = SCR_MAP;
	}
}

// ==== 9. LEVEL MAP ====
// Three stops along a route. Only unlocked levels can be entered.
int mapIndex = 0;

void drawLevelMap() {
	drawMenuBackdrop();
	drawTitle("SELECT A LEVEL");

	const char* names[3] = { "1  -  SUNNY SHALLOWS", "2  -  MIDNIGHT DEEP", "3  -  STORM WATERS" };
	double y0 = SCREEN_H / 2.0 + 60;

	// A dotted route connecting the three stops.
	iSetColor(90, 140, 180);
	for (int i = 0; i < 3 - 1; i++)
	for (int d = 0; d < 10; d++)
		iFilledCircle(SCREEN_W / 2.0, y0 - i * 90 - d * 9 - 52, 2.5, 6);

	for (int i = 0; i < 3; i++) {
		bool locked = (i + 1) > unlockedLevel;
		char label[64];
		if (locked) sprintf_s(label, "%s   [LOCKED]", names[i]);
		else         sprintf_s(label, "%s", names[i]);
		drawButton(SCREEN_W / 2.0, y0 - i * 90, 520, label, i == mapIndex, locked);
	}

	char buf[64];
	sprintf_s(buf, "PLAYER: %s", playerName);
	drawText(SCREEN_W / 2.0 - 60, y0 + 110, buf, 170, 215, 245);
	drawHint("UP / DOWN to move    ENTER to play    BACKSPACE to go back");
}

void updateLevelMap() {
	if (tapped(isSpecialKeyPressed(GLUT_KEY_UP) != 0, kUp)) {
		mapIndex = (mapIndex + 2) % 3;
		playButton();
	}
	if (tapped(isSpecialKeyPressed(GLUT_KEY_DOWN) != 0, kDown)) {
		mapIndex = (mapIndex + 1) % 3;
		playButton();
	}
	if (tapped(isKeyPressed(KEY_ENTER) != 0, kEnter)) {
		if (mapIndex + 1 > unlockedLevel) return;   // locked - ignore
		playButton();
		stats.score = 0;
		stats.lives = 3;
		startLevel(mapIndex + 1);
	}
}

// ==== 10. SCORES / HELP / CREDITS ====
void drawScores() {
	drawMenuBackdrop();
	drawTitle("HIGH SCORES");

	double x = SCREEN_W / 2.0 - 260, y = SCREEN_H - 300;
	drawPanel(x, y - 380, 520, 400, 10, 24, 38, 90, 140, 180);

	if (scoreCount == 0) {
		drawText(x + 150, y - 190, "no scores yet - go play!", 190, 210, 230);
	}
	for (int i = 0; i < scoreCount; i++) {
		char row[80];
		sprintf_s(row, "%2d.  %-16s %6d", i + 1, scoreTable[i].name, scoreTable[i].score);
		int bright = (i == 0) ? 255 : 210;
		drawText(x + 40, y - 40 - i * 34, row, bright, bright, (i == 0) ? 120 : 230);
	}
	drawHint("BACKSPACE to go back");
}

void drawHelp() {
	iShowImage(0, 0, SCREEN_W, SCREEN_H, helpImage);
	drawHint("BACKSPACE to go back");
}

void drawCredits() {
	iShowImage(0, 0, SCREEN_W, SCREEN_H, creditsImage);
	drawHint("BACKSPACE to go back");
}

// ==== 11. END-OF-RUN SCREENS ====
void drawGameOver() {
	iShowImage(0, 0, SCREEN_W, SCREEN_H, loseImage);
	char buf[64];
	sprintf_s(buf, "FINAL SCORE: %d", stats.score);
	drawText(SCREEN_W / 2.0 - 80, SCREEN_H / 2.0 - 60, buf, 255, 235, 150, BIG_FONT);
	drawHint("BACKSPACE to return to the menu");
}

void drawLevelWon() {
	iShowImage(0, 0, SCREEN_W, SCREEN_H, winImage);
	char buf[64];
	sprintf_s(buf, "SCORE: %d", stats.score);
	drawText(SCREEN_W / 2.0 - 60, SCREEN_H / 2.0 - 40, buf, 255, 235, 150, BIG_FONT);

	if (currentLevel < MAX_LEVELS)
		drawText(SCREEN_W / 2.0 - 150, SCREEN_H / 2.0 - 90, "ENTER for the next level", 180, 240, 200);
	else
		drawText(SCREEN_W / 2.0 - 170, SCREEN_H / 2.0 - 90, "You finished every level!", 180, 240, 200);
	drawHint("BACKSPACE to return to the menu");
}

// ==== 12. RETURNING TO THE MENU ====
void returnToMenu() {
	screen = SCR_MENU;
	isGameOver = false;
	isLevelWon = false;
	menuIndex = 0;
	currentLevel = 1;   // menu always shows the bright level-1 ocean
	scrollX = 0;        // stop the menu scenery being offset by the last level
	stopGameMusic();
	startMenuMusic();
}

#endif