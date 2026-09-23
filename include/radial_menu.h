#ifndef RADIAL_MENU_H
#define RADIAL_MENU_H

#include "ultra64/ultratypes.h"
#include "game.h"

#define RADIAL_COUNT_MAX 8
#define RADIAL_ITEM_INVALID 0xFF

struct RadialMenu;
struct RadialMenuItem;
struct RadialMenuContext;

typedef enum {
    RADIAL_ITEM_NOTHING = (0 << 0),
    RADIAL_ITEM_HOVERED = (1 << 0),
} RadialMenuItemState;

#define RADIAL_ITEM_CONTROL_COUNT 13

typedef enum {
    RADIAL_MENU_NOTHING = (0 << 0),
    RADIAL_MENU_OPEN = (1 << 0),
    RADIAL_MENU_DESTROY = (1 << 2),
} RadialMenuState;

typedef void (*RadialMenuItemSelectFunc)(u16 btn, struct RadialMenu*, struct RadialMenuItem*, GameState* state);
typedef void (*RadialMenuItemPostDrawFunc)(struct RadialMenu*, struct RadialMenuItem*, GameState* state, Gfx** gfxP);
typedef void (*RadialMenuUpdateFunc)(struct RadialMenu*, GameState*);

typedef struct RadialMenuItem {
    RadialMenuItemState state;
    void* texture;
    u8 texWidth;
    u8 texHeight;
    s16 x;
    s16 y;
    f32 scale;
    RadialMenuItemSelectFunc onSelect;
    RadialMenuItemPostDrawFunc postDraw;
    RadialMenuItemPostDrawFunc preDraw;
    u8 texFormat;
    u8 texPixelSize;
    s16 angle;
    u16 controlFlags;
    u8 alpha;
    u8 inventorySlot;
} RadialMenuItem;

typedef struct RadialMenu {
    RadialMenuState state;
    struct RadialMenuContext* context;
    struct {
        RadialMenuItem* elements;
        u8 count;
        u8 capacity;
    } items;
    u8 alpha;
    u8 hoveredItemIndex;
    u16 radius;
    u16 targetRadius;
    s16 x;
    s16 y;
    s16 targetX;
    s16 targetY;
    RadialMenuUpdateFunc update;
    f32 cursorX;
    f32 cursorY;
    u16 prevRadius;
    u8 targetAlpha;
    u8 openRequested: 1;
    u8 acceptingInput: 1;
    u8 shouldDraw: 1;
} RadialMenu;

typedef struct RadialMenuContext {
    RadialMenu* elements[RADIAL_COUNT_MAX];
    u8 count;
    u8* itemIconStatic;
    u8 itemIconStaticLoading: 1;
    u8 itemIconStaticValid: 1;
    u8* itemIcon24Static;
    u8 itemIcon24StaticLoading: 1;
    u8 itemIcon24StaticValid: 1;
} RadialMenuContext;

/* The icon_item_static segment is loaded once and remains available across PlayStates. */
extern u8* gItemIconStatic;
/* The quest icon segment is loaded once and remains available across PlayStates. */
extern u8* gItemIcon24Static;

void RadialMenuContext_Init(RadialMenuContext* this);
u8 RadialMenu_Init(RadialMenuContext* radialMenuCtx, f32 x, f32 y);
u8 RadialMenu_AddItem(RadialMenu* this, void* texture, u8 texFormat, u8 texPixelSize, u8 texWidth, u8 texHeight, RadialMenuItemSelectFunc onSelect, u16 controlFlags, RadialMenuItemPostDrawFunc postDraw, RadialMenuItemPostDrawFunc preDraw);
void RadialMenu_RemoveItem(RadialMenu* this, u8 index);
void RadialMenu_Open(RadialMenu* this, u16 radius);
void RadialMenu_Close(RadialMenu* this);
void RadialMenu_Destroy(RadialMenu* this, RadialMenuContext* radialMenuCtx, u8 handlerIndex);
void RadialMenu_HandleAll(RadialMenuContext* radialMenuCtx, GameState* state);
void RadialMenu_Closed(RadialMenu* this, GameState* state);
void RadialMenu_Opened(RadialMenu* this, GameState* state);
void RadialMenu_Closing(RadialMenu* this, GameState* state);
void RadialMenu_Opening(RadialMenu* this, GameState* state);

#endif
