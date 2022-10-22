#ifndef _SAVEFILE_H
#define _SAVEFILE_H

#define MAX_LEVELS  32

#define SAVEFILE_HEADER 0xBEEF

struct LevelSaveData {
    // measured in tenths of a second
    unsigned short completionTime;
};

struct SaveData {
    unsigned header;
    unsigned short secretsUnlocked;
    struct LevelSaveData levels[MAX_LEVELS];
};

extern int gShouldSave;

void saveFileLoad();
void saveFileSave();

void saveFileCheckSave();

int saveFileIsLevelComplete(int level);
unsigned short saveFileLevelTime(int level);
void saveFileMarkLevelComplete(int level, float time);

void saveFileErase();

#endif