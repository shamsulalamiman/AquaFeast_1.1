# AquaFeast

## Game Description

**AquaFeast** is a 2D **grow-by-eating arcade game** built with **C++** and the **iGraphics** library. The player controls a fish, eats smaller fish to grow, avoids larger predators, and progresses through three increasingly difficult levels.

Level 3 introduces a special **hook mini-game**, where the player must tap `SPACE` to escape.

## Features

🎮 Three levels with different underwater environments: **Sunny, Night, and Storm**.

🐟 Grow by eating smaller fish while avoiding larger predators.

🪝 Special **hook escape mini-game** in Level 3.

💰 Collectibles, power-ups, and a treasure chest.

📊 HUD for **level, lives, time, score, and progress**.

🎵 Background music and gameplay sound effects.

🖥️ Main menu, character selection, level map, scores, help, credits, and end-of-run screens.

## Project Details

**IDE:** Visual Studio

**Language:** C++

**Graphics Library:** iGraphics

**Platform:** Windows PC (Win32/x86)

**Genre:** 2D Arcade Survival

## Technical Architecture

The project is organized into modular header files, with each system having a clear responsibility:

- **utility.hpp**: Screen states, global helpers, image loading, input helpers, and common drawing utilities.
- **sound.hpp**: Background music and sound effects.
- **scores.hpp**: Score file handling, top-10 score table, and score submission.
- **player.hpp**: Player fish movement, growth, jumping, bubbles, hook shake, and respawn behavior.
- **entities.hpp**: Prey fish, predators, and level danger enemies.
- **boat.hpp**: Boat, nets, hook mini-game, power-ups, and treasure chest.
- **environment.hpp**: Sky, sun/moon, rain, water depth, sand, seaweed, and world scrolling.
- **hud.hpp**: Level, lives, time, score, progress, mute, and restart controls.
- **levels.hpp**: Level settings, updates, drawing, and danger-enemy events.
- **menu.hpp**: Splash screen, main menu, name entry, character selection, map, scores, help, credits, and result screens.
- **iMain.cpp**: Main game loop and routing for drawing, updates, and mouse input.

## How to Run the Project

Make sure you have the following installed:

- Visual Studio
- iGraphics library and the project dependencies included in this repository
- Windows PC

### Steps

1. Clone or download the repository.
2. Open **`test.sln`** in Visual Studio.
3. Make sure the project is configured for **Win32/x86**.
4. Build the solution using **Build → Build Solution**.
5. Run the game using **Debug → Start Without Debugging**.

## How to Play

### **Controls**

| Action | Key / Input |
|---|---|
| Move Left | `A` / `←` |
| Move Right | `D` / `→` |
| Move Up | `W` / `↑` |
| Move Down | `S` / `↓` |
| Jump | `SPACE` |
| Hook Escape (Level 3) | Tap `SPACE` |
| Menu Navigation | `W/S` or `↑/↓` |
| Select | `ENTER` |
| Back | `BACKSPACE` |
| Mute | Mouse click |
| Restart | Mouse click |

## Game Rules

- Eat smaller fish to grow.
- Avoid larger predators and dangerous enemies.
- Collect available rewards and power-ups.
- Survive each level to progress further.
- In Level 3, escape the hook challenge by tapping `SPACE`.
- Keep track of lives, time, score, and progress through the HUD.

## Project Contributors

1. MD. SHAMSUL ALAM AIMAN
2. PUSHPENDU ROY RAJ
3. TIRTHA BARUA

## Screenshots

### **Main Menu**
<img src="screenshots/menu.png" width="700">

### **Level 1 — Sunny Ocean**
<img src="screenshots/level1.png" width="700">

### **Level 2 — Night Ocean**
<img src="screenshots/level2.png" width="700">

### **Level 3 — Storm Ocean**
<img src="screenshots/level3.png" width="700">

## Youtube Link

[AquaFeast Gameplay Recording](https://dub.sh/AquaFeast_Gameplay)

## Project Report

[AquaFeast Gameplay Report](https://dub.sh/AquaFeast_ProjectFolder)
