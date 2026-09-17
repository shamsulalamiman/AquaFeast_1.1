# AquaFeast - Guide

Short on purpose - just what changed and what you need to do next.

## What changed this round

- **"Camera" is gone.** It's now just `scrollX` + `followPlayer()` in
  `utility.hpp`/`environment.hpp` - one number that shifts everything
  sideways as you swim. Height never scrolls, so there's nothing to
  track vertically at all. `worldToScreenX/Y` do the actual shifting.
- **Long functions got split up.** e.g. `updateEntities()` is now 4 short
  functions (`updatePrey`, `updatePredators`, `handlePreyEaten`,
  `handlePredatorCollisions`) instead of one long one. Same idea in
  `boats.hpp` and `menu.hpp`.
- **Predators wander now**, using the same random-turn movement as prey
  (shared as `pickNewDirection()` in `utility.hpp`), until you get close
  enough to trigger the chase.
- **Bubbles now shift sideways as you swim** - their `x` is a world
  position instead of a fixed screen spot (see `updateBubbles()` /
  `drawBubbles()` in `environment.hpp`).
- **3 prey fish looks**, picked randomly per fish (see `preySprite[3]`
  in `entities.hpp`).
- **Ocean is now 3 colored depth bands** (`drawOceanBands()`), plus a sand
  strip and seaweed along the floor (`drawSand()` / `drawSeaweed()`).
- **Instructions/Credits/Game Over/Win screens no longer reload their
  image every single frame.** This was the real bug behind BACKSPACE
  feeling broken on those screens - reloading a picture from disk 30+
  times a second was slow enough to make input lag badly. They're loaded
  once now (`loadMenuImages()` in `menu.hpp`, plus 2 lines in
  `iMain.cpp`), so BACKSPACE should feel instant everywhere now.
- **Menu music added** using your existing `gamestart.mp3` (was sitting
  unused) - plays on the home/instructions/credits/level-select screens,
  swaps to `bgMusic.mp3` the instant a level starts.

## About the "eat/win/loss sound not working" report

Checked the code path for all 5 sounds - it's correct and consistent for
all of them. But your `Sound/` folder is currently missing 3 files that
the code tries to open:

- `Sound/win.mp3` - missing
- `Sound/lose.mp3` - missing
- `Sound/collect.mp3` - missing

`eat.mp3` and `button.mp3` DO exist, and their code path is identical to
the others, so they should already work. If `eat.mp3` still doesn't play
even after you re-test, the most common real-world cause is the specific
mp3 encoding - some mp3 files fail to open through Windows' `mpegvideo`
MCI device depending on how they were exported, while otherwise-identical
files work fine. If that happens, try re-exporting it as a plain 44.1kHz
mp3 from any audio tool.

## Images/sounds to add

| File | Used for |
|---|---|
| `Images/Fish/prey_small_02.png` | 3rd prey fish look (the other 2 already exist) |
| `Images/Background/seaweed_01.png` | Seaweed on the ocean floor |
| `Images/Background/seaweed_02.png` | 2nd seaweed look |
| `Images/Background/seaweed_03.png` | 3rd seaweed look (only have 2? point `weedSprite[2]` at one you already have, in `loadEnvironment()`) |
| `Sound/win.mp3` | Plays once when you finish a level |
| `Sound/lose.mp3` | Plays once on game over |
| `Sound/collect.mp3` | Coin / extra life / speed-up / treasure - one shared sound |

Keep new images roughly the same size/style as their neighbors in the
same folder - nothing needs an exact pixel size, `iShowImage` stretches
to fit either way.

## Beginner tips

- Every gameplay number (speeds, sizes, spawn counts, timers) lives near
  the top of its file under `// ==== 2. CONSTANTS ====` - tune freely,
  nothing else needs to change.
- If a new feature needs its own on/off state, it almost always belongs
  as a `bool`/`int` next to the other globals in that same file's
  `// ==== 4. GLOBAL STATE ====` section - keep the existing pattern
  rather than starting a new one.
- `followPlayer()`, `updateEntities()`, `updateBoats()`, `levelUpdate()`
  are the 4 functions that run every tick - if something should happen
  "constantly while playing," it goes through one of these.
