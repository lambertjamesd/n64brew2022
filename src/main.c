#include <ultra64.h>
#include <sched.h>
#include "defs.h"

#include "util/rom.h"
#include "util/time.h"
#include "graphics/graphics.h"
#include "scene/scene.h"
#include "controls/controller.h"
#include "level/level.h"
#include "sk64/skelatool_animator.h"
#include "sk64/skelatool_defs.h"
#include "ui/nightchilde.h"
#include "menu/main_menu.h"
#include "savefile/savefile.h"

#ifdef WITH_DEBUGGER
#include "../debugger/debugger.h"
#endif

OSPiHandle *gPiHandle;

static OSMesg           PiMessages[DMA_QUEUE_SIZE];
static OSMesgQueue      PiMessageQ;

u64    mainStack[STACKSIZEBYTES/sizeof(u64)];

OSThread initThread;
u64 initThreadStack[STACKSIZEBYTES/sizeof(u64)];

OSThread gameThread;
u64 gameThreadStack[STACKSIZEBYTES/sizeof(u64)];

OSMesgQueue      gfxFrameMsgQ;
static OSMesg           gfxFrameMsgBuf[MAX_FRAME_BUFFER_MESGS];
static OSScClient       gfxClient;

OSSched scheduler;
u64            scheduleStack[OS_SC_STACKSIZE/8];
OSMesgQueue	*schedulerCommandQueue;

extern char _material_dataSegmentRomStart[];
extern char _material_dataSegmentRomEnd[];

void initProc(void *arg);
void gameProc(void *arg);

void main(void *arg) {
    osInitialize();

    gPiHandle = osCartRomInit();

    osCreateThread(
        &initThread,
        1,
        initProc,
        arg,
        (void*)(initThreadStack + (STACKSIZEBYTES/sizeof(u64))),
        (OSPri)INIT_PRIORITY
    );

    osStartThread(&initThread);
}

void initProc(void *arg) {
    osCreatePiManager(
        (OSPri)OS_PRIORITY_PIMGR,
        &PiMessageQ,
        PiMessages,
        DMA_QUEUE_SIZE
    );

    osCreateThread(
        &gameThread,
        6,
        gameProc,
        0,
        gameThreadStack + (STACKSIZEBYTES/sizeof(u64)),
        (OSPri)GAME_PRIORITY
    );

    osStartThread(&gameThread);

    osSetThreadPri(NULL, 0);
    for(;;);
}

extern OSMesgQueue dmaMessageQ;

extern char _heapStart[];

extern char _animation_segmentSegmentRomStart[];

struct Scene gScene;

enum GameState {
    GameStateMainMenu,
    GameStateScene,
};

void gameProc(void *arg) {
    u8 schedulerMode = OS_VI_NTSC_HPF1;

	switch (osTvType) {
		case 0: // PAL
			schedulerMode = HIGH_RES ? OS_VI_PAL_HPF1 : OS_VI_PAL_LPF1;
			break;
		case 1: // NTSC
			schedulerMode = HIGH_RES ? OS_VI_NTSC_HPF1 : OS_VI_NTSC_LPF1;
			break;
		case 2: // MPAL
            schedulerMode = HIGH_RES ? OS_VI_MPAL_HPF1 : OS_VI_MPAL_LPF1;
			break;
	}

    osCreateScheduler(
        &scheduler,
        (void *)(scheduleStack + OS_SC_STACKSIZE/8),
        SCHEDULER_PRIORITY,
        schedulerMode,
        1
    );


    schedulerCommandQueue = osScGetCmdQ(&scheduler);

    osCreateMesgQueue(&gfxFrameMsgQ, gfxFrameMsgBuf, MAX_FRAME_BUFFER_MESGS);
    osScAddClient(&scheduler, &gfxClient, &gfxFrameMsgQ);

	osViSetSpecialFeatures(OS_VI_GAMMA_OFF |
			OS_VI_GAMMA_DITHER_OFF |
			OS_VI_DIVOT_OFF |
			OS_VI_DITHER_FILTER_OFF);

    u32 pendingGFX = 0;
    u32 drawBufferIndex = 0;

    u16* memoryEnd = graphicsLayoutScreenBuffers((u16*)PHYS_TO_K0(osMemSize));

    int materialChunkSize = _material_dataSegmentRomEnd - _material_dataSegmentRomStart;

    memoryEnd -= materialChunkSize / sizeof(u16);

    heapInit(_heapStart, memoryEnd);
    romInit();

    gMaterialSegment = memoryEnd;
    romCopy(_material_dataSegmentRomStart, (char*)memoryEnd, materialChunkSize);

    nightChildeInit();

    skInitDataPool(gPiHandle);
    skSetSegmentLocation(CHARACTER_ANIMATION_SEGMENT, (unsigned)_animation_segmentSegmentRomStart);
    
    levelQueueLoad(MAIN_MENU_LEVEL);

    controllersInit();
    saveFileLoad();

#ifdef WITH_DEBUGGER
    OSThread* debugThreads[2];
    debugThreads[0] = &gameThread;
    gdbInitDebugger(gPiHandle, &dmaMessageQ, debugThreads, 1);
#endif

    while (1) {
        OSScMsg *msg = NULL;
        osRecvMesg(&gfxFrameMsgQ, (OSMesg*)&msg, OS_MESG_BLOCK);
        
        switch (msg->type) {
            case (OS_SC_RETRACE_MSG):
                static int renderSkip = 1;

                if (levelGetQueued() != NO_QUEUED_LEVEL) {
                    if (pendingGFX == 0) {
                        heapInit(_heapStart, memoryEnd);

                        if (levelGetQueued() == MAIN_MENU_LEVEL) {
                            mainMenuInit(&gMainMenu);
                            gCurrentLevelIndex = MAIN_MENU_LEVEL;
                            levelQueueLoad(NO_QUEUED_LEVEL);
                        } else {
                            loadLevel(levelGetQueued());
                            sceneInit(&gScene, gCurrentLevel, 1);
                        }
                    }

                    break;
                }

                if (pendingGFX < 2 && !renderSkip) {
                    if (gCurrentLevelIndex == MAIN_MENU_LEVEL) {
                        graphicsCreateTask(&gGraphicsTasks[drawBufferIndex], (GraphicsCallback)mainMenuRender, &gMainMenu);
                    } else {
                        graphicsCreateTask(&gGraphicsTasks[drawBufferIndex], (GraphicsCallback)sceneRender, &gScene);
                    }
                    drawBufferIndex = drawBufferIndex ^ 1;
                    ++pendingGFX;
                } else if (renderSkip) {
                    --renderSkip;
                }

                if (gShouldSave) {
                    if (!controllerHasPendingMessage()) {
                        saveFileCheckSave();
                    }
                } else {
                    controllersTriggerRead();
                }

                skReadMessages();
                if (gCurrentLevelIndex == MAIN_MENU_LEVEL) {
                    mainMenuUpdate(&gMainMenu);
                } else {
                    sceneUpdate(&gScene);
                }
                timeUpdateDelta();

                char msg[64];
                sprintf(msg, "current=%x start=%x end=%x dpstat=%x spstat=%x\n",
                    IO_READ(DPC_CURRENT_REG),					
                    IO_READ(DPC_START_REG),						
                    IO_READ(DPC_END_REG),						
                    IO_READ(DPC_STATUS_REG),					
                    IO_READ(SP_STATUS_REG));


                break;

            case (OS_SC_DONE_MSG):
                --pendingGFX;
                break;
            case (OS_SC_PRE_NMI_MSG):
                pendingGFX += 2;
                break;
            case SIMPLE_CONTROLLER_MSG:
                controllersUpdate();
                break;
        }
    }
}