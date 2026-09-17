#ifndef ENTITIES_H
#define ENTITIES_H

#include "utility.hpp"
#include "player.hpp"
#include "menu.hpp" // for playEatSound()

#define MAX_PREY 40
#define MAX_PREDATORS 10
#define PREY_LOOK_COUNT 3

const double PREDATOR_CHASE_RADIUS = 150.0;
const int PREDATOR_WARNING_TICKS = 20;

// LEVEL 2 SCHOOL OF FISH AI
#define MAX_SCHOOLS 10
const double SCHOOL_DETECT_RADIUS = 180.0;
const int SCHOOL_PANIC_TICKS = 120;

struct FishSchool
{
	int schoolID;
	double centerX, centerY;
	bool panicking;
	int panicTicks;
	int memberCount;
};

FishSchool schools[MAX_SCHOOLS];
int schoolCount = 0;

struct PreyFish
{
	double x, y;
	double size;
	double speed;

	// Movement direction
	double dirX, dirY;

	// Number of ticks before changing direction
	int ticksUntilTurn;

	int look;

	// Current movement direction
	FacingDirection facing;

	bool alive;

	// Level 2 School AI fields
	int schoolID;
	bool inSchool;
	bool panicking;
	int panicTicks;
};

struct Predator
{
	double x, y;
	double size;
	double speed;

	// Used while wandering
	double dirX, dirY;
	int ticksUntilTurn;

	// Predator type
	int spriteSet;

	// Current facing direction
	FacingDirection facing;

	bool alive;
	bool isChasing;
	int warningTicksLeft;
};

PreyFish preyList[MAX_PREY];
int preyCount = 0;

Predator predators[MAX_PREDATORS];
int predatorCount = 0;

int preySprite[PREY_LOOK_COUNT];
int preyLeftSprite[PREY_LOOK_COUNT];

// Level 2 specific prey arrays
int level2PreyRight[3];
int level2PreyLeft[3];

int warningIconSprite;

// Predator sprites
int predatorSprites[2][4];

void loadEntities()
{
	// Level 1 / Default Prey
	preySprite[0] = iLoadImage("Images/Fish/prey_small_01.png");
	preySprite[1] = iLoadImage("Images/Fish/prey_medium_01.png");
	preySprite[2] = iLoadImage("Images/Fish/prey_small_02.png");

	preyLeftSprite[0] = iLoadImage("Images/Fish/prey_small_01_left.png");
	preyLeftSprite[1] = iLoadImage("Images/Fish/prey_medium_01_left.png");
	preyLeftSprite[2] = iLoadImage("Images/Fish/prey_small_02.png");

	// Level 2 Prey
	level2PreyRight[0] = iLoadImage("Images/Fish/prey_small_01.png");
	level2PreyLeft[0] = iLoadImage("Images/Fish/prey_small_01_left.png");
	level2PreyRight[1] = iLoadImage("Images/Fish/prey_medium_01.png");
	level2PreyLeft[1] = iLoadImage("Images/Fish/prey_medium_01_left.png");
	level2PreyRight[2] = iLoadImage("Images/Fish/fish_character03_right.png");
	level2PreyLeft[2] = iLoadImage("Images/Fish/fish_character03_left.png");

	warningIconSprite = iLoadImage("Images/HUD/warning_icon.png");

	// Predator 01
	predatorSprites[0][FACE_RIGHT] = iLoadImage("Images/Predators/predator_01_right.png");
	predatorSprites[0][FACE_LEFT] = iLoadImage("Images/Predators/predator_01_left.png");
	predatorSprites[0][FACE_UP] = iLoadImage("Images/Predators/predator_01_up.png");
	predatorSprites[0][FACE_DOWN] = iLoadImage("Images/Predators/predator_01_down.png");

	// Predator 02
	predatorSprites[1][FACE_RIGHT] = iLoadImage("Images/Predators/predator_02_right.png");
	predatorSprites[1][FACE_LEFT] = iLoadImage("Images/Predators/predator_02_left.png");
	predatorSprites[1][FACE_UP] = predatorSprites[1][FACE_RIGHT];
	predatorSprites[1][FACE_DOWN] = predatorSprites[1][FACE_RIGHT];
}

void addPreyFish(
	double x,
	double y,
	double size,
	int schoolID = -1,
	bool inSchool = false)
{
	if (preyCount >= MAX_PREY)
		return;

	PreyFish f;

	f.x = x;
	f.y = y;
	f.size = size;
	f.speed = 1.5;
	f.look = rand() % PREY_LOOK_COUNT;

	pickNewDirection(f.dirX, f.dirY, f.ticksUntilTurn);
	f.facing = facingFromDelta(f.dirX, f.dirY);
	f.alive = true;

	f.schoolID = schoolID;
	f.inSchool = inSchool;
	f.panicking = false;
	f.panicTicks = 0;

	preyList[preyCount++] = f;
}

void addPredator(
	double x,
	double y,
	double speed,
	int spriteSet)
{
	if (predatorCount >= MAX_PREDATORS)
		return;

	Predator p;

	p.x = x;
	p.y = y;
	p.size = 30.0;
	p.speed = speed;
	p.spriteSet = spriteSet;

	pickNewDirection(p.dirX, p.dirY, p.ticksUntilTurn);
	p.facing = facingFromDelta(p.dirX, p.dirY);
	p.alive = true;
	p.isChasing = false;
	p.warningTicksLeft = 0;

	predators[predatorCount++] = p;
}

void spawnEntitiesForLevel(
	int level,
	double worldWidth)
{
	preyCount = 0;
	predatorCount = 0;
	schoolCount = 0;

	int preyToSpawn = 20 + level * 8;

	if (level == 2)
	{
		int numSchools = 4;
		int fishPerSchool = 5;

		for (int s = 0; s < numSchools; s++)
		{
			if (schoolCount >= MAX_SCHOOLS)
				break;

			FishSchool school;
			school.schoolID = s;
			school.centerX = 300.0 + s * 500.0;
			school.centerY = 100.0 + (s * 70) % (int)(WATER_SURFACE_Y - 140.0);
			school.panicking = false;
			school.panicTicks = 0;
			school.memberCount = fishPerSchool;

			schools[schoolCount++] = school;

			for (int f = 0; f < fishPerSchool; f++)
			{
				double offsetX = (rand() % 60) - 30;
				double offsetY = (rand() % 60) - 30;

				double x = school.centerX + offsetX;
				double y = school.centerY + offsetY;

				addPreyFish(x, y, 12.0, s, true);
			}
		}

		int remainingPrey = preyToSpawn - (numSchools * fishPerSchool);
		for (int i = 0; i < remainingPrey; i++)
		{
			double x = 150.0 + (rand() % (int)(worldWidth - 300.0));
			double y = 40.0 + (rand() % (int)(WATER_SURFACE_Y - 80.0));

			addPreyFish(x, y, 12.0, -1, false);
		}
	}
	else
	{
		for (int i = 0; i < preyToSpawn; i++)
		{
			double x = 150.0 + (rand() % (int)(worldWidth - 300.0));
			double y = 40.0 + (rand() % (int)(WATER_SURFACE_Y - 80.0));

			addPreyFish(x, y, 12.0, -1, false);
		}
	}

	int predatorsToSpawn = level + 2;

	for (int i = 0; i < predatorsToSpawn; i++)
	{
		double x = 600.0 + i * 400.0;
		double y = 40.0 + (i * 60) % (int)(WATER_SURFACE_Y - 80.0);
		double speed = 2.0 + (level - 1) * 0.6;
		int spriteSet = (level >= 2) ? 1 : 0;

		addPredator(x, y, speed, spriteSet);
	}
}

void keepInWater(
	double &y,
	double &dirY,
	double size)
{
	if (y > WATER_SURFACE_Y - size)
	{
		dirY = -fabs(dirY);
	}

	if (y < WORLD_FLOOR_Y + size)
	{
		dirY = fabs(dirY);
	}
}

void updateSchoolCenters()
{
	for (int s = 0; s < schoolCount; s++)
	{
		double sumX = 0.0;
		double sumY = 0.0;
		int count = 0;

		for (int i = 0; i < preyCount; i++)
		{
			if (preyList[i].alive && preyList[i].inSchool && preyList[i].schoolID == schools[s].schoolID)
			{
				sumX += preyList[i].x;
				sumY += preyList[i].y;
				count++;
			}
		}

		schools[s].memberCount = count;
		if (count > 0)
		{
			schools[s].centerX = sumX / count;
			schools[s].centerY = sumY / count;
		}
	}
}

void updateSchoolPanic()
{
	for (int s = 0; s < schoolCount; s++)
	{
		if (schools[s].memberCount <= 0)
			continue;

		FishSchool &school = schools[s];

		bool playerNear = false;
		for (int i = 0; i < preyCount; i++)
		{
			PreyFish &f = preyList[i];
			if (f.alive && f.inSchool && f.schoolID == school.schoolID)
			{
				double dist = distanceBetween(f.x, f.y, player.x, player.y);
				if (dist < SCHOOL_DETECT_RADIUS)
				{
					playerNear = true;
					break;
				}
			}
		}

		if (playerNear && !school.panicking)
		{
			school.panicking = true;
			school.panicTicks = SCHOOL_PANIC_TICKS;

			for (int i = 0; i < preyCount; i++)
			{
				PreyFish &f = preyList[i];
				if (f.alive && f.inSchool && f.schoolID == school.schoolID)
				{
					f.panicking = true;
					f.panicTicks = SCHOOL_PANIC_TICKS;

					double dx = f.x - player.x;
					double dy = f.y - player.y;
					double len = sqrt(dx * dx + dy * dy);

					if (len > 0.001)
					{
						dx /= len;
						dy /= len;
					}
					else
					{
						dx = 1.0;
						dy = 0.0;
					}

					double jitterAngle = ((rand() % 60) - 30) * 3.14159265 / 180.0;
					double cosA = cos(jitterAngle);
					double sinA = sin(jitterAngle);

					f.dirX = dx * cosA - dy * sinA;
					f.dirY = dx * sinA + dy * cosA;
				}
			}
		}

		if (school.panicking)
		{
			school.panicTicks--;
			if (school.panicTicks <= 0)
			{
				school.panicking = false;
				school.panicTicks = 0;

				for (int i = 0; i < preyCount; i++)
				{
					PreyFish &f = preyList[i];
					if (f.alive && f.inSchool && f.schoolID == school.schoolID)
					{
						f.panicking = false;
						f.panicTicks = 0;
					}
				}
			}
		}
	}
}

void updatePrey()
{
	if ((currentLevel == 2 || stats.level == 2) && schoolCount > 0)
	{
		updateSchoolCenters();
		updateSchoolPanic();
	}

	for (int i = 0; i < preyCount; i++)
	{
		if (!preyList[i].alive)
			continue;

		PreyFish &f = preyList[i];

		if ((currentLevel == 2 || stats.level == 2) && f.inSchool && f.schoolID >= 0)
		{
			int sIdx = -1;
			for (int s = 0; s < schoolCount; s++)
			{
				if (schools[s].schoolID == f.schoolID)
				{
					sIdx = s;
					break;
				}
			}

			if (f.panicking)
			{
				// High Difficulty: Increased panic speed (3.2x base speed)
				double panicSpeed = f.speed * 3.2;

				f.x += f.dirX * panicSpeed;
				f.y += f.dirY * panicSpeed;

				f.panicTicks--;
				if (f.panicTicks <= 0)
				{
					f.panicking = false;
				}
			}
			else
			{
				if (sIdx >= 0 && schools[sIdx].memberCount > 0)
				{
					double distToCenter = distanceBetween(f.x, f.y, schools[sIdx].centerX, schools[sIdx].centerY);

					if (distToCenter > 40.0)
					{
						double cDx = schools[sIdx].centerX - f.x;
						double cDy = schools[sIdx].centerY - f.y;
						double cLen = sqrt(cDx * cDx + cDy * cDy);

						if (cLen > 0.001)
						{
							cDx /= cLen;
							cDy /= cLen;

							f.dirX = f.dirX * 0.85 + cDx * 0.15;
							f.dirY = f.dirY * 0.85 + cDy * 0.15;

							double dLen = sqrt(f.dirX * f.dirX + f.dirY * f.dirY);
							if (dLen > 0.001)
							{
								f.dirX /= dLen;
								f.dirY /= dLen;
							}
						}
					}
				}

				f.x += f.dirX * f.speed;
				f.y += f.dirY * f.speed;

				f.ticksUntilTurn--;
				if (f.ticksUntilTurn <= 0)
				{
					pickNewDirection(f.dirX, f.dirY, f.ticksUntilTurn);
				}
			}
		}
		else
		{
			f.x += f.dirX * f.speed;
			f.y += f.dirY * f.speed;

			f.ticksUntilTurn--;

			if (f.ticksUntilTurn <= 0)
			{
				pickNewDirection(f.dirX, f.dirY, f.ticksUntilTurn);
			}
		}

		keepInWater(f.y, f.dirY, f.size);
		f.facing = facingFromDelta(f.dirX, f.dirY);
	}
}

void wanderPredator(Predator &p)
{
	p.x += p.dirX * p.speed * 0.5;
	p.y += p.dirY * p.speed * 0.5;

	p.ticksUntilTurn--;

	if (p.ticksUntilTurn <= 0)
	{
		pickNewDirection(p.dirX, p.dirY, p.ticksUntilTurn);
	}

	keepInWater(p.y, p.dirY, p.size);
	p.facing = facingFromDelta(p.dirX, p.dirY);
}

void chasePlayer(Predator &p)
{
	double dx = player.x - p.x;
	double dy = player.y - p.y;

	double length = sqrt(dx * dx + dy * dy);

	if (length > 0.001)
	{
		dx /= length;
		dy /= length;
	}

	p.x += dx * p.speed;
	p.y += dy * p.speed;

	p.facing = facingFromDelta(dx, dy);
}

void updateChaseState(Predator &p)
{
	double dist = distanceBetween(p.x, p.y, player.x, player.y);
	bool inRange = dist < PREDATOR_CHASE_RADIUS;

	if (inRange && !p.isChasing)
	{
		if (p.warningTicksLeft == 0)
		{
			p.warningTicksLeft = PREDATOR_WARNING_TICKS;
		}

		p.warningTicksLeft--;

		if (p.warningTicksLeft <= 0)
		{
			p.isChasing = true;
		}
	}
	else if (!inRange)
	{
		p.isChasing = false;
		p.warningTicksLeft = 0;
	}
}

void updatePredators()
{
	for (int i = 0; i < predatorCount; i++)
	{
		if (!predators[i].alive)
			continue;

		updateChaseState(predators[i]);

		if (predators[i].isChasing)
		{
			chasePlayer(predators[i]);
		}
		else
		{
			wanderPredator(predators[i]);
		}
	}
}

void handlePreyEaten()
{
	for (int i = 0; i < preyCount; i++)
	{
		if (!preyList[i].alive)
			continue;

		bool touching = circlesTouch(
			player.x, player.y, player.size,
			preyList[i].x, preyList[i].y, preyList[i].size);

		if (touching && player.size > preyList[i].size)
		{
			preyList[i].alive = false;
			growPlayer(GROWTH_PER_FISH);
			stats.score += (int)(preyList[i].size * 2);
			playEatSound();
		}
	}
}

void handlePredatorCollisions()
{
	if (playerInvulnerableTicks > 0)
	{
		playerInvulnerableTicks--;
		return;
	}

	for (int i = 0; i < predatorCount; i++)
	{
		if (!predators[i].alive)
			continue;

		bool touching = circlesTouch(
			player.x, player.y, player.size,
			predators[i].x, predators[i].y, predators[i].size);

		if (!touching)
			continue;

		if (player.size > predators[i].size)
		{
			predators[i].alive = false;
			growPlayer(GROWTH_PER_FISH * 2.0);
			stats.score += (int)(predators[i].size * 3);
			playEatSound();
		}
		else
		{
			stats.lives -= 1;
			playerInvulnerableTicks = 60;

			if (stats.lives <= 0)
			{
				triggerGameOver();
			}
		}
	}
}

void updateEntities()
{
	updatePrey();
	updatePredators();
	handlePreyEaten();
	handlePredatorCollisions();
}

void drawEntities()
{
	for (int i = 0; i < preyCount; i++)
	{
		if (!preyList[i].alive)
			continue;

		PreyFish &f = preyList[i];
		double s = f.size * 2.0;

		int screenX = (int)(worldToScreenX(f.x) - s / 2);
		int screenY = (int)(worldToScreenY(f.y) - s / 2);

		int sprite;

		if (currentLevel == 2 || stats.level == 2)
		{
			if (f.facing == FACE_LEFT)
			{
				sprite = level2PreyLeft[f.look];
			}
			else
			{
				sprite = level2PreyRight[f.look];
			}
		}
		else
		{
			if (f.facing == FACE_LEFT)
			{
				sprite = preyLeftSprite[f.look];
			}
			else
			{
				sprite = preySprite[f.look];
			}
		}

		iShowImage(screenX, screenY, (int)s, (int)s, sprite);
	}

	for (int i = 0; i < predatorCount; i++)
	{
		if (!predators[i].alive)
			continue;

		double s = predators[i].size * 2.0;

		int screenX = (int)(worldToScreenX(predators[i].x) - s / 2);
		int screenY = (int)(worldToScreenY(predators[i].y) - s / 2);

		int sprite = predatorSprites[predators[i].spriteSet][predators[i].facing];

		iShowImage(screenX, screenY, (int)s, (int)s, sprite);

		if (predators[i].warningTicksLeft > 0)
		{
			int iconX = (int)(worldToScreenX(predators[i].x) - 12);
			int iconY = (int)(worldToScreenY(predators[i].y) + s / 2 + 6);

			iShowImage(iconX, iconY, 24, 24, warningIconSprite);
		}
	}
}

#endif