#ifndef MENU_H
#define MENU_H

// ============================================================
// 1. INCLUDES
// ============================================================

#include <windows.h>
#include <mmsystem.h>
#include "utility.hpp"
#include "player.hpp"


// ============================================================
// 2. CONSTANTS
// ============================================================

#define HOME_ITEM_COUNT   4
#define LEVEL_ITEM_COUNT  4


// ============================================================
// 3. GLOBAL STATE
// ============================================================

extern int bgImage;

int instructionImage;
int creditImage;

// 3 distinct character selection images
int menuCharacterSprites[3];

const char* homeMenuLabels[HOME_ITEM_COUNT] =
{
	"Start Game",
	"Instructions",
	"Credits",
	"Exit"
};

const char* levelMenuLabels[LEVEL_ITEM_COUNT] =
{
	"Level 1",
	"Level 2",
	"Level 3",
	"Back"
};

int selectedHomeIndex = 0;
int selectedLevelIndex = 0;


// Keyboard state
bool wasUp = false;
bool wasDown = false;
bool wasEnter = false;
bool wasBack = false;
bool wasLeft = false;
bool wasRight = false;


// ============================================================
// 4. MENU IMAGES
// ============================================================

void loadMenuImages()
{
	instructionImage =
		iLoadImage("Images/instruction.png");

	creditImage =
		iLoadImage("Images/credit.png");

	// Load 3 distinct character images side-by-side for character selection
	menuCharacterSprites[0] = iLoadImage("Images/Character/fish_character_01_right.png");
	if (menuCharacterSprites[0] == 0) menuCharacterSprites[0] = iLoadImage("Images/Character/fish_character_01.png");

	menuCharacterSprites[1] = iLoadImage("Images/Character/fish_character_02_right.png");
	if (menuCharacterSprites[1] == 0) menuCharacterSprites[1] = iLoadImage("Images/Character/fish_character_02.png");

	menuCharacterSprites[2] = iLoadImage("Images/Character/fish_character_03_right.png");
	if (menuCharacterSprites[2] == 0) menuCharacterSprites[2] = iLoadImage("Images/Character/fish_character_03.png");

	// Sync with player characters so chosen fish is ready for gameplay
	playerCharacterSprites[0] = menuCharacterSprites[0];
	playerCharacterSprites[1] = menuCharacterSprites[1];
	playerCharacterSprites[2] = menuCharacterSprites[2];
}


// ============================================================
// 5. SOUND
// ============================================================

void loadSounds()
{
	mciSendString(
		"open \"Sound/button.mp3\" type mpegvideo alias btnSound",
		NULL, 0, NULL);

	mciSendString(
		"open \"Sound/eat.mp3\" type mpegvideo alias eatSound",
		NULL, 0, NULL);

	mciSendString(
		"open \"Sound/collect.mp3\" type mpegvideo alias collectSound",
		NULL, 0, NULL);

	mciSendString(
		"open \"Sound/win.mp3\" type mpegvideo alias winSound",
		NULL, 0, NULL);

	mciSendString(
		"open \"Sound/lose.mp3\" type mpegvideo alias loseSound",
		NULL, 0, NULL);

	mciSendString(
		"open \"Sound/bgMusic.mp3\" type mpegvideo alias bgm",
		NULL, 0, NULL);

	// Menu music
	mciSendString(
		"open \"Sound/gamestart.mp3\" type mpegvideo alias menuMusic",
		NULL, 0, NULL);
}


// ============================================================
// PLAY SOUND
// ============================================================

void playSound(const char* alias)
{
	if (isMuted)
		return;

	char cmd[64];

	sprintf_s(
		cmd,
		"play %s from 0",
		alias);

	mciSendString(
		cmd,
		NULL,
		0,
		NULL);
}


void playButtonSound()
{
	playSound("btnSound");
}

void playEatSound()
{
	playSound("eatSound");
}

void playCollectSound()
{
	playSound("collectSound");
}

void playWinSound()
{
	playSound("winSound");
}

void playLoseSound()
{
	playSound("loseSound");
}


// ============================================================
// MUSIC
// ============================================================

void startGameplayMusic()
{
	if (!isMuted)
	{
		mciSendString(
			"play bgm from 0 repeat",
			NULL,
			0,
			NULL);
	}
}


void stopGameplayMusic()
{
	mciSendString(
		"stop bgm",
		NULL,
		0,
		NULL);
}


void startMenuMusic()
{
	if (!isMuted)
	{
		mciSendString(
			"play menuMusic from 0 repeat",
			NULL,
			0,
			NULL);
	}
}


void stopMenuMusic()
{
	mciSendString(
		"stop menuMusic",
		NULL,
		0,
		NULL);
}


// ============================================================
// MUTE
// ============================================================

void toggleMute()
{
	isMuted = !isMuted;

	if (isMuted)
	{
		stopGameplayMusic();
		stopMenuMusic();
	}
	else if (isPlaying)
	{
		startGameplayMusic();
	}
	else if (isInMenu)
	{
		startMenuMusic();
	}
}


// ============================================================
// GAME OVER
// ============================================================

void triggerGameOver()
{
	if (isGameOver)
		return;

	isGameOver = true;

	playLoseSound();

	stopGameplayMusic();
}


// ============================================================
// HOME MENU INITIALIZATION
// ============================================================

void homeMenuInitialize()
{
	stats = GameStats();

	currentLevel = 1;

	currentMenuScreen = MENU_HOME;

	gameState = MENU;

	selectedHomeIndex = 0;

	selectedLevelIndex = 0;

	// No fish selected initially
	selectedCharacter = -1;

	startMenuMusic();
}


// ============================================================
// RETURN TO HOME MENU
// ============================================================

void returnToHomeMenu()
{
	isInMenu = true;

	isPlaying = false;

	isGameOver = false;

	isLevelComplete = false;

	currentMenuScreen = MENU_HOME;

	gameState = MENU;

	selectedHomeIndex = 0;

	selectedLevelIndex = 0;

	selectedCharacter = -1;

	stopGameplayMusic();

	startMenuMusic();
}


// ============================================================
// START LEVEL
// ============================================================

void requestStartLevel(int levelNumber)
{
	currentLevel = levelNumber;

	isInMenu = false;

	isPlaying = true;

	gameState = PLAYING;

	requestLevelStart = true;

	stopMenuMusic();

	startGameplayMusic();
}


// ============================================================
// 6. HOME MENU INPUT
// ============================================================

void handleHomeInput(
	bool up,
	bool down,
	bool enter)
{
	if (up)
	{
		selectedHomeIndex =
			(selectedHomeIndex - 1 + HOME_ITEM_COUNT)
			% HOME_ITEM_COUNT;
	}

	if (down)
	{
		selectedHomeIndex =
			(selectedHomeIndex + 1)
			% HOME_ITEM_COUNT;
	}

	if (!enter)
		return;


	// START GAME
	if (selectedHomeIndex == 0)
	{
		selectedCharacter = -1;

		currentMenuScreen =
			MENU_CHARACTER_SELECT;

		gameState =
			CHARACTER_SELECT;
	}


	// INSTRUCTIONS
	else if (selectedHomeIndex == 1)
	{
		currentMenuScreen =
			MENU_INSTRUCTIONS;
	}


	// CREDITS
	else if (selectedHomeIndex == 2)
	{
		currentMenuScreen =
			MENU_CREDITS;
	}


	// EXIT
	else if (selectedHomeIndex == 3)
	{
		exit(0);
	}
}


// ============================================================
// 7. LEVEL SELECTION INPUT
// ============================================================

void handleLevelSelectInput(
	bool up,
	bool down,
	bool enter,
	bool back)
{
	if (up)
	{
		selectedLevelIndex =
			(selectedLevelIndex - 1 + LEVEL_ITEM_COUNT)
			% LEVEL_ITEM_COUNT;
	}

	if (down)
	{
		selectedLevelIndex =
			(selectedLevelIndex + 1)
			% LEVEL_ITEM_COUNT;
	}


	// ENTER
	if (enter)
	{
		// BACK option
		if (selectedLevelIndex == 3)
		{
			currentMenuScreen =
				MENU_CHARACTER_SELECT;

			gameState =
				CHARACTER_SELECT;
		}

		// LEVEL 1, 2 or 3
		else
		{
			requestStartLevel(
				selectedLevelIndex + 1);
		}
	}


	// BACKSPACE
	if (back)
	{
		currentMenuScreen =
			MENU_CHARACTER_SELECT;

		gameState =
			CHARACTER_SELECT;
	}
}


// ============================================================
// 8. CHARACTER SELECTION INPUT
// ============================================================

void handleCharacterSelectInput(
	bool left,
	bool right,
	bool enter,
	bool back)
{
	// LEFT
	if (left)
	{
		if (selectedCharacter <= 0)
		{
			selectedCharacter = 2;
		}
		else
		{
			selectedCharacter--;
		}
	}


	// RIGHT
	if (right)
	{
		if (selectedCharacter == -1 ||
			selectedCharacter >= 2)
		{
			selectedCharacter = 0;
		}
		else
		{
			selectedCharacter++;
		}
	}


	// ENTER
	// IMPORTANT:
	// DO NOT START LEVEL 1 HERE.
	// Go to LEVEL SELECTION instead.
	if (enter && selectedCharacter != -1)
	{
		selectedLevelIndex = 0;

		currentMenuScreen =
			MENU_LEVEL_SELECT;

		gameState =
			MENU;
	}


	// BACKSPACE
	if (back)
	{
		currentMenuScreen =
			MENU_HOME;

		gameState =
			MENU;
	}
}


// ============================================================
// 9. CHARACTER SELECTION MOUSE
// ============================================================

void handleCharacterSelectMouse(
	int mx,
	int my)
{
	if (currentMenuScreen != MENU_CHARACTER_SELECT &&
		gameState != CHARACTER_SELECT)
	{
		return;
	}


	// ========================================================
	// FISH 1
	// ========================================================

	if (mx >= 280 &&
		mx <= 460 &&
		my >= 280 &&
		my <= 480)
	{
		selectedCharacter = 0;

		playButtonSound();
	}


	// ========================================================
	// FISH 2
	// ========================================================

	else if (mx >= 550 &&
		mx <= 730 &&
		my >= 280 &&
		my <= 480)
	{
		selectedCharacter = 1;

		playButtonSound();
	}


	// ========================================================
	// FISH 3
	// ========================================================

	else if (mx >= 820 &&
		mx <= 1000 &&
		my >= 280 &&
		my <= 480)
	{
		selectedCharacter = 2;

		playButtonSound();
	}


	// ========================================================
	// CONTINUE
	// ========================================================

	else if (mx >= 515 &&
		mx <= 765 &&
		my >= 180 &&
		my <= 230)
	{
		if (selectedCharacter != -1)
		{
			playButtonSound();

			// Go to level selection
			selectedLevelIndex = 0;

			currentMenuScreen =
				MENU_LEVEL_SELECT;

			gameState =
				MENU;
		}
	}


	// ========================================================
	// BACK
	// ========================================================

	else if (mx >= 540 &&
		mx <= 740 &&
		my >= 105 &&
		my <= 150)
	{
		playButtonSound();

		currentMenuScreen =
			MENU_HOME;

		gameState =
			MENU;
	}
}


// ============================================================
// 10. MENU NAVIGATION
// ============================================================

void updateMenuNavigation()
{
	if (!isInMenu)
		return;


	bool up =
		wasJustPressed(
		isSpecialKeyPressed(GLUT_KEY_UP),
		wasUp);

	bool down =
		wasJustPressed(
		isSpecialKeyPressed(GLUT_KEY_DOWN),
		wasDown);

	bool left =
		wasJustPressed(
		isSpecialKeyPressed(GLUT_KEY_LEFT),
		wasLeft);

	bool right =
		wasJustPressed(
		isSpecialKeyPressed(GLUT_KEY_RIGHT),
		wasRight);

	bool enter =
		wasJustPressed(
		isKeyPressed(KEY_ENTER),
		wasEnter);

	bool back =
		wasJustPressed(
		isKeyPressed(KEY_BACKSPACE),
		wasBack);


	if (up ||
		down ||
		left ||
		right ||
		enter ||
		back)
	{
		playButtonSound();
	}


	// HOME
	if (currentMenuScreen == MENU_HOME)
	{
		handleHomeInput(
			up,
			down,
			enter);
	}


	// LEVEL SELECT
	else if (currentMenuScreen == MENU_LEVEL_SELECT)
	{
		handleLevelSelectInput(
			up,
			down,
			enter,
			back);
	}


	// CHARACTER SELECT
	else if (currentMenuScreen == MENU_CHARACTER_SELECT)
	{
		handleCharacterSelectInput(
			left,
			right,
			enter,
			back);
	}


	// INSTRUCTIONS / CREDITS
	else if (back)
	{
		currentMenuScreen =
			MENU_HOME;

		gameState =
			MENU;
	}
}


// ============================================================
// 11. MENU BACKGROUND
// ============================================================

void drawMenuBackdrop()
{
	iShowImage(
		0,
		0,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		bgImage);
}


// ============================================================
// 12. DRAW SELECTABLE LIST
// ============================================================

void drawSelectableList(
	const char* items[],
	int itemCount,
	int selectedIndex)
{
	int startY = 420;

	int spacing = 55;


	for (int i = 0;
		i < itemCount;
		i++)
	{
		int y =
			startY -
			i * spacing;


		if (i == selectedIndex)
		{
			iSetColor(
				80,
				200,
				255);

			iFilledRectangle(
				SCREEN_WIDTH / 2 - 130,
				y - 12,
				260,
				38);

			iSetColor(
				10,
				10,
				20);
		}
		else
		{
			iSetColor(
				255,
				255,
				255);
		}


		iText(
			SCREEN_WIDTH / 2 - 100,
			y,
			(char*)items[i],
			GAME_FONT);
	}
}


// ============================================================
// 13. HOME MENU DRAW
// ============================================================

void drawHomeMenu()
{
	drawMenuBackdrop();

	drawSelectableList(
		homeMenuLabels,
		HOME_ITEM_COUNT,
		selectedHomeIndex);
}


// ============================================================
// 14. LEVEL SELECT MENU DRAW
// ============================================================

void drawLevelSelectMenu()
{
	drawMenuBackdrop();


	drawPixelTitle(
		SCREEN_WIDTH / 2 - 38,
		500,
		"LEVELS",
		255,
		255,
		255);


	drawSelectableList(
		levelMenuLabels,
		LEVEL_ITEM_COUNT,
		selectedLevelIndex);


	iSetColor(
		200,
		200,
		200);


	iText(
		SCREEN_WIDTH / 2 - 200,
		60,
		"UP/DOWN to move   ENTER to select   BACKSPACE to go back",
		GAME_FONT);
}


// ============================================================
// 15. INSTRUCTIONS
// ============================================================

void drawInstructionsMenu()
{
	iShowImage(
		0,
		0,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		instructionImage);


	iSetColor(
		200,
		200,
		200);


	iText(
		SCREEN_WIDTH / 2 - 100,
		40,
		"BACKSPACE to go back",
		GAME_FONT);
}


// ============================================================
// 16. CREDITS
// ============================================================

void drawCreditsMenu()
{
	iShowImage(
		0,
		0,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		creditImage);


	iSetColor(
		200,
		200,
		200);


	iText(
		SCREEN_WIDTH / 2 - 100,
		40,
		"BACKSPACE to go back",
		GAME_FONT);
}


// ============================================================
// 17. CHARACTER SELECT SCREEN
// ============================================================

void drawCharacterSelect()
{
	drawMenuBackdrop();


	// ========================================================
	// TITLE
	// ========================================================

	drawPixelTitle(
		SCREEN_WIDTH / 2 - 70,
		520,
		"CHOOSE YOUR FISH",
		255,
		255,
		255);


	// ========================================================
	// FISH CARDS
	// ========================================================

	int cardX[3] =
	{
		280,
		550,
		820
	};

	int cardY = 280;

	int cardW = 180;

	int cardH = 200;


	const char* fishNames[3] =
	{
		"Character 01",
		"Character 02",
		"Character 03"
	};


	for (int i = 0;
		i < 3;
		i++)
	{
		int x =
			cardX[i];


		bool isSelected =
			(selectedCharacter == i);


		// ====================================================
		// SELECTED CARD
		// ====================================================

		if (isSelected)
		{
			iSetColor(
				20,
				60,
				100);

			iFilledRectangle(
				x,
				cardY,
				cardW,
				cardH);


			iSetColor(
				255,
				215,
				0);

			iRectangle(
				x,
				cardY,
				cardW,
				cardH);

			iRectangle(
				x - 2,
				cardY - 2,
				cardW + 4,
				cardH + 4);

			iRectangle(
				x - 4,
				cardY - 4,
				cardW + 8,
				cardH + 8);


			iSetColor(
				255,
				215,
				0);

			iText(
				x + 50,
				cardY + cardH + 15,
				"SELECTED",
				GAME_FONT);
		}


		// ====================================================
		// NORMAL CARD
		// ====================================================

		else
		{
			iSetColor(
				15,
				30,
				55);

			iFilledRectangle(
				x,
				cardY,
				cardW,
				cardH);


			iSetColor(
				100,
				150,
				200);

			iRectangle(
				x,
				cardY,
				cardW,
				cardH);
		}


		// ====================================================
		// FISH IMAGE (Side-by-Side 3 Distinct Characters)
		// ====================================================

		iShowImage(
			x + 30,
			cardY + 60,
			120,
			110,
			menuCharacterSprites[i]);


		// ====================================================
		// FISH NAME
		// ====================================================

		if (isSelected)
		{
			iSetColor(
				255,
				215,
				0);
		}
		else
		{
			iSetColor(
				255,
				255,
				255);
		}


		iText(
			x + 36,
			cardY + 20,
			(char*)fishNames[i],
			GAME_FONT);
	}


	// ========================================================
	// CONTINUE BUTTON
	// ========================================================

	int btnContX = 515;
	int btnContY = 180;
	int btnContW = 250;
	int btnContH = 50;


	if (selectedCharacter != -1)
	{
		iSetColor(
			40,
			180,
			100);

		iFilledRectangle(
			btnContX,
			btnContY,
			btnContW,
			btnContH);


		iSetColor(
			255,
			255,
			255);

		iRectangle(
			btnContX,
			btnContY,
			btnContW,
			btnContH);


		iText(
			btnContX + 75,
			btnContY + 18,
			"CONTINUE",
			GAME_FONT);
	}
	else
	{
		iSetColor(
			60,
			70,
			80);

		iFilledRectangle(
			btnContX,
			btnContY,
			btnContW,
			btnContH);


		iSetColor(
			100,
			110,
			120);

		iRectangle(
			btnContX,
			btnContY,
			btnContW,
			btnContH);


		iSetColor(
			170,
			170,
			170);

		iText(
			btnContX + 30,
			btnContY + 18,
			"SELECT A FISH FIRST",
			GAME_FONT);
	}


	// ========================================================
	// BACK BUTTON
	// ========================================================

	int btnBackX = 540;
	int btnBackY = 105;
	int btnBackW = 200;
	int btnBackH = 45;


	iSetColor(
		180,
		50,
		50);

	iFilledRectangle(
		btnBackX,
		btnBackY,
		btnBackW,
		btnBackH);


	iSetColor(
		255,
		255,
		255);

	iRectangle(
		btnBackX,
		btnBackY,
		btnBackW,
		btnBackH);


	iText(
		btnBackX + 80,
		btnBackY + 15,
		"BACK",
		GAME_FONT);


	// ========================================================
	// HELP TEXT
	// ========================================================

	iSetColor(
		200,
		200,
		200);


	iText(
		SCREEN_WIDTH / 2 - 210,
		45,
		"Click a fish to select   Click CONTINUE to choose level",
		GAME_FONT);
}


// ============================================================
// 18. DRAW CURRENT MENU
// ============================================================

void drawCurrentMenu()
{
	if (currentMenuScreen == MENU_HOME)
	{
		drawHomeMenu();
	}

	else if (currentMenuScreen == MENU_LEVEL_SELECT)
	{
		drawLevelSelectMenu();
	}

	else if (currentMenuScreen == MENU_INSTRUCTIONS)
	{
		drawInstructionsMenu();
	}

	else if (currentMenuScreen == MENU_CREDITS)
	{
		drawCreditsMenu();
	}

	else if (currentMenuScreen == MENU_CHARACTER_SELECT)
	{
		drawCharacterSelect();
	}
}


#endif