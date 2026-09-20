# AquaFeast - Guide

Short on purpose. Read this before your submission.

---

## 1. Files you still need to add

The game **runs without these** (each one falls back to an image you
already have), but adding them makes it look finished.

| File | What it's for | If missing |
|---|---|---|
| `Images/splash.png` | Full-screen intro art behind the loading bar | uses `backgroundImage.png` |
| `Images/Background/bird_02.png` | Bird wing frame 2 | bird glides, no flapping |
| `Images/Background/bird_03.png` | Bird wing frame 3 | bird glides, no flapping |
| `Images/Fish/prey_small_02_left.png` | 3rd prey fish facing left | uses the right-facing image |
| `Images/Predators/predator_02_up.png` | Level 2/3 predator, up | uses right-facing image |
| `Images/Predators/predator_02_down.png` | Level 2/3 predator, down | uses right-facing image |
| `Sound/swim.mp3` | Soft swish while swimming sideways | silent |

**Naming rule:** `name_direction.png` — always lowercase, underscores,
no spaces. A fish that faces 4 ways needs
`_right`, `_left`, `_up`, `_down`. Sounds are lowercase `.mp3` in `Sound/`.

---

## 2. Bugs that were found and fixed

- **`iLoadImage()` never returns 0 when a file is missing.** It calls
  `glGenTextures` either way, so you always get a valid (but blank)
  texture id. Every `if (sprite == 0)` fallback in the old code was dead
  code that never ran. `loadImg()` in `utility.hpp` now checks the file
  really exists first.
- **Two boat systems were running at once** — `Boat.hpp` and `boats.hpp`
  were both included and both tried to do the same job. Now one
  `boat.hpp`.
- **Level 2/3 enemy movement.** The old `applyOceanCurrents()` pushed
  fish sideways with no re-clamping, so they drifted out of the world and
  never returned. Level 2 also had its own separate movement code. Now
  **every level runs the exact same movement code** — `wander()` +
  `keepInWater()` + `keepInWorld()` in `entities.hpp`. One path means a
  fix anywhere fixes all 3 levels.
- **This iGraphics version has no `iKeyboard()` callback** (only `iDraw`,
  `fixedUpdate`, `iMouse`, `iMouseMove`, `iPassiveMouseMove`). Typing the
  nickname is done by polling `isKeyPressed()` per letter — see
  `readNameTyping()` in `iMain.cpp`. Remember this if you add more text
  input later.
- **Header guard clash.** `#define BOAT_H` collided with the constant
  `BOAT_H`. All guards are now `_HPP`.

---

## 3. New file layout

Files include each other in a straight line, never in a circle:

```
utility -> sound -> scores -> player -> entities -> boat
        -> environment -> hud -> levels -> menu -> iMain.cpp
```

| File | Holds |
|---|---|
| `utility.hpp` | screen size, game state, maths helpers, safe image loading |
| `sound.hpp` | every sound and music track |
| `scores.hpp` | the player score database (`scores.txt`) |
| `player.hpp` | your fish: moving, growing, jumping, bubbles, respawn |
| `entities.hpp` | prey fish and predators |
| `boat.hpp` | boat, nets (L2), hook tug-of-war (L3), power-ups, chest |
| `environment.hpp` | sky, water bands, sand, seaweed, sun/moon/rain |
| `hud.hpp` | the top status bar |
| `levels.hpp` | the 3 levels' numbers + per-tick update/draw |
| `menu.hpp` | every screen that is not gameplay |
| `iMain.cpp` | loads everything, then routes draw/update per screen |

`.hpp` files are all `#include`d into `iMain.cpp` only. Never include one
from a second `.cpp` or you will get "already defined" linker errors.

---

## 4. How the screens flow

```
SPLASH -> MENU -> START -> NAME -> CHARACTER -> MAP -> gameplay
              -> INSTRUCTIONS / SCORES / CREDITS / EXIT
```

BACKSPACE always goes back one step. On the name screen it deletes a
letter first, then goes back when the box is empty.

---

## 5. Scores

Saved to `scores.txt` next to the `.exe`, one line per player:
`NAME SCORE`. Each player keeps only their **best** score, top 10 shown.
The score is saved once per run whether you win, run out of time, or
lose your last life (`saveRunScore()` in `levels.hpp`).

---

## 6. Difficulty (tune these in `levels.hpp`)

|  | L1 sunny | L2 night | L3 rain |
|---|---|---|---|
| Grow to | 46 | 62 | 80 |
| Time | 120s | 100s | 85s |
| Prey | 30 | 26 | 22 |
| Prey speed | 1.5 | 2.1 | 2.7 |
| Predators | 3 | 5 | 7 |
| Predator speed | 2.1 | 2.9 | 3.6 |
| Hazard | none | nets | nets + hook |

Later levels give you **less food that moves faster** while asking you to
grow bigger in less time — that is where the extra difficulty comes from.

---

## 7. The level 3 hook

Get hooked and a two-colour bar appears beside your fish:

- **red (falling from the top)** = the fisherman reeling you in
- **green (rising from the bottom)** = your escape effort

Mash SPACE to push green up. Stop tapping and it slips back down. Green
full = you escape. Red full = you lose one life. Constants to tune are at
the top of `boat.hpp`: `PULL_PER_TICK`, `PUSH_PER_TAP`, `STRUGGLE_DECAY`.

Nets and hooks cost **one life, never an instant game over**. After any
life loss the fish keeps its size, drops in from the top of the screen,
and carries on from the middle of the ocean with ~2 seconds of blinking
safety.

---

## 8. Files that were deleted

Junk and duplicates: the four `ChatGPT Image ....png`, `k.png`, `l.png`,
the whole duplicate `Images/Fish/fish_character*` set, the old
`Images/Player/` sprites (replaced by the 3 selectable characters), the
level-2 bats and old underwater jpg (replaced by the shared moon/stars
sky), the duplicate `Images/Boat/` folder, and the mute/restart HUD icons
(the HUD is now only level, life, time, score, progress).

Assets went from ~90 files to 45 — and all 45 are actually used. They are
still in your GitHub history if you want any of them back.

---

## 9. Beginner tips

- Every number you might want to tune sits near the **top** of its file
  under a `CONSTANTS` heading. Change those, not the logic.
- **Never call `iLoadImage()` while drawing.** Loading from disk every
  frame makes the game stutter and makes keys feel unresponsive. All
  loading happens once in `loadEverything()`.
- If you add a feature that needs its own on/off state, put the variable
  next to the others in that file's `STATE` section — keep the pattern.
- `updateLevel()` and `drawLevel()` in `levels.hpp` are the two functions
  that run every tick during play. Anything that should happen
  "constantly while playing" goes through one of them.
