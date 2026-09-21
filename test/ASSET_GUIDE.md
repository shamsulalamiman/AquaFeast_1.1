# AquaFeast - Asset Guide

Everything about images and sounds, in one place. Short on purpose.

---

## 1. Folder structure

```
test/
├── iMain.cpp + 10 .hpp files      <- the code
├── Images/
│   ├── Background/   sky, sea and scenery
│   ├── Boats/        boat, net, hook
│   ├── Character/    the 3 playable fish
│   ├── Fish/         prey fish (food)
│   ├── HUD/          status-bar icons
│   ├── Menu/         menu button art
│   ├── Powerups/     coin, life, speed, chest
│   ├── Predators/    enemy fish
│   └── splash.png, instruction.png, credit.png, backgroundImage.png
├── Sound/            all .mp3 files
└── scores.txt        created automatically when you first play
```

---

## 2. Naming rules

- lowercase, words joined by `_`, **no spaces**: `prey_small_01.png`
- direction goes last: `_right` `_left` `_up` `_down`
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
rule works for every prey, predator and danger fish.

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
| `Predators/predator_02_up.png` | 120×90 | L2/L3 predator, up | uses right image |
| `Predators/predator_02_down.png` | 120×90 | L2/L3 predator, down | uses right image |
| `Predators/danger_01_right.png` | 150×110 | the halfway danger fish | uses predator art |
| `Predators/danger_01_left.png` | 150×110 | same, facing left | uses right image |
| `Predators/danger_01_up.png` | 150×110 | same, facing up | uses right image |
| `Predators/danger_01_down.png` | 150×110 | same, facing down | uses right image |
| `HUD/mute_on.png` | 44×44 | sound-on icon | uses coin icon |
| `HUD/mute_off.png` | 44×44 | sound-off icon | uses coin icon |
| `HUD/restart.png` | 44×44 | HUD restart button | uses level icon |
| `HUD/progress_fish.png` | 34×26 | fish riding the progress bar | uses a prey fish |
| `HUD/danger_icon.png` | 32×32 | marker over the danger fish | uses warning icon |
| `Menu/button_normal.png` | 360×56 | menu button, not selected | plain panel drawn |
| `Menu/button_selected.png` | 360×56 | menu button, bright/selected | plain panel drawn |
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
| `Predators/predator_01_*`, `predator_02_*` | enemies (01 = L1, 02 = L2/L3) |
| `Background/cloud_01.png` | drifting clouds |
| `Background/bird_01.png` | flying birds |
| `Background/seaweed_01..03.png` | the plants covering the sea floor |
| `Background/bubble_01.png` | **both** bubble systems (rising + swim trail) |
| `Boats/boat_01.png` | the fishing boat |
| `Boats/net_open.png` | level 2 nets |
| `Boats/hook_01.png` | level 3 hook |
| `Powerups/*.png` | coin, extra life, speed up, treasure chest |
| `HUD/level, life, timer, coin.png` | status bar icons |
| `HUD/warning_icon.png` | the "this fish can eat you" marker |
| `splash.png` | the loading screen background |
| `instruction.png`, `credit.png` | currently unused - both popups are now drawn as text so they fit the 500×400 popup size. Keep them or delete them, your choice. |
| `gameover.png`, `win.png` | unused - the end popups are drawn instead |

---

## 6. Recommended sizes

- **prey fish**: 80×60
- **predators**: 120×90 · **danger fish**: 150×110
- **player fish**: 140×100
- **HUD icons**: 44×44 · **small markers**: 32×32
- **menu buttons**: 360×56
- **full-screen art** (`splash.png`): 1900×1000

All images should be **PNG with a transparent background** (except
full-screen art). Exact sizes are not critical - the game stretches
images to fit - but keeping the shape roughly right stops them looking
squashed.

---

## 7. Beginner tips

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
