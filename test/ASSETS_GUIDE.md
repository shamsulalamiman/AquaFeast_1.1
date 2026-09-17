# AquaFeast — Asset Naming, Folder & Image Size Guide

Claude can't generate real game art, so this is the missing piece: a
consistent naming/folder scheme AND exact pixel sizes for every image the
code currently expects, so whatever you draw, buy, or download lines up
with `iLoadImage(...)` calls without you having to guess or edit code paths.

## What changed recently (read this if you saw an earlier version of this file)

- **Sky, ocean, and hills are no longer images.** They're drawn with
  `iFilledRectangle` (sky/ocean) and `iFilledPolygon` (hills, as simple
  triangles) directly in `environment.hpp`, using plain colors. There is
  **no `sky_01.png`, `ocean_01.png`, or `hill_01.png` anymore** — don't
  bother making these.
- **The HUD status bar is a plain rectangle now too**, not an image.
- **The home menu, level-select, instructions, and credits screens are
  all plain colored backgrounds + text** — no menu button/background
  images needed at all. The `Images/Menus/` folder is no longer used by
  the code; you can delete it or leave it empty.
- **Magnet has been removed.** `Images/Powerups/magnet_01.png` is gone.
- **Speed Boost was renamed Speed-Up** and its file is now
  `Images/Powerups/speed_up_01.png` (was `speed_boost_01.png`).
- **`fish_up.png` and `fish_down.png` are now actually used** — the
  player's fish visibly faces up/down as well as left/right.
- **High Score has been removed** (coming back later as its own feature)
  — no high-score screen or save file anymore.
- **Coins are gone as a separate stat.** Coin pickups and the treasure
  chest add straight to score now — there's nothing coin-related left to
  make art for.
- **Dark Water Mode / rain has been removed entirely.**
  `rain_overlay_01.jpg` is no longer used.

## How to read the size table

`iShowImage(x, y, width, height, textureID)` **stretches** whatever image
you give it to exactly `width × height` on screen — it does not care what
resolution the source file actually is. That means:

- **"Draw size"** is what the code currently displays it at.
- **"Recommended source size"** is bigger than the draw size on purpose —
  supplying a higher-resolution image and letting `iShowImage` scale it
  down looks sharp; supplying something *smaller* than the draw size gets
  stretched and looks blurry. Roughly "2-4x the draw size" covers everything.
- **Where width ≠ height, your source image must use the same ratio**, or
  it will look squashed. Where width = height, use a square source image.

## Every image the code currently expects

| Folder / filename | Used in | Draw size (W×H px) | Recommended source size | Needs transparency? |
|---|---|---|---|---|
| `Images/Player/fish_right.png` | `player.hpp` | 24–110 (scales with growth) | 128×128 | Yes |
| `Images/Player/fish_left.png` | `player.hpp` | 24–110 | 128×128 | Yes |
| `Images/Player/fish_up.png` | `player.hpp` | 24–110 | 128×128 | Yes |
| `Images/Player/fish_down.png` | `player.hpp` | 24–110 | 128×128 | Yes |
| `Images/Player/fish_jump.png` | `player.hpp` | 24–110 | 128×128 | Yes |
| `Images/Fish/prey_small_01.png` | `entities.hpp` | 16×16 | 64×64 | Yes |
| `Images/Fish/prey_medium_01.png` | `entities.hpp` (loaded, reserved for a bigger prey type you spawn later) | your choice | 64×64 | Yes |
| `Images/Fish/school_fish_01.png` | `entities.hpp` | 14×14 | 64×64 | Yes |
| `Images/Predators/predator_01.png` | `entities.hpp` (level 1, temporary fallback for all 4 directions) | 60×60 | 128×128 | Yes |
| `Images/Predators/predator_01_right.png` / `_left` / `_up` / `_down` | `entities.hpp` (NEW - not added yet) | 60×60 | 128×128 | Yes |
| `Images/Predators/predator_02.png` | `entities.hpp` (level 2 & 3, tougher look) | 60×60 | 128×128 | Yes |
| `Images/HUD/warning_icon.png` | `entities.hpp` (predator attack warning) | 24×24 | 64×64 | Yes |
| `Images/HUD/restart_icon.png` | `environment.hpp` (NEW) | 26×26 | 64×64 | Yes |
| `Images/HUD/mute_icon.png` | `environment.hpp` (NEW) | 26×26 | 64×64 | Yes |
| `Images/Powerups/coin_01.png` | `boats.hpp` | 20×20 | 64×64 | Yes |
| `Images/Powerups/extra_life_01.png` | `boats.hpp` | 20×20 | 64×64 | Yes |
| `Images/Powerups/speed_up_01.png` | `boats.hpp` | 20×20 | 64×64 | Yes |
| `Images/Powerups/treasure_chest_01.png` | `boats.hpp` | 28×28 | 96×96 | Yes |
| `Images/Boats/boat_01.png` | `boats.hpp` | 80×40 | 320×160 | Yes |
| `Images/Boats/hook_01.png` | `boats.hpp` (level 3 only) | 16×16 | 64×64 | Yes |
| `Images/Boats/net_01.png` | `boats.hpp` (level 2+) | 16×16 | 64×64 | Yes |
| `Images/Background/cloud_01.png` | `environment.hpp` | 80×40 | 240×120 | Yes |
| `Images/Background/bird_01.png` | `environment.hpp` | 40×24 | 120×72 | Yes |
| `Images/Background/bubble_01.png` | `environment.hpp` (NEW, drawn at 25% opacity) | 16×16 | 64×64, soft round edges | Yes |

That's the complete list — sky, ocean, hills, the HUD bar, and every menu
screen are drawn with code (colors + shapes + text), not image files.
`powerups.hpp` was merged into `boats.hpp` this round (see `DEV_NOTES.md`),
so every power-up/chest sprite is loaded there now, not in a separate file.
Dark Water Mode has been removed entirely, so
`Images/Background/rain_overlay_01.jpg` is no longer loaded by anything —
safe to delete.

**"Needs transparency?"** = save as `.png` with a real alpha channel.
Everything in the table above overlaps other art, so all of it needs this.

## Folder layout (already created in the project)

```
Images/
├── Player/       — the player's own fish sprite(s): right/left/up/down/jump
├── Fish/         — prey fish + schooling fish the player can eat
├── Predators/    — dangerous fish/predators
├── Powerups/     — coins, extra life, speed-up, treasure chest
├── HUD/          — just the predator warning icon now
├── Boats/        — boats, hooks, nets
└── Background/   — clouds, birds (sky/ocean/hills are code, not files)
Sound/
├── button.mp3    — the one click sound for every menu button press
└── bgMusic.mp3   — loops for as long as you're actually playing a level
```

Every menu screen is drawn with code (colors + shapes + text) — there's no
`Images/Menus/` folder at all anymore.

## File naming pattern

```
<subject>_<variant/number>.<ext>
```

- `<subject>` — lowercase, underscores instead of spaces: `fish_right`,
  `predator`, `coin`, `boat`, `cloud`.
- `<variant/number>` — a two-digit number (`01`, `02`, ...) for multiple
  versions of the same thing, or a descriptive word for a distinct
  *state* (`right` / `left` / `up` / `down` / `jump`, not `01`/`02`).
- `<ext>` — `.png` for everything in the table above (all of it needs
  transparency).

## Adding a NEW kind of sprite (e.g. a third predator species)

1. Drop the file in the matching folder, sized per the table above —
   e.g. `Images/Predators/predator_03.png`.
2. Add one `int` sprite handle + one `iLoadImage(...)` line in that
   system's `loadX()` function (e.g. `loadEntities()` in `entities.hpp`).
3. Use that new handle when spawning the new entity.

Same three-step pattern every time: folder → `loadX()` line → use the
handle when spawning/drawing.

## Audio (now implemented, via `mciSendString`)

Sound is live now — `Sound/button.mp3` plays on every menu Up/Down/Enter/
Backspace press, and `Sound/bgMusic.mp3` loops for the duration of a level
(starts in `requestStartLevel()`, stops in `returnToHomeMenu()`, both in
`menu.hpp`). There's also an unused `Sound/gamestart.mp3` sitting ready if
you want a one-shot "level start" sting later — see `DEV_NOTES.md`.

If you add more sound effects later (eat, hurt, jump), the same
`mciSendString` pattern in `playButtonSound()`/`startGameplayMusic()`
(`menu.hpp`) is the one to copy — three lines each: close any previous
instance, open the file with a unique alias, then play it.

## If you add art with a different filename than the code expects

Rename the *file* to match the code (preferred), or tell Claude the real
filename and have it update the matching `iLoadImage(...)` call. Either
works, just keep the two in sync.

## Why this matters for working with Claude across sessions

Every path Claude writes in code is a *guess* about what you'll eventually
put on disk. If folder/naming/size stays consistent, Claude's guesses stay
correct across many sessions without re-explaining your file layout each
time — the `igraphics-cpp` skill does this for the *library's* API, this
guide does it for *your own* asset paths.
