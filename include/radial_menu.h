#ifndef RADIAL_MENU_H
#define RADIAL_MENU_H

#include "ultra64/ultratypes.h"
#include "game.h"

#define RADIAL_COUNT_MAX 8

struct RadialMenu;

typedef enum {
    RADIAL_ITEM_NOTHING = (0 << 0),
    RADIAL_ITEM_HOVERED = (1 << 0),
    RADIAL_ITEM_TEX_SYNCED = (1 << 1),
} RadialMenuItemState;

typedef enum {
    RADIAL_MENU_NOTHING = (0 << 0),
    RADIAL_MENU_OPEN = (1 << 0),
    RADIAL_MENU_DESTROY = (1 << 2),
} RadialMenuState;

typedef void (*RadialMenuItemSelectFunc)(void*);
typedef void (*RadialMenuUpdateFunc)(struct RadialMenu*, GameState*);

typedef struct {
    RadialMenuItemState state;
    void* texture;
    u8 texWidth;
    u8 texHeight;
    s16 x;
    s16 y;
    f32 scale;
    RadialMenuItemSelectFunc onSelect;
    u8* texSegment;
    size_t texLen;
    u8 texFormat;
    u8 texPixelSize;
    s16 angle;
} RadialMenuItem;

typedef struct RadialMenu {
    RadialMenuState state;
    struct {
        RadialMenuItem* elements;
        u8 count;
    } items;
    u8 alpha;
    u8 hoveredItemIndex;
    u16 radius;
    u16 targetRadius;
    s16 x;
    s16 y;
    RadialMenuUpdateFunc update;
    s16 cursorX;
    s16 cursorY;
    f32 progress;
    u16 prevRadius;
    u8 targetAlpha;
} RadialMenu;

typedef struct {
    RadialMenu* elements[RADIAL_COUNT_MAX];
    u8 count;
} RadialMenuContext;

u8 RadialMenu_Init(RadialMenuContext* radialMenuCtx, s16 x, s16 y);
u8 RadialMenu_AddItem(RadialMenu* this, void* texture, u8 texFormat, u8 texPixelSize, u8 texWidth, u8 texHeight, RadialMenuItemSelectFunc onSelect);
u8 RadialMenu_AddItemSync(RadialMenu* this, uintptr_t textureVrom, u8 texFormat, u8 texPixelSize, u8 texWidth, u8 texHeight, RadialMenuItemSelectFunc onSelect, size_t texLen);
void RadialMenu_RemoveItem(RadialMenu* this, u8 index);
void RadialMenu_Open(RadialMenu* this, u16 radius);
void RadialMenu_Close(RadialMenu* this);
void RadialMenu_Destroy(RadialMenu* this, RadialMenuContext* radialMenuCtx, u8* handlerIndex);
void RadialMenu_HandleAll(RadialMenuContext* radialMenuCtx, GameState* state);

#endif
