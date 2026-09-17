#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
// ==== 1. INCLUDES ====
#include "utility.hpp"
#include "player.hpp"

// ==== 2. CONSTANTS ====
#define MAX_CLOUDS   3
#define MAX_BIRDS    2
#define MAX_BUBBLES 15
#define MAX_WEEDS    20   // seaweed plants along the ocean floor

const double ICON_SIZE = 26.0;
const double RESTART_ICON_X = SCREEN_WIDTH - 110, RESTART_ICON_Y = SCREEN_HEIGHT - 46;
const double MUTE_ICON_X = SCREEN_WIDTH - 60, MUTE_ICON_Y = SCREEN_HEIGHT - 46;
const double SAND_HEIGHT = 30.0; // sandy strip along the very bottom

// ==== 3. STRUCTS ====
struct DriftingSprite { double x, y, speed; int spriteId; };
struct Bubble { double x, y, speed; }; // x is a WORLD position - see updateBubbles()
struct Seaweed { double x; int spriteIndex; }; // sits on the sand, so it only needs an x
enum HudClick { CLICK_NONE, CLICK_RESTART, CLICK_MUTE };

// ==== 4. GLOBAL ARRAYS / STATE ====
DriftingSprite clouds[MAX_CLOUDS];
DriftingSprite birds[MAX_BIRDS];
Bubble bubbles[MAX_BUBBLES];
Seaweed weeds[MAX_WEEDS];
int cloudSprite, birdSprite, bubbleSprite;
int surfaceSprite = 0; // LEVEL 2 BOTTOM SEABED SURFACE TEXTURE
// LEVEL 2 NIGHT MODE: 4-frame Bat animation sprites for Level 2 night sky
int batSprites[4] = { 0, 0, 0, 0 };
// LEVEL 2 DYNAMIC UNDERWATER BACKGROUND & WATER DEPTH SYSTEM
int level2UnderwaterBg = 0;

#define MAX_DEPTH_SPORES 24
struct DepthSpore {
	double x, y;
	double speedX, speedY;
	double size;
	double pulsePhase;
	double baseAlpha;
};
DepthSpore depthSpores[MAX_DEPTH_SPORES];
bool depthSporesInitialized = false;
double depthAnimTimer = 0.0;

void initDepthSpores() {
	for (int i = 0; i < MAX_DEPTH_SPORES; i++) {
		depthSpores[i].x = (double)(rand() % SCREEN_WIDTH);
		depthSpores[i].y = 20.0 + (double)(rand() % (int)(WATER_SURFACE_Y - 40));
		depthSpores[i].speedX = 0.2 + (rand() % 40) / 100.0;
		depthSpores[i].speedY = 0.1 + (rand() % 30) / 100.0;
		depthSpores[i].size = 1.5 + (rand() % 25) / 10.0;
		depthSpores[i].pulsePhase = (rand() % 628) / 100.0;
		depthSpores[i].baseAlpha = 0.35 + (rand() % 45) / 100.0;
	}
	depthSporesInitialized = true;
}

int weedSprite[3];             // up to 3 look variants - see loadEnvironment()
int restartIconSprite, muteIconSprite;
double worldMinX = 0, worldMaxX = 3000;

// ==== 5. FUNCTIONS ====
void loadEnvironment() {
	cloudSprite = iLoadImage("Images/Background/cloud_01.png");
	birdSprite = iLoadImage("Images/Background/bird_01.png");
	surfaceSprite = iLoadImage("Images/Background/surface.png");

	// LEVEL 2 NIGHT MODE: Load 4 bat animation frames for Level 2 night environment
	batSprites[0] = iLoadImage("Images/Background/bat1.png");
	batSprites[1] = iLoadImage("Images/Background/bat2.png");
	batSprites[2] = iLoadImage("Images/Background/bat3.png");
	batSprites[3] = iLoadImage("Images/Background/bat4.png");

	// LEVEL 2 DYNAMIC UNDERWATER BACKGROUND IMAGE
	level2UnderwaterBg = iLoadImage("Images/Background/level2_underwater_bg.jpg");
	bubbleSprite = iLoadImage("Images/Background/bubble_01.png");
	restartIconSprite = iLoadImage("Images/HUD/restart_icon.png");
	muteIconSprite = iLoadImage("Images/HUD/mute_icon.png");

	// Seaweed comes in up to 3 looks, picked randomly per plant below.
	weedSprite[0] = iLoadImage("Images/Background/seaweed_01.png");
	weedSprite[1] = iLoadImage("Images/Background/seaweed_02.png");
	weedSprite[2] = iLoadImage("Images/Background/seaweed_03.png");

	for (int i = 0; i < MAX_CLOUDS; i++)
		clouds[i] = { i * 400.0, WATER_SURFACE_Y + 40 + i * 20.0, 0.3 + i * 0.1, cloudSprite };
	for (int i = 0; i < MAX_BIRDS; i++)
		birds[i] = { i * 600.0 + 200, WATER_SURFACE_Y + 90 + i * 15.0, -(0.8 + i * 0.2), birdSprite };
	for (int i = 0; i < MAX_BUBBLES; i++)
		bubbles[i] = { (double)(rand() % SCREEN_WIDTH), (double)(rand() % SCREEN_HEIGHT), 0.5 + (rand() % 100) / 100.0 };
	for (int i = 0; i < MAX_WEEDS; i++)
		weeds[i] = { i * 260.0 + (rand() % 120), rand() % 3 };
}

void resetEnvironment(double worldWidth) {
	worldMinX = 0;
	worldMaxX = worldWidth;
}

void followPlayer() {
	scrollX = player.x;
	if (scrollX < worldMinX) scrollX = worldMinX;
	if (scrollX > worldMaxX) scrollX = worldMaxX;
}

void drawOceanBands(double surfaceY) {
	double bandHeight = surfaceY / 3.0;

	// LEVEL 2 NIGHT MODE: Dark blue / night underwater atmosphere
	if (currentLevel == 2) {
		iSetColor(2, 8, 24);                           // deep abyssal midnight blue (bottom band)
		iFilledRectangle(0, 0, SCREEN_WIDTH, bandHeight);
		iSetColor(5, 18, 46);                          // mid dark ocean band
		iFilledRectangle(0, bandHeight, SCREEN_WIDTH, bandHeight);
		iSetColor(10, 32, 72);                         // shallow moonlit night surface band
		iFilledRectangle(0, bandHeight * 2.0, SCREEN_WIDTH, surfaceY - bandHeight * 2.0);
	}
	else {
		// LEVEL 1 & 3: Untouched original daytime ocean bands
		iSetColor(8, 40, 75);                          // deep (bottom band)
		iFilledRectangle(0, 0, SCREEN_WIDTH, bandHeight);
		iSetColor(14, 65, 105);                        // mid band
		iFilledRectangle(0, bandHeight, SCREEN_WIDTH, bandHeight);
		iSetColor(20, 90, 140);                        // shallow (top band)
		iFilledRectangle(0, bandHeight * 2.0, SCREEN_WIDTH, surfaceY - bandHeight * 2.0);
	}
}

void drawSand() {
	if (currentLevel == 2) {
		if (surfaceSprite != 0) {
			double bgWidth = 400.0;
			double tileHeight = SAND_HEIGHT + 20.0;
			double parallaxOffset = scrollX;
			double startX = -fmod(parallaxOffset, bgWidth);
			if (startX > 0) startX -= bgWidth;

			for (double px = startX; px < SCREEN_WIDTH + bgWidth; px += bgWidth) {
				iShowImage((int)px, 16, (int)bgWidth, (int)tileHeight, surfaceSprite);
			}
		}
		else {
			glColor4f(0.01f, 0.02f, 0.05f, 0.35f);
			iFilledRectangle(0, 0, SCREEN_WIDTH, SAND_HEIGHT);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
	else {
		// LEVEL 1 & 3: Untouched original daytime sand
		iSetColor(210, 190, 140);
		iFilledRectangle(0, 0, SCREEN_WIDTH, SAND_HEIGHT);
	}
}

void drawSeaweed() {
	for (int i = 0; i < MAX_WEEDS; i++) {
		double screenX = worldToScreenX(weeds[i].x);
		if (screenX < -40 || screenX > SCREEN_WIDTH + 40) continue;
		iShowImage((int)screenX - 20, 25, 60, 60, weedSprite[weeds[i].spriteIndex]);
	}
}

void drawLevel2DynamicBackground(double surfaceY) {
	if (level2UnderwaterBg == 0) {
		drawOceanBands(surfaceY);
		return;
	}

	double bgWidth = SCREEN_WIDTH;
	double parallaxOffset = scrollX * 0.35;
	double startX = -fmod(parallaxOffset, bgWidth);
	if (startX > 0) startX -= bgWidth;

	for (double px = startX; px < SCREEN_WIDTH + bgWidth; px += bgWidth) {
		iShowImage((int)px, 0, (int)bgWidth, (int)surfaceY, level2UnderwaterBg);
	}

	double playerDepthRatio = (WATER_SURFACE_Y - player.y) / WATER_SURFACE_Y;
	if (playerDepthRatio < 0.0) playerDepthRatio = 0.0;
	if (playerDepthRatio > 1.0) playerDepthRatio = 1.0;

	double pressurePulse = 0.04 * sin(depthAnimTimer * 0.9);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_QUADS);
	glColor4f(0.04f, 0.12f, 0.24f, (float)(0.06 + playerDepthRatio * 0.10 + pressurePulse * 0.5));
	glVertex2f(0, surfaceY);
	glVertex2f(SCREEN_WIDTH, surfaceY);

	glColor4f(0.005f, 0.01f, 0.03f, (float)(0.40 + playerDepthRatio * 0.30 + pressurePulse));
	glVertex2f(SCREEN_WIDTH, 0);
	glVertex2f(0, 0);
	glEnd();

	for (int r = 0; r < 5; r++) {
		double rayBaseX = 140.0 + r * 250.0;
		double sway = sin(depthAnimTimer * 1.1 + r * 1.4) * 35.0;
		double rayTopX1 = rayBaseX + sway * 0.3;
		double rayTopX2 = rayTopX1 + 90.0 + r * 15.0;
		double rayBotX1 = rayBaseX + sway * 1.5 - 70.0;
		double rayBotX2 = rayBotX1 + 180.0 + r * 25.0;

		float rayAlphaTop = (float)((0.14 - playerDepthRatio * 0.06) + 0.04 * sin(depthAnimTimer * 1.6 + r * 0.8));
		if (rayAlphaTop < 0.02f) rayAlphaTop = 0.02f;

		glBegin(GL_QUADS);
		glColor4f(0.45f, 0.75f, 1.0f, rayAlphaTop);
		glVertex2f(rayTopX1, surfaceY);
		glVertex2f(rayTopX2, surfaceY);

		glColor4f(0.15f, 0.40f, 0.70f, 0.0f);
		glVertex2f(rayBotX2, surfaceY * 0.18);
		glVertex2f(rayBotX1, surfaceY * 0.18);
		glEnd();
	}

	glLineWidth(2.0f);
	glBegin(GL_LINE_STRIP);
	for (int x = 0; x <= SCREEN_WIDTH; x += 16) {
		double waveY = surfaceY + sin(x * 0.012 + depthAnimTimer * 2.2) * 2.8
			+ cos(x * 0.024 - depthAnimTimer * 1.4) * 1.5;
		glColor4f(0.55f, 0.85f, 1.0f, 0.45f);
		glVertex2f(x, waveY);
	}
	glEnd();
	glLineWidth(1.0f);

	for (int i = 0; i < MAX_DEPTH_SPORES; i++) {
		DepthSpore &sp = depthSpores[i];
		if (sp.y > surfaceY) continue;

		double localDepth = (surfaceY - sp.y) / surfaceY;
		double glowPulse = sin(depthAnimTimer * 2.0 + sp.pulsePhase);
		float sporeAlpha = (float)(sp.baseAlpha * (0.35 + 0.65 * localDepth) * (0.7 + 0.3 * glowPulse));

		glColor4f(0.30f, 0.95f, 0.85f, sporeAlpha);
		iFilledCircle(sp.x, sp.y, sp.size, 10);

		glColor4f(0.20f, 0.70f, 0.90f, sporeAlpha * 0.35f);
		iFilledCircle(sp.x, sp.y, sp.size * 2.2, 12);
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void drawLevel2Moonlight(double surfaceY) {
	glColor4f(0.35f, 0.65f, 0.95f, 0.06f);
	glBegin(GL_POLYGON);
	glVertex2f(860, surfaceY);
	glVertex2f(980, surfaceY);
	glVertex2f(1080, 0);
	glVertex2f(740, 0);
	glEnd();

	glColor4f(0.30f, 0.60f, 0.90f, 0.05f);
	glBegin(GL_POLYGON);
	glVertex2f(520, surfaceY);
	glVertex2f(620, surfaceY);
	glVertex2f(680, 0);
	glVertex2f(420, 0);
	glEnd();

	glColor4f(0.25f, 0.55f, 0.85f, 0.04f);
	glBegin(GL_POLYGON);
	glVertex2f(180, surfaceY);
	glVertex2f(280, surfaceY);
	glVertex2f(320, 0);
	glVertex2f(100, 0);
	glEnd();

	glColor4f(0.45f, 0.75f, 1.0f, 0.30f);
	glBegin(GL_POLYGON);
	glVertex2f(0, surfaceY - 2);
	glVertex2f(SCREEN_WIDTH, surfaceY - 2);
	glVertex2f(SCREEN_WIDTH, surfaceY + 2);
	glVertex2f(0, surfaceY + 2);
	glEnd();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void drawSky() {
	double surfaceY = worldToScreenY(WATER_SURFACE_Y);

	// LEVEL 2 NIGHT MODE: Dark night sky, moon, stars, and bats flying near water surface
	if (currentLevel == 2) {
		// Dark midnight night sky
		iSetColor(6, 12, 28);
		iFilledRectangle(0, surfaceY, SCREEN_WIDTH, SCREEN_HEIGHT - surfaceY + 100);

		// Twinkling stars across the night sky
		static const double starPoints[][2] = {
			{ 70, 640 }, { 150, 680 }, { 230, 630 }, { 310, 670 },
			{ 410, 650 }, { 490, 685 }, { 570, 625 }, { 660, 675 },
			{ 730, 640 }, { 820, 690 }, { 900, 635 }, { 1090, 660 },
			{ 1160, 630 }, { 1220, 675 }, { 360, 615 }, { 850, 665 }
		};
		for (int s = 0; s < 16; s++) {
			iSetColor(210, 230, 255);
			iFilledCircle(starPoints[s][0], starPoints[s][1], (s % 3 == 0) ? 2.0 : 1.2, 8);
		}

		// Glowing Moon in the night sky
		double moonX = 1000.0;
		double moonY = surfaceY + 85.0;
		glColor4f(0.4f, 0.65f, 1.0f, 0.12f);
		iFilledCircle(moonX, moonY, 36.0, 32);
		glColor4f(0.7f, 0.85f, 1.0f, 0.22f);
		iFilledCircle(moonX, moonY, 26.0, 32);
		iSetColor(240, 245, 255);
		iFilledCircle(moonX, moonY, 18.0, 32);
		iSetColor(205, 220, 240);
		iFilledCircle(moonX + 3.0, moonY + 2.0, 15.0, 32);
		iSetColor(240, 245, 255);
		iFilledCircle(moonX + 1.0, moonY + 1.0, 14.0, 32);

		// Night-tinted drifting clouds
		glColor4f(0.35f, 0.40f, 0.55f, 0.55f);
		for (int i = 0; i < MAX_CLOUDS; i++)
			iShowImage((int)clouds[i].x, (int)worldToScreenY(clouds[i].y), 90, 90, clouds[i].spriteId);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		// LEVEL 2 NIGHT MODE: 4-Frame Bat animation flying right-to-left
		for (int i = 0; i < MAX_BIRDS; i++) {
			double batY = WATER_SURFACE_Y + 16.0 + i * 20.0;

			// Calculate frame index changing every 5 pixels of movement
			int frameIndex = (abs((int)(birds[i].x / 5.0))) % 4;
			int currentBatSprite = batSprites[frameIndex];

			if (currentBatSprite != 0) {
				iShowImage((int)birds[i].x, (int)worldToScreenY(batY), 50, 32, currentBatSprite);
			}
		}
	}
	else {
		// LEVEL 1 & 3: Untouched original daytime sky, clouds, and birds
		iSetColor(135, 206, 235);
		iFilledRectangle(0, surfaceY, SCREEN_WIDTH, SCREEN_HEIGHT - surfaceY + 100);

		for (int i = 0; i < MAX_CLOUDS; i++)
			iShowImage((int)clouds[i].x, (int)worldToScreenY(clouds[i].y), 90, 90, clouds[i].spriteId);
		for (int i = 0; i < MAX_BIRDS; i++)
			iShowImage((int)birds[i].x, (int)worldToScreenY(birds[i].y), 45, 45, birds[i].spriteId);
	}
}

void drawEnvironment() {
	double surfaceY = worldToScreenY(WATER_SURFACE_Y);
	if (currentLevel == 2) {
		drawLevel2DynamicBackground(surfaceY);
	}
	else {
		drawOceanBands(surfaceY);
	}
	drawSeaweed();
	//drawSand();
	drawSky();

	if (currentLevel == 2) {
		drawLevel2Moonlight(surfaceY);
	}
}

void updateEnvironmentAnimation() {
	for (int i = 0; i < MAX_CLOUDS; i++) {
		clouds[i].x += clouds[i].speed;
		if (clouds[i].x > SCREEN_WIDTH) clouds[i].x = -80;
	}
	for (int i = 0; i < MAX_BIRDS; i++) {
		birds[i].x += birds[i].speed;
		if (birds[i].x < -40) birds[i].x = SCREEN_WIDTH;
	}

	if (currentLevel == 2) {
		depthAnimTimer += 0.025;

		if (!depthSporesInitialized) {
			initDepthSpores();
		}

		for (int i = 0; i < MAX_DEPTH_SPORES; i++) {
			depthSpores[i].x += depthSpores[i].speedX;
			depthSpores[i].y += sin(depthAnimTimer + i) * 0.35;
			if (depthSpores[i].x > SCREEN_WIDTH + 20) {
				depthSpores[i].x = -20;
				depthSpores[i].y = 20.0 + (rand() % (int)(WATER_SURFACE_Y - 40));
			}
		}
	}
}

void updateBubbles() {
	double surfaceScreenY = worldToScreenY(WATER_SURFACE_Y);
	for (int i = 0; i < MAX_BUBBLES; i++) {
		bubbles[i].y += bubbles[i].speed;
		if (bubbles[i].y > surfaceScreenY) {
			bubbles[i].y = 0;
			bubbles[i].x = scrollX + (rand() % SCREEN_WIDTH) - SCREEN_CENTER_X;
		}
	}
}

void drawBubbles() {
	glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
	for (int i = 0; i < MAX_BUBBLES; i++)
		iShowImage((int)worldToScreenX(bubbles[i].x) - 8, (int)bubbles[i].y - 8, 16, 16, bubbleSprite);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void drawHud() {
	if (currentLevel == 2) {
		iSetColor(240, 245, 255);
	}
	else {
		iSetColor(0, 0, 0);
	}
	char textBuffer[64];
	sprintf_s(textBuffer, "Score: %d", stats.score);
	iText(20, SCREEN_HEIGHT - 20, textBuffer, GAME_FONT);
	sprintf_s(textBuffer, "Lives: %d", stats.lives);
	iText(280, SCREEN_HEIGHT - 20, textBuffer, GAME_FONT);
	sprintf_s(textBuffer, "Time Left: %ds", stats.timeRemaining);
	iText(480, SCREEN_HEIGHT - 20, textBuffer, GAME_FONT);
	sprintf_s(textBuffer, "Level: %d", stats.level);
	iText(750, SCREEN_HEIGHT - 20, textBuffer, GAME_FONT);

	if (currentLevel == 2) {
		double depthMeters = (WATER_SURFACE_Y - player.y) * 0.18;
		if (depthMeters < 0.0) depthMeters = 0.0;

		const char* zoneName = "SURFACE";
		if (depthMeters >= 70.0)      zoneName = "ABYSSAL TRENCH";
		else if (depthMeters >= 45.0) zoneName = "MIDNIGHT ZONE";
		else if (depthMeters >= 20.0) zoneName = "TWILIGHT ZONE";

		double badgeX = 20, badgeY = SCREEN_HEIGHT - 44;
		iSetColor(8, 18, 36);
		iFilledRectangle(badgeX, badgeY, 230, 18);
		iSetColor(30, 80, 150);
		iRectangle(badgeX, badgeY, 230, 18);

		double maxMeters = WATER_SURFACE_Y * 0.18;
		double depthFrac = depthMeters / maxMeters;
		if (depthFrac > 1.0) depthFrac = 1.0;
		iSetColor(24, 140, 200);
		iFilledRectangle(badgeX + 2, badgeY + 2, (int)(226 * depthFrac), 14);

		char depthBuf[64];
		sprintf_s(depthBuf, "DEPTH: %.1fm [%s]", depthMeters, zoneName);
		iSetColor(230, 245, 255);
		iText(badgeX + 8, badgeY + 5, depthBuf, GLUT_BITMAP_8_BY_13);
	}

	double barX = 880, barY = SCREEN_HEIGHT - 30, barWidth = 200, barHeight = 16;
	iSetColor(60, 60, 60);
	iFilledRectangle(barX, barY, barWidth, barHeight);
	iSetColor(80, 200, 255);
	iFilledRectangle(barX, barY, barWidth * stats.progress, barHeight);
	iSetColor(255, 255, 255);
	iRectangle(barX, barY, barWidth, barHeight);

	iShowImage((int)RESTART_ICON_X, (int)RESTART_ICON_Y, (int)ICON_SIZE, (int)ICON_SIZE, restartIconSprite);
	iShowImage((int)MUTE_ICON_X, (int)MUTE_ICON_Y, (int)ICON_SIZE, (int)ICON_SIZE, muteIconSprite);
	if (isMuted) {
		iSetColor(220, 40, 40);
		iLine(MUTE_ICON_X, MUTE_ICON_Y, MUTE_ICON_X + ICON_SIZE, MUTE_ICON_Y + ICON_SIZE);
	}
}

HudClick checkHudClick(double mouseX, double mouseY) {
	if (pointInRect(mouseX, mouseY, RESTART_ICON_X, RESTART_ICON_Y, ICON_SIZE, ICON_SIZE)) return CLICK_RESTART;
	if (pointInRect(mouseX, mouseY, MUTE_ICON_X, MUTE_ICON_Y, ICON_SIZE, ICON_SIZE)) return CLICK_MUTE;
	return CLICK_NONE;
}

#endif