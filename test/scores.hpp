#ifndef SCORES_HPP
#define SCORES_HPP
// =====================================================================
// scores.hpp - the player score database.
//
// Stored as a plain text file (scores.txt), one record per line:
//     <name> <score>
// Plain text means you can open it in Notepad to check it, and it needs
// no database library. Only the top MAX_SCORES are ever kept.
// =====================================================================
#include "utility.hpp"

#define MAX_SCORES 10

struct ScoreRow {
    char name[24];
    int  score;
};

ScoreRow scoreTable[MAX_SCORES];
int scoreCount = 0;

void loadScores() {
    scoreCount = 0;
    FILE* f = NULL;
    fopen_s(&f, "scores.txt", "r");
    if (!f) return;                     // no file yet = no scores yet

    while (scoreCount < MAX_SCORES) {
        char n[24];
        int s;
        if (fscanf_s(f, "%23s %d", n, (unsigned)_countof(n), &s) != 2) break;
        strcpy_s(scoreTable[scoreCount].name, n);
        scoreTable[scoreCount].score = s;
        scoreCount++;
    }
    fclose(f);
}

void saveScores() {
    FILE* f = NULL;
    fopen_s(&f, "scores.txt", "w");
    if (!f) return;
    for (int i = 0; i < scoreCount; i++)
        fprintf(f, "%s %d\n", scoreTable[i].name, scoreTable[i].score);
    fclose(f);
}

// Finds a player's existing row, or -1 if they are new.
int findScoreRow(const char* name) {
    for (int i = 0; i < scoreCount; i++)
        if (strcmp(scoreTable[i].name, name) == 0) return i;
    return -1;
}

// Keeps the table ordered highest-first (simple bubble sort - the table
// is only 10 rows, so an easy-to-read sort is better than a clever one).
void sortScores() {
    for (int i = 0; i < scoreCount - 1; i++)
        for (int j = 0; j < scoreCount - 1 - i; j++)
            if (scoreTable[j].score < scoreTable[j + 1].score) {
                ScoreRow tmp = scoreTable[j];
                scoreTable[j] = scoreTable[j + 1];
                scoreTable[j + 1] = tmp;
            }
}

// Called when a run ends. A player keeps only their BEST score, so
// replaying never lowers what they already earned.
void submitScore(const char* name, int score) {
    if (name[0] == '\0') return;

    int row = findScoreRow(name);
    if (row >= 0) {
        if (score > scoreTable[row].score) scoreTable[row].score = score;
    }
    else if (scoreCount < MAX_SCORES) {
        strcpy_s(scoreTable[scoreCount].name, name);
        scoreTable[scoreCount].score = score;
        scoreCount++;
    }
    else {
        // Table is full - replace the lowest score, but only if we beat it.
        sortScores();
        if (score > scoreTable[MAX_SCORES - 1].score) {
            strcpy_s(scoreTable[MAX_SCORES - 1].name, name);
            scoreTable[MAX_SCORES - 1].score = score;
        }
    }
    sortScores();
    saveScores();
}

int bestScore() {
    sortScores();
    return scoreCount > 0 ? scoreTable[0].score : 0;
}

#endif
