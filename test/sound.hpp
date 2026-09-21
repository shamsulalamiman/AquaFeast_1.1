#ifndef SOUND_HPP
#define SOUND_HPP
// =====================================================================
// sound.hpp - every sound in the game, in one place.
//
// <windows.h> + <mmsystem.h> are needed for mciSendString(), the
// Windows function that plays .mp3 files. iGraphics has no audio of
// its own, so this is the standard way to add sound on Windows.
// (winmm.lib is linked in iMain.cpp.)
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
    openSound("Sound/bgMusic.mp3",   "musGame");
    openSound("Sound/gamestart.mp3", "musMenu");
	openSound("Sound/loading.mp3", "loading");
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
void loadingSound()   { playSound("loading"); }

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

void startGameMusic() { if (!isMuted) mciSendString("play musGame from 0 repeat", NULL, 0, NULL); }
void stopGameMusic()  { mciSendString("stop musGame", NULL, 0, NULL); }
void startMenuMusic() { if (!isMuted) mciSendString("play musMenu from 0 repeat", NULL, 0, NULL); }
void stopMenuMusic()  { mciSendString("stop musMenu", NULL, 0, NULL); }

void toggleMute() {
    isMuted = !isMuted;
    if (isMuted) { stopGameMusic(); stopMenuMusic(); return; }
    if (screen == SCR_PLAY) startGameMusic();
    else startMenuMusic();
}

// Game over can be triggered from more than one place, so it lives here
// - that way the lose sound can only ever fire once.
void triggerGameOver() {
    if (isGameOver) return;
    isGameOver = true;
    stopGameMusic();
    playLose();
}

#endif
