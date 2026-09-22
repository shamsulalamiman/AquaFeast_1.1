# AquaFeast - Asset Guide

Everything about images and sounds, in one place. Short on purpose.

---

## 1. Folder structure

```
test/
├── iMain.cpp + 10 .hpp files      <- the code
├── Images/
│   ├── Background/   sky, sea, seaweed, sun/moon, clouds/birds, bubble
│   ├── Boats/        boat, net, hook
│   ├── Character/    the 3 playable fish
│   ├── Fish/         prey fish (food)
│   ├── HUD/          status-bar icons
│   ├── Menu/         menu button art + selection fish
│   ├── Powerups/     coin, life, speed, chest
│   ├── Predators/    enemy fish (normal + the danger enemy)
│   └── splash.png, logo.png, instruction.png, credit.png,
│       score.png, backgroundImage.png
├── Sound/            all .mp3 files
└── scores.txt        created automatically when you first play
```

---

## 2. Naming rules

- lowercase, words joined by `_`, **no spaces**: `prey_small_01.png`
- direction goes last: `_right` `_left` - **that is all**. Every
  swimmer (player, prey, predators, the danger enemy) only ever shows
  its left/right picture now, even while moving up or down, so `_up`
  and `_down` images are no longer used anywhere and do not need to
  exist.
- extra animation frames add `_f2` / `_f3` **before** `.png`
- sounds are lowercase `.mp3`

**Never rename a file that already works** - the code looks for these
exact names.

---

## 3. How fish animation works

A swimming fish uses up to 3 pictures shown in turn:

```
prey_small_01.png        <- frame 1 (you already have this)
prey_small_01_f2.png     <- frame 2 (optional)
prey_small_01_f3.png     <- frame 3 (optional)
```

Missing frames fall back to frame 1, so the fish just glides instead of
flapping. **Nothing breaks if you never add them.** The same `_f2`/`_f3`
rule works for prey, predators, and the danger enemy - `loadSwimFrames()`
in `entities.hpp` is the ONE function all of them use (predators no
longer have their own separate loading function, since they no longer
need up/down variants).

---

## 4. Files the game wants but does not have yet

The game **runs right now** without all of these - each falls back to an
image you already own. Add them to finish the look.

### Images

| File | Size | Used for | If missing |
|---|---|---|---|
| `Background/sun.png` | 170×170 | Level 1 sunny sky | blank space |
| `Background/moon.png` | 150×150 | Level 2 night sky | blank space |
| `Background/bird_02.png` | 66×48 | bird wing frame 2 | bird glides |
| `Background/bird_03.png` | 66×48 | bird wing frame 3 | bird glides |
| `Fish/prey_small_02_left.png` | 80×60 | 3rd prey facing left | uses right image |
| `Predators/danger_01_right.png` | 150×110 | the halfway danger fish | uses predator art |
| `Predators/danger_01_left.png` | 150×110 | same, facing left | uses right image |
| `HUD/mute_on.png` | 44×44 | sound-on icon | uses coin icon |
| `HUD/mute_off.png` | 44×44 | sound-off icon | uses coin icon |
| `HUD/restart.png` | 44×44 | HUD restart button | uses level icon |
| `HUD/progress_fish.png` | 34×26 | fish riding the progress bar | uses a prey fish |
| `HUD/danger_icon.png` | 32×32 | marker over the danger fish | uses warning icon |
| `Menu/button_normal.png` | 360×56 | menu button, not selected | plain panel drawn |
| `Menu/button_selected.png` | 360×56 | menu button, bright/selected (popups/map only) | plain panel drawn |
| `Menu/select_fish.png` | 44×32 | marker beside the chosen row | uses a prey fish |

**Menu button art:** both files must exist or the game uses drawn
panels for both - it never mixes one image with one drawn panel.

### Sounds

| File | Used for | If missing |
|---|---|---|
| `Sound/swim.mp3` | soft swish while swimming sideways | silent |
| `Sound/struggle.mp3` | fish shaking on the level-3 hook | silent |
| `Sound/alert.mp3` | the danger fish arriving at halfway | silent |

Keep them **short** (under 1 second) - they replay often.

---

## 5. Where each existing asset is used

| Asset | Used by |
|---|---|
| `Character/fish_character_0N_right/left.png` | the 3 fish you can choose |
| `Fish/prey_*.png` | food fish, and the menu background fish |
| `Predators/predator_01_*`, `predator_02_*` | enemies (01 = L1, 02 = L2/L3) - right/left only |
| `Predators/danger_01_*` | the halfway "danger enemy", one per level, right/left only |
| `Background/cloud_01.png` | drifting clouds |
| `Background/bird_01.png` | flying birds |
| `Background/seaweed_01..03.png` | the plants covering the whole sea floor |
| `Background/bubble_01.png` | **both** bubble systems (rising + swim trail) |
| `Boats/boat_01.png` | the fishing boat |
| `Boats/net_open.png` | level 2 nets |
| `Boats/hook_01.png` | level 3 hook |
| `Powerups/*.png` | coin, extra life, speed up, treasure chest |
| `HUD/level, life, timer, coin.png` | status bar icons |
| `HUD/warning_icon.png` | the "this fish can eat you" marker |
| `splash.png` | the loading screen background |
| `logo.png` | the AquaFeast title art shown on the main menu |
| `score.png` | the Score popup's background art |
| `instruction.png` | the Instructions popup's background art |
| `credit.png` | the Credits popup's background art |
| `gameover.png`, `win.png` | unused - the end popups are drawn instead |

---

## 6. Music (one continuous track)

There is only ONE background track: `Sound/bgMusic.mp3`. It starts once,
the moment the splash/loading screen finishes, and keeps playing through
the menu, every popup, AND gameplay without ever stopping or restarting.
Muting pauses it (so unmuting picks up from the same spot); it is never
stopped and replayed from the start.

Every other sound (`eat`, `collect`, `win`, `lose`, `swim`, `struggle`,
`alert`, `button`) is a short one-shot effect that plays on top of the
music independently - eating a fish, for example, does not interrupt
the music at all.

`Sound/gamestart.mp3` is no longer wired into anything (the game used to
switch to it as separate "menu music", which is what caused the music
bugs) - the file can stay in the folder unused, or you can remove it.
`Sound/loading.mp3` is not used either - there is no splash-screen music
by design.

---

## 7. Recommended sizes

- **prey fish**: 80×60
- **predators**: 100×70 · **danger fish (the big one)**: 170×120
- **player fish**: 120×80
- **HUD icons**: 44×44 · **small markers**: 32×32
- **menu buttons**: 360×56
- **full-screen art** (`splash.png`, `logo.png` is wide/short instead): 1900×1000

All images should be **PNG with a transparent background** (except
full-screen art). Exact sizes are not critical - the game stretches
images to fit - but keeping the shape roughly right stops them looking
squashed. The whole game now uses smaller everyday fish than before, on
purpose - the danger enemy is the one that should look noticeably big.

---

## 8. Beginner tips

- Every number you might want to tune is near the **top** of its file
  under a `TUNING` or `CONSTANTS` heading.
- **Never call `iLoadImage()` while drawing.** All loading happens once
  in `loadEverything()` in `iMain.cpp`. Loading from disk every frame
  makes the game stutter and keys feel unresponsive.
- To change how hard a level is, edit the `levels[]` table in
  `levels.hpp` - width, goal size, time, fish counts and speeds are all
  in one place.
- To change the hook fight, edit `PULL_PER_TICK`, `PUSH_PER_TAP` and
  `EFFORT_DECAY` at the top of `boat.hpp`.
- To change the world-scroll range, see `followPlayer()` in
  `environment.hpp` - it must always be `[CENTER_X, worldWidth - CENTER_X]`,
  never `[0, worldWidth]`, or the camera can slide past the true edges
  of the ocean.
