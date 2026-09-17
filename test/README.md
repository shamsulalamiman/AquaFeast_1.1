# AquaFeast — full project (3 levels, keyboard-only menu, sound, real art)

A complete, playable AquaFeast build — Home menu → 3 levels, each harder
than the last, entirely keyboard-controlled, with real sprite art and
sound. Written for someone who has recently learned C/C++: plain
fixed-size arrays instead of `vector`, `struct` instead of `class` where
possible, and comments that explain *why* something works the way it
does rather than just restating the line below them.

**Read `DEV_NOTES.md` first** — every decision, assumption, and change is
logged there, plus an alphabetical index of every function in the project
and where to find it.

**Read `ASSETS_GUIDE.md` before adding or changing any images/sounds** —
exact folder, filename, and pixel size for every asset the code expects.

## Controls

- **Home / Level Select menus:** Up / Down to choose, Enter to select, Backspace to go back.
- **Playing:** Arrow keys to swim, Space to jump above the water surface, Backspace any time to quit back to the Home menu.
- **After a level ends (win, lose, or out of time):** Backspace to return to the Home menu.

## The 5 home-screen options

`Start Game` (jumps straight into Level 1) · `Levels` (choose Level 1/2/3) ·
`Instructions` · `Credits` · `Exit` — all Up/Down/Enter, no mouse needed
anywhere. (High Score is intentionally not here yet — coming back later
as its own feature.)

## What's implemented

| Feature | Where |
|---|---|
| Growth by eating, size-based danger | `player.hpp`, `entities.hpp` |
| Once you outgrow a predator, you can eat IT | `entities.hpp` |
| Randomized prey swimming + fish schooling (forms a line, scatters when approached) | `entities.hpp` |
| Extra Life / Speed-Up float above the water, collected only while jumping | `boats.hpp` |
| Speed-Up: 2x speed for 10 seconds, then reverts | `boats.hpp` |
| Treasure chest: random on-screen spot, 10s visible countdown, every 30s | `boats.hpp` |
| Coins and the chest add straight to score — no separate coin count | `boats.hpp` |
| Boat sails left→right every level; Level 2+ throws nets; Level 3 also throws hooks | `boats.hpp` |
| Sky/ocean as colored rectangles, hills as `iFilledPolygon` triangles starting right at the water line | `environment.hpp` |
| Clouds drift left→right, birds drift right→left, continuously, every level | `environment.hpp` |
| Countdown timer per level (120s / 100s / 80s) | `utility.hpp`, `levels.hpp` |
| Keyboard-only menus, one consistent pixel-style font, working Back everywhere | `menu.hpp`, `utility.hpp` |
| Score / Lives / Time Left / Level / Progress HUD | `environment.hpp` |
| One click sound for every menu button, looping music during gameplay only | `menu.hpp`, `iMain.cpp` |
| 3 levels, each with more enemies, a shorter timer, a bigger target size | `levels.hpp`'s `levelConfigs[]` table |

## What's here

- `iGraphics.h` + `glut.h`/`glaux.h`/`stb_image.h`/`bitmap_loader.h` — the
  library itself, bundled so the project compiles standalone.
- `GLU32.LIB`, `GLUT32.DLL`, `Glaux.lib`, `OPENGL32.LIB`, `glut32.lib`,
  `glui32.lib` — binary dependencies, need to sit next to the built `.exe`.
- `test.sln` / `test.vcxproj` / `test.vcxproj.filters` — the Visual Studio
  project. Open `test.sln` and build.
- `utility.hpp` — constants, the pixel font + title-drawing helper, screen
  state flags, the `MenuScreen` enum, `GameStats`, camera, collision helpers.
- `player.hpp`, `entities.hpp`, `boats.hpp`, `environment.hpp`, `menu.hpp` —
  one system each (see `DEV_NOTES.md` for what got merged into what).
- `levels.hpp` — the `levelConfigs[]` table + shared init/update/draw
  logic for all 3 levels.
- `iMain.cpp` — the required iGraphics callbacks + `main()`.
- `Images/`, `Sound/` — real art and audio, already wired up.
- `ASSETS_GUIDE.md` — every image/sound the code expects, with exact size.
- `DEV_NOTES.md` — changelog, function index, and open notes.

## Next steps

Ask for whatever's next — High Score coming back, a pause screen, more
sound effects, tuning the difficulty numbers in `levels.hpp`, or anything
else. `DEV_NOTES.md` has a running "housekeeping" list of small optional
cleanups too.
