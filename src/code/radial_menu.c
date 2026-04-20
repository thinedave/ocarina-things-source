/**
 * @file radial_menu.c
 * @author thinedave
 *
 * This file implements radial menus, a la Twilight Princess.
 */

#include "radial_menu.h"
#include "gfx.h"
#include "gfx_setupdl.h"
#include "assets/objects/gameplay_keep/shopkeeper_controls_tex.h"
#include "assets/textures/parameter_static/parameter_static.h"
#include "printf.h"
#include "sys_math.h"
#include "ultra64/gbi.h"
#include "z_lib.h"
#include "z_math.h"
#include "zelda_arena.h"
#include "play_state.h"
#include "libc/assert.h"
#include "game.h"

// This doesn't already exist for some reason
#ifdef DEBUG_FEATURES
#define ZELDA_ARENA_REALLOC(ptr, size, file, line) ZeldaArena_ReallocDebug(ptr, size, file, line)
#else
#define ZELDA_ARENA_REALLOC(ptr, size, file, line) ZeldaArena_Realloc(ptr, size)
#endif

#define RADIAL_MENU_RADIUS_CLOSED 0
#define RADIAL_MENU_STICK_DEADZONE 20.0f
#define RADIAL_ITEM_INVALID UINT8_MAX

#define RADIAL_MENU_REALLOC_ITEMS(this) ZELDA_ARENA_REALLOC((this)->items.elements, sizeof(RadialMenuItem) * (this)->items.count, __FILE__, __LINE__)
#define RADIAL_MENU_SYNC_TEX(item) DMA_REQUEST_SYNC((item)->texSegment, (uintptr_t)(item)->texture, (item)->texLen, __FILE__, __LINE__)

#define RADIAL_DO_NOTHING

void RadialMenu_Closed(RadialMenu* this, GameState* state);
void RadialMenu_Opened(RadialMenu* this, GameState* state);
void RadialMenu_Closing(RadialMenu* this, GameState* state);
void RadialMenu_Opening(RadialMenu* this, GameState* state);

void RadialMenu_ReceiveInput(RadialMenu* this, Input* input);
void RadialMenu_UpdateHoveredItem(RadialMenu* this, Input* input);

u8 RadialMenu_Init(RadialMenuContext* radialMenuCtx, s16 x, s16 y) {
    if (radialMenuCtx->count == RADIAL_COUNT_MAX) {
        return RADIAL_COUNT_MAX;
    }

    RadialMenu* this = ZELDA_ARENA_MALLOC(sizeof(RadialMenu), __FILE__, __LINE__);

    ASSERT(this != NULL, "RadialMenu failed to malloc!", __FILE__, __LINE__);

    this->state = RADIAL_MENU_NOTHING;

    this->items.count = 0;
    this->items.elements = ZELDA_ARENA_MALLOC(sizeof(RadialMenuItem), __FILE__, __LINE__);

    ASSERT(this->items.elements != NULL, "RadialMenu->items.elements failed to malloc!", __FILE__, __LINE__);

    this->alpha = 0;
    this->hoveredItemIndex = 0;
    this->radius = RADIAL_MENU_RADIUS_CLOSED;
    this->targetRadius = RADIAL_MENU_RADIUS_CLOSED;
    this->x = x;
    this->y = y;
    this->update = RadialMenu_Closed;
    this->cursorX = x;
    this->cursorY = y;
    this->progress = 0.0f;
    this->prevRadius = RADIAL_MENU_RADIUS_CLOSED;
    this->targetAlpha = 0;

    u8 index = radialMenuCtx->count;

    radialMenuCtx->elements[index] = this;
    radialMenuCtx->count++;

    return index;
}

u8 RadialMenu_AddItem(RadialMenu* this, void* texture, u8 texFormat, u8 texPixelSize, u8 texWidth, u8 texHeight, RadialMenuItemSelectFunc onSelect) {
    u8 index = this->items.count;

    if (index >= (RADIAL_ITEM_INVALID - 1)) {
        return RADIAL_ITEM_INVALID;
    }

    this->items.count++;

    this->items.elements = RADIAL_MENU_REALLOC_ITEMS(this);

    ASSERT(this->items.elements != NULL, "RadialMenu->items.elements failed to realloc!", __FILE__, __LINE__);

    RadialMenuItem* item = &this->items.elements[index];
    item->state = RADIAL_ITEM_NOTHING;
    item->texture = texture;
    item->texWidth = texWidth;
    item->texHeight = texHeight;
    item->x = 0;
    item->y = 0;
    item->scale = 1.0f;
    item->onSelect = onSelect;
    item->texSegment = NULL;
    item->texLen = 0;
    item->texFormat = texFormat;
    item->texPixelSize = texPixelSize;
    item->angle = ((s16)(((u32)index * 0x10000) / this->items.count) + 0x3FFF);

    return index;
}

u8 RadialMenu_AddItemSync(RadialMenu* this, uintptr_t textureVrom, u8 texFormat, u8 texPixelSize, u8 texWidth, u8 texHeight, RadialMenuItemSelectFunc onSelect, size_t texLen) {
    u8 index = RadialMenu_AddItem(this, (void*)textureVrom, texFormat, texPixelSize, texWidth, texHeight, onSelect);

    if (index == RADIAL_ITEM_INVALID) {
        return RADIAL_ITEM_INVALID;
    }

    RadialMenuItem* item = &this->items.elements[index];

    item->texLen = texLen;

    item->texSegment = ZELDA_ARENA_MALLOC(item->texLen, __FILE__, __LINE__);

    ASSERT(item->texSegment != NULL, "RadialItem->texSegment failed to malloc!", __FILE__, __LINE__);

    DMA_REQUEST_SYNC(item->texSegment, (uintptr_t)item->texture, item->texLen, __FILE__, __LINE__);

    item->state |= RADIAL_ITEM_TEX_SYNCED;

    return index;
}

void RadialMenu_ReorderFromIndex(RadialMenu* this, u8 index) {
    for (u8 i = index; i < (this->items.count - 1); i++) {
        this->items.elements[i] = this->items.elements[i + 1];
    }
}

void RadialMenu_RemoveItem(RadialMenu* this, u8 index) {
    if (index >= this->items.count) {
        return;
    }

    if (this->items.elements[index].state & RADIAL_ITEM_TEX_SYNCED) {
        ZELDA_ARENA_FREE(this->items.elements[index].texSegment, __FILE__, __LINE__);
        this->items.elements[index].texSegment = NULL;
    }

    RadialMenu_ReorderFromIndex(this, index);
    
    this->items.count--;

    this->items.elements = RADIAL_MENU_REALLOC_ITEMS(this);

    ASSERT(this->items.elements != NULL, "RadialMenu->items.elements failed to realloc!", __FILE__, __LINE__);

    // We do not need to bother nullifying or zeroing out any data because it cannot be accessed from here on and will be overwritten if another item is added
}

void RadialMenu_Destroy(RadialMenu* this, RadialMenuContext* radialMenuCtx, u8* handlerIndex) {
    for (u8 i = 0; i < this->items.count; i++) {
        ZELDA_ARENA_FREE(this->items.elements[i].texSegment, __FILE__, __LINE__);

        this->items.elements[i].texSegment = NULL;
    }

    ZELDA_ARENA_FREE(this->items.elements, __FILE__, __LINE__);

    this->items.elements = NULL;

    if (handlerIndex != NULL) {
        radialMenuCtx->elements[*handlerIndex] = NULL;

        for (u8 i = *handlerIndex; i < (radialMenuCtx->count - 1); i++) {
            radialMenuCtx->elements[i] = radialMenuCtx->elements[i + 1];
        }

        handlerIndex--;
    }

    ZELDA_ARENA_FREE(this, __FILE__, __LINE__);

    this = NULL;
}

void RadialMenu_Open(RadialMenu* this, u16 radius) {
    this->prevRadius = this->radius;
    this->targetRadius = radius;
    this->targetAlpha = 255;
    this->update = RadialMenu_Opening;
    this->progress = 0.0f;
}

void RadialMenu_Close(RadialMenu* this) {
    this->targetRadius = 0;
    this->targetAlpha = 0;
    this->update = RadialMenu_Closing;
    this->progress = 0.0f;
}

void RadialMenu_Update(RadialMenu* this, GameState* state, RadialMenuContext* radialMenuCtx, u8* handlerIndex) {
    this->update(this, state);

    RadialMenu_UpdateHoveredItem(this, &state->input[0]);

    if (this->state & RADIAL_MENU_DESTROY) {
        RadialMenu_Destroy(this, radialMenuCtx, handlerIndex);
    }
}

void RadialMenu_DrawBackground(RadialMenu* this, GameState* state, Gfx** gfxP) {
    if (this->radius == 0) {
        return;
    }

    OPEN_DISPS(state->gfxCtx, __FILE__, __LINE__);

    Gfx* gfx = *gfxP;

    gDPPipeSync(gfx++);

    gDPSetCombineMode(gfx++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    u8 alpha = LERP(0, 100, (f32)this->alpha / 255.0f);

    gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, alpha);
    gDPSetEnvColor(gfx++, 0, 0, 0, alpha);

    u16 halfRadius = (this->radius / 2);
    u16 radiusST = ((64.0f / (f32)this->radius) * 1024.0f);

    // Top left
    gDPLoadTextureBlock(gfx++, gRadialMenuBackgroundTex, G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0,
        G_TX_MIRROR | G_TX_WRAP, G_TX_MIRROR | G_TX_WRAP,
        5, 5, G_TX_NOLOD, G_TX_NOLOD);

    gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, 0, (32 - 1) << 2, (32 - 1) << 2);

    gSPTextureRectangle(gfx++, ((this->x - halfRadius) << 2), ((this->y - halfRadius) << 2), ((this->x + halfRadius) << 2), ((this->y + halfRadius) << 2),
        G_TX_RENDERTILE, 0, 0, radiusST, radiusST);

    *gfxP = gfx;

    CLOSE_DISPS(state->gfxCtx, __FILE__, __LINE__);
}

void RadialMenu_DrawCursor(RadialMenu* this, GameState* state, Gfx** gfxP) {
    OPEN_DISPS(state->gfxCtx, __FILE__, __LINE__);

    Gfx* gfx = *gfxP;

    gDPPipeSync(gfx++);

    gDPSetCombineMode(gfx++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, this->alpha);
    gDPSetEnvColor(gfx++, 255, 255, 255, this->alpha);

    gDPLoadTextureBlock(gfx++, gControlStickTex, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 16, 0,
        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 4, G_TX_NOMASK, G_TX_NOLOD,
        G_TX_NOLOD);

    gSPTextureRectangle(gfx++, ((this->cursorX - 8) << 2), ((this->cursorY - 8) << 2), ((this->cursorX + 8) << 2), ((this->cursorY + 8) << 2),
        G_TX_RENDERTILE, 0, 0, (1 << 10), (1 << 10));

    *gfxP = gfx;

    CLOSE_DISPS(state->gfxCtx, __FILE__, __LINE__);
}

#define BUILD_SIZ_INFO(siz) \
[siz] = {   \
    siz##_LOAD_BLOCK,   \
    siz##_INCR, \
    siz##_SHIFT,    \
    siz##_BYTES,    \
    siz##_LINE_BYTES,    \
}

static struct {
    u8 loadBlock;
    u8 incr;
    u8 shift;
    u8 bytes;
    u8 lineBytes;
} sImgSizInfo[] = {
    BUILD_SIZ_INFO(G_IM_SIZ_4b),
    BUILD_SIZ_INFO(G_IM_SIZ_8b),
    BUILD_SIZ_INFO(G_IM_SIZ_16b),
    BUILD_SIZ_INFO(G_IM_SIZ_32b),
};

void RadialMenu_DrawItem(RadialMenu* this, GameState* state, RadialMenuItem* item, s16 x, s16 y, u8 index, Gfx** gfxP) {
    OPEN_DISPS(state->gfxCtx, __FILE__, __LINE__);

    Gfx* gfx = *gfxP;

    gDPPipeSync(gfx++);

    gDPSetCombineMode(gfx++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, this->alpha);
    gDPSetEnvColor(gfx++, 255, 255, 255, this->alpha);

    void* texture = item->texture;

    if (item->state & RADIAL_ITEM_TEX_SYNCED) {
        texture = item->texSegment;
    }

    gDPSetTextureImage(gfx++, item->texFormat, sImgSizInfo[item->texPixelSize].loadBlock, 1, texture);

    gDPSetTile(gfx++, item->texFormat, sImgSizInfo[item->texPixelSize].loadBlock, 0, 0, G_TX_LOADTILE,
        0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
        G_TX_NOLOD);
    gDPLoadSync(gfx++);

    gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0,
        (((item->texWidth) * (item->texHeight) + sImgSizInfo[item->texPixelSize].incr) >> sImgSizInfo[item->texPixelSize].shift) -1,
        CALC_DXT(item->texWidth, sImgSizInfo[item->texPixelSize].bytes));
    gDPPipeSync(gfx++);

    gDPSetTile(gfx++, item->texFormat, item->texPixelSize,
        (((item->texWidth) * sImgSizInfo[item->texPixelSize].lineBytes) + 7) >> 3, 0,
        G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP,
        G_TX_NOMASK, G_TX_NOLOD);

    gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, 0,                              
        ((item->texWidth)  - 1) << G_TEXTURE_IMAGE_FRAC,
        ((item->texHeight) - 1) << G_TEXTURE_IMAGE_FRAC);

    f32 scale = (this->hoveredItemIndex == index ? 1.0f : 0.7f);
    f32 sizeX = (item->texWidth * scale);
    f32 sizeY = (item->texHeight * scale);

    u16 radiusS = ((item->texWidth / sizeX) * 1024.0f);
    u16 radiusT = ((item->texHeight / sizeY) * 1024.0f);

    u8 halfWidth = ((u8)sizeX / 2);
    u8 halfHeight = ((u8)sizeY / 2);

    gSPTextureRectangle(gfx++, ((x - halfWidth) << 2), ((y - halfHeight) << 2), ((x + halfWidth) << 2), ((y + halfHeight) << 2),
        G_TX_RENDERTILE, 0, 0, radiusS, radiusT);

    *gfxP = gfx;

    CLOSE_DISPS(state->gfxCtx, __FILE__, __LINE__);
}

void RadialMenu_DrawItems(RadialMenu* this, GameState* state, Gfx** gfxP) {
    if (this->items.count == 0) {
        return;
    }

    for (u8 i = 0; i < this->items.count; i++) {
        RadialMenuItem* item = &this->items.elements[i];
        s16 targetAngle = (s16)(((u32)i * 0x10000) / this->items.count);

        item->angle = BINANG_LERPIMP(item->angle, targetAngle, 0.1f);

        s16 x = (this->x + (Math_SinS(item->angle) * (this->radius - 32)));
        s16 y = (this->y + (Math_CosS(item->angle) * (this->radius - 32)));

        RadialMenu_DrawItem(this, state, item, x, y, i, gfxP);
    }
}

void RadialMenu_Draw(RadialMenu* this, GameState* state, u8* handlerIndex) {
    if (this->radius == 0) {
        return;
    }

    OPEN_DISPS(state->gfxCtx, __FILE__, __LINE__);

    Gfx* gfx = (POLY_OPA_DISP + 1);

    gSPDisplayList(OVERLAY_DISP++, gfx);

    Gfx_SetupDL_39Ptr(&gfx);

    gDPSetAlphaCompare(gfx++, G_AC_NONE);

    RadialMenu_DrawBackground(this, state, &gfx);

    RadialMenu_DrawItems(this, state, &gfx);

    RadialMenu_DrawCursor(this, state, &gfx);

    gSPEndDisplayList(gfx++);
    gSPBranchList(POLY_OPA_DISP, gfx);
    POLY_OPA_DISP = gfx;

    CLOSE_DISPS(state->gfxCtx, __FILE__, __LINE__);
}

void RadialMenu_ProgressRadius(RadialMenu* this) {
    this->radius = LERP(this->radius, this->targetRadius, 0.3);
    this->alpha = LERP(this->alpha, this->targetAlpha, 0.5);
}

void RadialMenu_Opening(RadialMenu* this, GameState* state) {
    RadialMenu_ReceiveInput(this, &state->input[0]);
    RadialMenu_ProgressRadius(this);

    if (this->progress == 1.0f) {
        this->update = RadialMenu_Opened;
    }
}

void RadialMenu_Closing(RadialMenu* this, GameState* state) {
    RadialMenu_ReceiveInput(this, &state->input[0]);
    RadialMenu_ProgressRadius(this);

    if (this->progress == 1.0f) {
        this->update = RadialMenu_Closed;
    }
}

void RadialMenu_Closed(RadialMenu* this, GameState* state) {
    RADIAL_DO_NOTHING;
}

void RadialMenu_Opened(RadialMenu* this, GameState* state) {
    RadialMenu_ReceiveInput(this, &state->input[0]);
}

void RadialMenu_ReceiveInput(RadialMenu* this, Input* input) {
    f32 stickX = input->cur.stick_x;
    f32 stickY = input->cur.stick_y;

    //PRINTF("stickX=%.2f\nstickY=%.2f\n", stickX, stickY);
    //PRINTF("fracX=%.2f\nfracY=%.2f\n", ((stickX + 85.0f) / 170.0f), ((stickY + 85.0f) / 170.0f));

    s16 cursorRadius = (this->radius / 2);

    this->cursorX = this->x + (LERP((cursorRadius * -1), cursorRadius, ((stickX + 85.0f) / 170.0f)));
    this->cursorY = this->y - (LERP((cursorRadius * -1), cursorRadius, ((stickY + 85.0f) / 170.0f)));

    //PRINTF("cursorX=%i\ncursorY=%i\n", this->cursorX, this->cursorY);
}

void RadialMenu_UpdateHoveredItem(RadialMenu* this, Input* input) {
    if (this->items.count == 0) {
        this->hoveredItemIndex = RADIAL_ITEM_INVALID;

        return;
    }

    f32 stickX = input->cur.stick_x;
    f32 stickY = input->cur.stick_y;
    f32 stickMagnitudeSq = SQ(stickX) + SQ(stickY);

    if (stickMagnitudeSq < SQ(RADIAL_MENU_STICK_DEADZONE)) {
        this->hoveredItemIndex = RADIAL_ITEM_INVALID;

        return;
    }

    f32 slotSize = (0x10000 / this->items.count);
    s16 angle = Math_Atan2S(stickX, stickY) + 0x3FFF;
    u8 index = ((u16)(angle + (slotSize / 2)) / slotSize);

    if (index >= this->items.count) {
        index = 0;
    }

    this->hoveredItemIndex = index;
}

void RadialMenu_HandleAll(RadialMenuContext* radialMenuCtx, GameState* state) {
    for (u8 i = 0; i < radialMenuCtx->count; i++) {
        if (radialMenuCtx->elements[i] == NULL) {
            break;
        }

        RadialMenu_Draw(radialMenuCtx->elements[i], state, &i);
        RadialMenu_Update(radialMenuCtx->elements[i], state, radialMenuCtx, &i);
    }
}