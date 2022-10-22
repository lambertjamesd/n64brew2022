
#include "savefile.h"
#include "util/memory.h"
#include "controls/controller.h"

struct SaveData gSaveData;
int gEepromProbe;
int gShouldSave = 0;

#ifdef DEBUG
#define UNLOCK_ALL  1
#else
#define UNLOCK_ALL  0
#endif

void saveFileNew() {
    zeroMemory(&gSaveData, sizeof(gSaveData));
    gSaveData.header = SAVEFILE_HEADER;
}

void saveFileLoad() {
    OSMesgQueue         serialMsgQ;
    OSMesg              serialMsg;

    osCreateMesgQueue(&serialMsgQ, &serialMsg, 1);
    osSetEventMesg(OS_EVENT_SI, &serialMsgQ, (OSMesg)1);

    gEepromProbe = osEepromProbe(&serialMsgQ);

    if (gEepromProbe) {
        osEepromLongRead(&serialMsgQ, 0, (u8*)&gSaveData, sizeof(gSaveData));

        if (gSaveData.header != SAVEFILE_HEADER) {
            saveFileNew();
        }
    } else {
        saveFileNew();
    }

    controllersListen();
}

void saveFileCheckSave() {
    if (gShouldSave && gEepromProbe) {
        OSMesgQueue         serialMsgQ;
        OSMesg              serialMsg[8];

        osCreateMesgQueue(&serialMsgQ, &serialMsg[0], 8);
        osSetEventMesg(OS_EVENT_SI, &serialMsgQ, (OSMesg)1);
        
        controllersClearState();
        osEepromLongWrite(&serialMsgQ, 0, (u8*)&gSaveData, sizeof(gSaveData));
        controllersListen();
        gShouldSave = 0;
    } else {
        gShouldSave = 0;
    }
}

void saveFileSave() {
    gShouldSave = 1;
}

int saveFileIsLevelComplete(int level) {
    return UNLOCK_ALL || gSaveData.levels[level].completionTime != 0;
}

unsigned short saveFileLevelTime(int level) {
    return gSaveData.levels[level].completionTime;
}

void saveFileMarkLevelComplete(int level, float time) {
    unsigned short newTime = (unsigned short)(time * 10.0f);
    if (gSaveData.levels[level].completionTime == 0 || newTime < gSaveData.levels[level].completionTime) {
        gSaveData.levels[level].completionTime = newTime;
    }
    gShouldSave = 1;
}

void saveFileErase() {
    saveFileNew();
    gShouldSave = 1;
}