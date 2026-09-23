#ifndef PAUSE_MANAGER_H
#define PAUSE_MANAGER_H

#include "radial_menu.h"

struct PlayState;

typedef enum {
    PAUSE_PAGE_NONE,
    PAUSE_PAGE_QUEST,
    PAUSE_PAGE_INVENTORY,
    PAUSE_PAGE_EQUIPMENT,
} PausePage;

typedef struct {
    PausePage page;
    RadialMenu* inventory;
    RadialMenu* equipment;
    u8 menuCleanupPending;
    u8 playerInputBlocked;
    u8 questShouldDraw;
    s8 questCursorSong;
    s8 questCursorRepeatStateX;
    s8 questCursorRepeatStateY;
    s16 questCursorRepeatTimerX;
    s16 questCursorRepeatTimerY;
} PauseManager;

void PauseManager_Init(PauseManager* this);
void PauseManager_Update(PauseManager* this, struct PlayState* play);
s32 PauseManager_IsPlayerInputBlocked(PauseManager* this, struct PlayState* play);

#endif
