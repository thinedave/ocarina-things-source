/**
 * @file radial_menu.c
 * @author thinedave
 *
 * This file implements radial menus, a la Twilight Princess.
 */

#include "radial_menu.h"
#include "controller.h"
#include "gfx.h"
#include "gfx_setupdl.h"
#include "assets/objects/gameplay_keep/shopkeeper_controls_tex.h"
#include "assets/textures/parameter_static/parameter_static.h"
#include "dma.h"
#include "item.h"
#include "libc64/malloc.h"
#include "printf.h"
#include "segment_symbols.h"
#include "sys_math.h"
#include "ultra64/gbi.h"
#include "z_lib.h"
#include "z_math.h"
#include "zelda_arena.h"
#include "play_state.h"
#include "libc/assert.h"
#include "game.h"
#include "libu64/gfxprint.h"

#pragma region macros
#define RADIAL_MENU_RADIUS_CLOSED 0
#define RADIAL_MENU_BACKGROUND_INSET 32.0f
#define RADIAL_MENU_STICK_DEADZONE 20.0f
#define CURSOR_SPEED 0.6f
#define MOVE_SPEED 0.25f
#define RADIAL_MENU_ITEM_CAPACITY 32
#define RADIAL_DO_NOTHING

#define BUILD_SIZ_INFO(siz)                                                       \
    [siz] = {                                                                     \
        siz##_LOAD_BLOCK, siz##_INCR, siz##_SHIFT, siz##_BYTES, siz##_LINE_BYTES, \
    }
#pragma endregion

#pragma region statics
u8* gItemIconStatic;
u8* gItemIcon24Static;

static DmaRequest sItemIconStaticDmaRequest;
static OSMesgQueue sItemIconStaticLoadQueue;
static OSMesg sItemIconStaticLoadMsg;
static u8 sItemIconStaticLoading;
static u8 sItemIconStaticValid;
static DmaRequest sItemIcon24StaticDmaRequest;
static OSMesgQueue sItemIcon24StaticLoadQueue;
static OSMesg sItemIcon24StaticLoadMsg;
static u8 sItemIcon24StaticLoading;
static u8 sItemIcon24StaticValid;

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
#pragma endregion

void RadialMenu_ReceiveInput(RadialMenu* this, Input* input);
void RadialMenu_UpdateHoveredItem(RadialMenu* this, Input* input);
void RadialMenu_HandleSelection(RadialMenu* this, Input* input, GameState* state);
void RadialMenu_ProcessItemIconStatic(RadialMenuContext* this);

void RadialMenuContext_Init(RadialMenuContext* this) {
    size_t itemIconStaticSize;
    size_t itemIcon24StaticSize;
    s32 dmaResult;

    this->count = 0;
    this->itemIconStatic = gItemIconStatic;
    this->itemIconStaticLoading = sItemIconStaticLoading;
    this->itemIconStaticValid = sItemIconStaticValid;
    this->itemIcon24Static = gItemIcon24Static;
    this->itemIcon24StaticLoading = sItemIcon24StaticLoading;
    this->itemIcon24StaticValid = sItemIcon24StaticValid;

    if (gItemIconStatic == NULL) {
        itemIconStaticSize = (uintptr_t)_icon_item_staticSegmentRomEnd - (uintptr_t)_icon_item_staticSegmentRomStart;
        gItemIconStatic = SYSTEM_ARENA_MALLOC(itemIconStaticSize, __FILE__, __LINE__);

        ASSERT(gItemIconStatic != NULL, "gItemIconStatic failed to malloc!", __FILE__, __LINE__);

        osCreateMesgQueue(&sItemIconStaticLoadQueue, &sItemIconStaticLoadMsg, 1);
        dmaResult = DMA_REQUEST_ASYNC(&sItemIconStaticDmaRequest, gItemIconStatic,
                                      (uintptr_t)_icon_item_staticSegmentRomStart, itemIconStaticSize, 0,
                                      &sItemIconStaticLoadQueue, NULL, __FILE__, __LINE__);

        if (dmaResult == 0) {
            sItemIconStaticLoading = 1;
        } else {
            SYSTEM_ARENA_FREE(gItemIconStatic, __FILE__, __LINE__);
            gItemIconStatic = NULL;
        }
    }

    if (gItemIcon24Static == NULL) {
        itemIcon24StaticSize = (uintptr_t)_icon_item_24_staticSegmentRomEnd -
                               (uintptr_t)_icon_item_24_staticSegmentRomStart;
        gItemIcon24Static = SYSTEM_ARENA_MALLOC(itemIcon24StaticSize, __FILE__, __LINE__);

        ASSERT(gItemIcon24Static != NULL, "gItemIcon24Static failed to malloc!", __FILE__, __LINE__);

        osCreateMesgQueue(&sItemIcon24StaticLoadQueue, &sItemIcon24StaticLoadMsg, 1);
        dmaResult = DMA_REQUEST_ASYNC(&sItemIcon24StaticDmaRequest, gItemIcon24Static,
                                      (uintptr_t)_icon_item_24_staticSegmentRomStart, itemIcon24StaticSize, 0,
                                      &sItemIcon24StaticLoadQueue, NULL, __FILE__, __LINE__);

        if (dmaResult == 0) {
            sItemIcon24StaticLoading = 1;
        } else {
            SYSTEM_ARENA_FREE(gItemIcon24Static, __FILE__, __LINE__);
            gItemIcon24Static = NULL;
        }
    }

    this->itemIconStatic = gItemIconStatic;
    this->itemIconStaticLoading = sItemIconStaticLoading;
    this->itemIconStaticValid = sItemIconStaticValid;
    this->itemIcon24Static = gItemIcon24Static;
    this->itemIcon24StaticLoading = sItemIcon24StaticLoading;
    this->itemIcon24StaticValid = sItemIcon24StaticValid;
}

u8 RadialMenu_Init(RadialMenuContext* radialMenuCtx, f32 x, f32 y) {
    if (radialMenuCtx->count == RADIAL_COUNT_MAX) {
        return RADIAL_COUNT_MAX;
    }

    RadialMenu* this = ZELDA_ARENA_MALLOC(sizeof(RadialMenu), __FILE__, __LINE__);

    ASSERT(this != NULL, "RadialMenu failed to malloc!", __FILE__, __LINE__);

    this->state = RADIAL_MENU_NOTHING;
    this->context = radialMenuCtx;

    this->items.count = 0;
    this->items.capacity = RADIAL_MENU_ITEM_CAPACITY;
    this->items.elements = ZELDA_ARENA_MALLOC(sizeof(RadialMenuItem) * this->items.capacity, __FILE__, __LINE__);

    ASSERT(this->items.elements != NULL, "RadialMenu->items.elements failed to malloc!", __FILE__, __LINE__);

    this->alpha = 0;
    this->hoveredItemIndex = 0;
    this->radius = RADIAL_MENU_RADIUS_CLOSED;
    this->targetRadius = RADIAL_MENU_RADIUS_CLOSED;
    this->x = x;
    this->y = y;
    this->targetX = x;
    this->targetY = y;
    this->update = RadialMenu_Closed;
    this->cursorX = x;
    this->cursorY = y;
    this->prevRadius = RADIAL_MENU_RADIUS_CLOSED;
    this->targetAlpha = 0;

    this->openRequested = 0;
    this->acceptingInput = 0;
    this->shouldDraw = 0;

    u8 index = radialMenuCtx->count;

    radialMenuCtx->elements[index] = this;
    radialMenuCtx->count++;

    return index;
}

u8 RadialMenu_AddItem(RadialMenu* this, void* texture, u8 texFormat, u8 texPixelSize, u8 texWidth, u8 texHeight, RadialMenuItemSelectFunc onSelect, u16 controlFlags, RadialMenuItemPostDrawFunc postDraw, RadialMenuItemPostDrawFunc preDraw) {
    u8 index = this->items.count;

    if (index >= this->items.capacity) {
        return RADIAL_ITEM_INVALID;
    }

    this->items.count++;

    RadialMenuItem* item = &this->items.elements[index];
    item->state = RADIAL_ITEM_NOTHING;
    item->texture = texture;
    item->texWidth = texWidth;
    item->texHeight = texHeight;
    item->x = 0;
    item->y = 0;
    item->scale = 1.0f;
    item->onSelect = onSelect;
    item->postDraw = postDraw;
    item->preDraw = preDraw;
    item->texFormat = texFormat;
    item->texPixelSize = texPixelSize;
    item->angle = ((s16)(((u32)index * 0x10000) / this->items.count) + 0x3FFF);
    item->controlFlags = controlFlags;
    item->alpha = 255;
    item->inventorySlot = SLOT_NONE;

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

    RadialMenu_ReorderFromIndex(this, index);

    this->items.count--;

    // The array remains at its original capacity so adding and removing items does not fragment ZeldaArena.
}

void RadialMenu_Destroy(RadialMenu* this, RadialMenuContext* radialMenuCtx, u8 handlerIndex) {
    ZELDA_ARENA_FREE(this->items.elements, __FILE__, __LINE__);

    this->items.elements = NULL;

    radialMenuCtx->elements[handlerIndex] = NULL;

    for (u8 i = handlerIndex; i < (radialMenuCtx->count - 1); i++) {
        radialMenuCtx->elements[i] = radialMenuCtx->elements[i + 1];
    }

    radialMenuCtx->count--;

    ZELDA_ARENA_FREE(this, __FILE__, __LINE__);
}

void RadialMenu_Open(RadialMenu* this, u16 radius) {
    this->prevRadius = this->radius;
    this->targetRadius = radius;
    this->targetAlpha = 255;
    this->acceptingInput = 1;
    this->shouldDraw = 1;

    if (this->context->itemIconStaticValid) {
        this->update = RadialMenu_Opening;
    } else if (this->context->itemIconStaticLoading) {
        this->openRequested = 1;
    }
}

void RadialMenu_Close(RadialMenu* this) {
    this->targetRadius = 0;
    this->targetAlpha = 0;
    this->openRequested = 0;
    this->acceptingInput = 0;
    this->update = RadialMenu_Closing;
}

void RadialMenu_MoveToTarget(RadialMenu* this) {
    this->x = LERP(this->x, this->targetX, MOVE_SPEED);
    this->y = LERP(this->y, this->targetY, MOVE_SPEED);
}

s32 RadialMenu_Update(RadialMenu* this, GameState* state, RadialMenuContext* radialMenuCtx, u8 handlerIndex) {
    RadialMenu_MoveToTarget(this);

    if (this->openRequested && radialMenuCtx->itemIconStaticValid) {
        this->openRequested = 0;
        this->update = RadialMenu_Opening;
    }

    this->update(this, state);

    RadialMenu_UpdateHoveredItem(this, &state->input[0]);

    if (this->state & RADIAL_MENU_DESTROY) {
        RadialMenu_Destroy(this, radialMenuCtx, handlerIndex);

        return true;
    }

    RadialMenu_HandleSelection(this, &state->input[0], state);

    return false;
}

void RadialMenu_ProcessItemIconStatic(RadialMenuContext* this) {
    if (sItemIconStaticLoading && osRecvMesg(&sItemIconStaticLoadQueue, NULL, OS_MESG_NOBLOCK) == 0) {
        sItemIconStaticLoading = 0;
        sItemIconStaticValid = (gItemIconStatic != NULL);
    }

    if (sItemIcon24StaticLoading && osRecvMesg(&sItemIcon24StaticLoadQueue, NULL, OS_MESG_NOBLOCK) == 0) {
        sItemIcon24StaticLoading = 0;
        sItemIcon24StaticValid = (gItemIcon24Static != NULL);
    }

    this->itemIconStatic = gItemIconStatic;
    this->itemIconStaticLoading = sItemIconStaticLoading;
    this->itemIconStaticValid = sItemIconStaticValid;
    this->itemIcon24Static = gItemIcon24Static;
    this->itemIcon24StaticLoading = sItemIcon24StaticLoading;
    this->itemIcon24StaticValid = sItemIcon24StaticValid;
}

void RadialMenu_DrawBackground(RadialMenu* this, GameState* state, Gfx** gfxP) {
    if (this->radius <= RADIAL_MENU_BACKGROUND_INSET) {
        return;
    }

    OPEN_DISPS(state->gfxCtx, __FILE__, __LINE__);

    Gfx* gfx = *gfxP;

    gDPPipeSync(gfx++);

    gDPSetCombineMode(gfx++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    u8 alpha = LERP(0, 100, (f32)this->alpha / 255.0f);

    gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, alpha);
    gDPSetEnvColor(gfx++, 0, 0, 0, alpha);

    u16 radius = (this->radius - RADIAL_MENU_BACKGROUND_INSET);
    u16 halfRadius = (radius / 2);
    u16 radiusST = ((64.0f / (f32)radius) * 1024.0f);

    // Top left
    gDPLoadTextureBlock(gfx++, gRadialMenuBackgroundTex, G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0,
                        G_TX_MIRROR | G_TX_WRAP, G_TX_MIRROR | G_TX_WRAP, 5, 5, G_TX_NOLOD, G_TX_NOLOD);

    gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, 0, (32 - 1) << 2, (32 - 1) << 2);

    gSPTextureRectangle(gfx++, ((this->x - halfRadius) << 2), ((this->y - halfRadius) << 2),
                        ((this->x + halfRadius) << 2), ((this->y + halfRadius) << 2), G_TX_RENDERTILE, 0, 0, radiusST,
                        radiusST);

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

    gDPLoadTextureBlock(gfx++, gControlStickTex, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 16, 0, G_TX_NOMIRROR | G_TX_WRAP,
                        G_TX_NOMIRROR | G_TX_WRAP, 4, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSPTextureRectangle(gfx++, ((s16)(this->cursorX - 8) << 2), ((s16)(this->cursorY - 8) << 2), ((s16)(this->cursorX + 8) << 2),
                        ((s16)(this->cursorY + 8) << 2), G_TX_RENDERTILE, 0, 0, (1 << 10), (1 << 10));

    *gfxP = gfx;

    CLOSE_DISPS(state->gfxCtx, __FILE__, __LINE__);
}

void RadialMenu_DrawItem(RadialMenu* this, GameState* state, RadialMenuItem* item, s16 x, s16 y, u8 index, Gfx** gfxP) {
    OPEN_DISPS(state->gfxCtx, __FILE__, __LINE__);

    Gfx* gfx = *gfxP;

    gDPPipeSync(gfx++);

    gDPSetCombineMode(gfx++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    u8 alpha = (item->alpha * (this->alpha / 255.0f));

    gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, alpha);
    gDPSetEnvColor(gfx++, 255, 255, 255, alpha);

    gDPSetTextureImage(gfx++, item->texFormat, sImgSizInfo[item->texPixelSize].loadBlock, 1, item->texture);

    gDPSetTile(gfx++, item->texFormat, sImgSizInfo[item->texPixelSize].loadBlock, 0, 0, G_TX_LOADTILE, 0,
               G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
    gDPLoadSync(gfx++);

    gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0,
                 (((item->texWidth) * (item->texHeight) + sImgSizInfo[item->texPixelSize].incr) >>
                  sImgSizInfo[item->texPixelSize].shift) -
                     1,
                 CALC_DXT(item->texWidth, sImgSizInfo[item->texPixelSize].bytes));
    gDPPipeSync(gfx++);

    gDPSetTile(gfx++, item->texFormat, item->texPixelSize,
               (((item->texWidth) * sImgSizInfo[item->texPixelSize].lineBytes) + 7) >> 3, 0, G_TX_RENDERTILE, 0,
               G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);

    gDPSetTileSize(gfx++, G_TX_RENDERTILE, 0, 0, ((item->texWidth) - 1) << G_TEXTURE_IMAGE_FRAC,
                   ((item->texHeight) - 1) << G_TEXTURE_IMAGE_FRAC);

    f32 scale = (this->hoveredItemIndex == index ? 1.0f : 0.7f);
    f32 sizeX = (item->texWidth * scale);
    f32 sizeY = (item->texHeight * scale);

    u16 radiusS = ((item->texWidth / sizeX) * 1024.0f);
    u16 radiusT = ((item->texHeight / sizeY) * 1024.0f);

    u8 halfWidth = ((u8)sizeX / 2);
    u8 halfHeight = ((u8)sizeY / 2);

    gSPTextureRectangle(gfx++, ((x - halfWidth) << 2), ((y - halfHeight) << 2), ((x + halfWidth) << 2),
                        ((y + halfHeight) << 2), G_TX_RENDERTILE, 0, 0, radiusS, radiusT);

    *gfxP = gfx;

    CLOSE_DISPS(state->gfxCtx, __FILE__, __LINE__);
}

void RadialMenu_DrawItems(RadialMenu* this, GameState* state, Gfx** gfxP) {
    if (this->items.count == 0) {
        return;
    }

    f32 radius = (this->radius * 0.5f);

    for (u8 i = 0; i < this->items.count; i++) {
        RadialMenuItem* item = &this->items.elements[i];
        if (item->texture == NULL) {
            continue;
        }

        s16 targetAngle = (s16)(((u32)i * 0x10000) / this->items.count);

        item->angle = BINANG_LERPIMP(item->angle, targetAngle, 0.4f);
        item->x = (this->x + (Math_SinS(item->angle) * radius));
        item->y = (this->y + (Math_CosS(item->angle) * radius));

        if (item->preDraw != NULL) {
            item->preDraw(this, item, state, gfxP);
        }

        RadialMenu_DrawItem(this, state, item, item->x, item->y, i, gfxP);

        if (item->postDraw != NULL) {
            item->postDraw(this, item, state, gfxP);
        }
    }
}

void RadialMenu_DrawDebug(RadialMenu* this, GameState* state, Gfx** gfxP) {
    GfxPrint printer;

    GfxPrint_Init(&printer);
    GfxPrint_Open(&printer, *gfxP);

    GfxPrint_SetColor(&printer, 255, 155, 255, 255);
    GfxPrint_SetPos(&printer, 2, 10);
    GfxPrint_Printf(&printer, "TR=%i", this->targetRadius);

    *gfxP = GfxPrint_Close(&printer);
    GfxPrint_Destroy(&printer);
}

void RadialMenu_Draw(RadialMenu* this, GameState* state, u8* handlerIndex) {
    if (this->radius == 0 || this->x < -(this->radius + 32) || this->x > (SCREEN_WIDTH + this->radius + 32)) {
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

    //RadialMenu_DrawDebug(this, state, &gfx);

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

    if (this->radius == this->targetRadius) {
        this->update = RadialMenu_Opened;
    }
}

void RadialMenu_Closing(RadialMenu* this, GameState* state) {
    RadialMenu_ReceiveInput(this, &state->input[0]);
    RadialMenu_ProgressRadius(this);

    if (this->radius == this->targetRadius) {
        this->shouldDraw = 0;
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
    if (!this->acceptingInput) {
        this->cursorX = this->x;
        this->cursorY = this->y;
        return;
    }

    f32 stickX = input->cur.stick_x;
    f32 stickY = input->cur.stick_y;

    // PRINTF("stickX=%.2f\nstickY=%.2f\n", stickX, stickY);
    // PRINTF("fracX=%.2f\nfracY=%.2f\n", ((stickX + 85.0f) / 170.0f), ((stickY + 85.0f) / 170.0f));

    s16 cursorRadius = (this->radius / 2);

    this->cursorX = LERP(this->cursorX, this->x + (LERP((cursorRadius * -1), cursorRadius, ((stickX + 85.0f) / 170.0f))), CURSOR_SPEED);
    this->cursorY = LERP(this->cursorY, this->y - (LERP((cursorRadius * -1), cursorRadius, ((stickY + 85.0f) / 170.0f))), CURSOR_SPEED);

    // PRINTF("cursorX=%i\ncursorY=%i\n", this->cursorX, this->cursorY);
}

void RadialMenu_UpdateHoveredItem(RadialMenu* this, Input* input) {
    if (this->items.count == 0 || !this->acceptingInput) {
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

u16 sRadialItemButtonTable[RADIAL_ITEM_CONTROL_COUNT] = {
    BTN_A,     BTN_B,      BTN_R,   BTN_L,     BTN_Z,     BTN_CUP,    BTN_CDOWN,
    BTN_CLEFT, BTN_CRIGHT, BTN_DUP, BTN_DDOWN, BTN_DLEFT, BTN_DRIGHT,
};

void RadialMenu_HandleSelection(RadialMenu* this, Input* input, GameState* state) {
    if (this->hoveredItemIndex == RADIAL_ITEM_INVALID || !this->acceptingInput) {
        return;
    }

    RadialMenuItem* item = &this->items.elements[this->hoveredItemIndex];

    if (item->onSelect == NULL || item->controlFlags == 0) {
        return;
    }

    for (u8 i = 0; i < RADIAL_ITEM_CONTROL_COUNT; i++) {
        if (item->controlFlags & sRadialItemButtonTable[i] &&
            CHECK_BTN_ANY(input->press.button, sRadialItemButtonTable[i])) {
            item->onSelect(sRadialItemButtonTable[i], this, item, state);

            break;
        }
    }
}

void RadialMenu_HandleAll(RadialMenuContext* radialMenuCtx, GameState* state) {
    RadialMenu_ProcessItemIconStatic(radialMenuCtx);

    for (u8 i = 0; i < radialMenuCtx->count;) {
        if (radialMenuCtx->elements[i] == NULL) {
            break;
        }

        if (radialMenuCtx->elements[i]->shouldDraw) {
            RadialMenu_Draw(radialMenuCtx->elements[i], state, &i);
        }

        if (!RadialMenu_Update(radialMenuCtx->elements[i], state, radialMenuCtx, i)) {
            i++;
        }
    }
}
