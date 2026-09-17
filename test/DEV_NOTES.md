# AquaFeast — Dev Notes (this round)

## The real cause of the slow menu

Not a code bug — `Images/Powerups/coin_01.png` was **5000×5000px / 12.3MB**
and `treasure_chest_01.png` was **3412×3180px / 8.4MB**, both drawn
on-screen at ~25px. Decoding two images that size at startup freezes the
game before the menu even appears. **I resized both (and `speed_up_01.png`,
and the 1536×1024 `backgroundImage.png`) down to sane sizes in place** —
same filenames, ~23.7MB → ~1.5MB combined. I also stopped every sound
effect from re-opening its mp3 file on every single play (see below) since
that's a second, smaller source of lag. Menu should feel instant now.

## What changed

| You asked for | What was done |
|---|---|
| Remove fish schooling | Deleted `FishSchool` struct and all school-spawning/update code from `entities.hpp` — prey now just wander individually. |
| Remove hills | Deleted `drawHills()` and its call — ocean/sky are two plain rectangles now. |
| Fix slow menu | See above, plus sounds are opened once at startup instead of on every press (`loadSounds()` in `menu.hpp`) — repeated open/close was a blocking disk operation on every keystroke. |
| 1/3 sky, 3/4 ocean | Implemented as `OCEAN_FRACTION = 0.75` in `utility.hpp`. Your wording ("one-third sky, three-four ocean") doesn't add to a whole, so I read it as 1/4 sky + 3/4 ocean — change that one constant if you meant something else. |
| Fix 4-direction predators | `predator_01_*` (all 4 directions) are wired in and working. `predator_02` only has one image on your side still — all 4 directions reuse it for now, see the table below. |
| Mouse-clickable Restart/Mute | Real `iMouse()` handling added (`iMain.cpp` + `checkHudClick()` in `environment.hpp`) — click the icons, top-right of the HUD. Keyboard R/M still work too. |
| Simpler functions, fewer variables | Removed dead variables (`playerSize` was set but never read; `preySpriteMedium`/`schoolFishSprite` were loaded but unused). Added one shared `wasJustPressed()` helper instead of repeating the same 3-line debounce pattern ~8 times. |
| Polished file structure | Every `.hpp` now follows the same order: includes → constants → structs → global state → functions. |
| Remove unnecessary lines | Trimmed throughout — restated-the-obvious comments are gone, kept only what explains non-obvious logic. |
| Shorter header files | Combined with the above — every file is close to or under 150 lines now. |
| Easier camera | `cameraY` is now just a plain constant (`= VIEWPORT_CENTER_Y`, never changes) instead of a hand-tuned magic number — so world Y and screen Y always line up automatically, no matter what `WATER_SURFACE_Y` is set to. |
| Fish showing in the sky | Two separate fixes: (1) the camera fix above, and (2) prey now actively bounce back down if wandering carries them near the surface or floor (`updateEntities()` in `entities.hpp`) instead of only being caught by the player's own clamp. |

## Still needed

- `Images/Predators/predator_02_right.png` / `_left` / `_up` / `_down` —
  same naming pattern as `predator_01_*`, which already works correctly.
- Sound files referenced but not confirmed present: `Sound/eat.mp3`,
  `Sound/collect.mp3`, `Sound/win.mp3`, `Sound/lose.mp3`.
- `Images/Fish/school_fish_01.png` and `prey_medium_01.png` are unused now
  that schooling is removed — safe to delete.
