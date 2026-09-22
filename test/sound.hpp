#ifndef SOUND_HPP
#define SOUND_HPP
// =====================================================================
// sound.hpp - every sound in the game, in one place.
//
// <windows.h> + <mmsystem.h> are needed for mciSendString(), the
// Windows function that plays .mp3 files. iGraphics has no audio of
// its own, so this is the standard way to add sound on Windows.
// (winmm.lib is linked in iMain.cpp.)
//
// MUSIC: there is now only ONE background track (bgMusic.mp3). It
// starts once, the moment the splash screen finishes, and just keeps
// playing continuously through the menu, every popup, AND gameplay -
// it is never stopped or restarted, only paused/resumed by mute. The
// short one-shot sounds (eat/collect/win/lose/etc.) all play over it
// normally, since each mp3 alias is independent.
// =====================================================================
#include <windows.h>
#include <mmsystem.h>
#include "utility.hpp"

// Each sound is opened ONCE at startup and kept open. Opening an mp3 is
// slow (it reads the disk), so doing it on every play would stutter the
// game. After this, playing is just "rewind and go".
inline void openSound(const char* file, const char* alias) {
    if (!imageExists(file)) return;   // the same file check works for any file
    char cmd[200];
    sprintf_s(cmd, "open \"%s\" type mpegvideo alias %s", file, alias);
    mciSendString(cmd, NULL, 0, NULL);
}

void loadSounds() {
    openSound("Sound/button.mp3",    "sndBtn");
    openSound("Sound/eat.mp3",       "sndEat");
    openSound("Sound/collect.mp3",   "sndCollect");
    openSound("Sound/win.mp3",       "sndWin");
    openSound("Sound/lose.mp3",      "sndLose");
    openSound("Sound/swim.mp3",      "sndSwim");
    openSound("Sound/struggle.mp3",  "sndStruggle");
    openSound("Sound/alert.mp3",     "sndAlert");
    openSound("Sound/bgMusic.mp3",   "musBg");
    // gamestart.mp3 is no longer used as separate menu music (see the
    // MUSIC note above) - bgMusic.mp3 alone now covers menu+gameplay.
}

inline void playSound(const char* alias) {
    if (isMuted) return;
    char cmd[64];
    sprintf_s(cmd, "play %s from 0", alias);
    mciSendString(cmd, NULL, 0, NULL);
}

inline void stopSound(const char* alias) {
    char cmd[64];
    sprintf_s(cmd, "stop %s", alias);
    mciSendString(cmd, NULL, 0, NULL);
}

void playButton()  { playSound("sndBtn"); }
void playEat()     { playSound("sndEat"); }
void playCollect() { playSound("sndCollect"); }
void playWin()     { playSound("sndWin"); }
void playLose()    { playSound("sndLose"); }
void playAlert()   { playSound("sndAlert"); }

// Short sounds that would machine-gun if played every frame get a
// cooldown, so they space out into one natural-sounding effect.
int swimCooldown = 0;
int struggleCooldown = 0;

void playSwim() {
    if (swimCooldown > 0) return;
    playSound("sndSwim");
    swimCooldown = 14;      // ~0.45s between swishes at a 30ms tick
}

// Played while the fish shakes on the hook.
void playStruggle() {
    if (struggleCooldown > 0) return;
    playSound("sndStruggle");
    struggleCooldown = 10;
}

// Explicitly cuts the struggle sound off - called the instant the fish
// escapes or gets reeled in, so no tail end of it can keep playing.
void stopStruggle() {
    stopSound("sndStruggle");
    struggleCooldown = 0;
}

void tickSoundCooldowns() {
    if (swimCooldown > 0) swimCooldown--;
    if (struggleCooldown > 0) struggleCooldown--;
}

// ==== THE ONE CONTINUOUS BACKGROUND TRACK ====
bool musicStarted = false;

// Called once, right when the splash screen finishes (see menu.hpp's
// updateSplash()). After that it is never called again - the track
// just keeps looping for the rest of the session.
void ensureMusicPlaying() {
    if (musicStarted) return;
    musicStarted = true;
    if (!isMuted) mciSendString("play musBg from 0 repeat", NULL, 0, NULL);
}

// Mute PAUSES the track (so unmuting resumes from the same spot)
// instead of stopping it, which is what "runs all the time" means -
// muting is not the same as the music restarting later.
void toggleMute() {
    isMuted = !isMuted;
    if (!musicStarted) return;
    if (isMuted) mciSendString("pause musBg", NULL, 0, NULL);
    else          mciSendString("resume musBg", NULL, 0, NULL);
}

// Game over can be triggered from more than one place, so it lives here
// - that way the lose sound can only ever fire once. The music itself
// keeps playing straight through a game over.
void triggerGameOver() {
    if (isGameOver) return;
    isGameOver = true;
    playLose();
}

#endif
