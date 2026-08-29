#ifndef SFXLIST_STATE_H
#define SFXLIST_STATE_H

#include "game.h"
#include "view.h"

typedef struct {
    GameState state;
    View view;
} SFXListState;

void SFXList_Init(GameState* thisx);
void SFXList_Destroy(GameState* thisx);

#endif