#include "array_count.h"
#include "game.h"
#include "libu64/gfxprint.h"
#include "sfx.h"
#include "gfx.h"
#include "view.h"
#include "gfx_setupdl.h"
#include "controller.h"
#include "audiomgr.h"
#include "regs.h"
#include "sys_matrix.h"
#include "libc64/math64.h"
#include "sfxlist_state.h"
#include "z_lib.h"
#include "alloca.h"

#ifndef OPEN_DISPS_AUTO
#define OPEN_DISPS_AUTO(game) OPEN_DISPS((game).gfxCtx, __FILE__, __LINE__)
#endif

#ifndef CLOSE_DISPS_AUTO
#define CLOSE_DISPS_AUTO(game) CLOSE_DISPS((game).gfxCtx, __FILE__, __LINE__)
#endif

static bool sDrawSFXDebug = false;
static s32 sSfxIndex = 0;
static u8 sSfxPerPage = 25;
static s16 sSfxPage = 0;
static bool sFilledSFXList = false;
static u8 sSfxFillIndex = 0;
static f32 sSfxFreq = 1.0f;
static f32 sSfxVol = 1.0f;
static s8 sSfxReverb = 0;
static s8 sSfxOption = 0;

typedef struct SfxEntry {
    char* name;
    SfxId id;
} SfxEntry;

#define DEFINE_SFX(_0, enum, _2, _3, _4, _5) {#enum, enum},

static SfxEntry sSfxList[] = {
    #include "tables/sfx/playerbank_table.h"
    #include "tables/sfx/itembank_table.h"
    #include "tables/sfx/environmentbank_table.h"
    #include "tables/sfx/enemybank_table.h"
    #include "tables/sfx/systembank_table.h"
    #include "tables/sfx/ocarinabank_table.h"
    #include "tables/sfx/voicebank_table.h"
};

static u32 sSfxCount = ARRAY_COUNTU(sSfxList);
static u32 sSfxMaxPage;

#undef DEFINE_SFX

void SFXList_Draw(SFXListState* this) {
    GraphicsContext* gfxCtx = this->state.gfxCtx;
    GfxPrint printer;

    OPEN_DISPS_AUTO(this->state);

    gSPSegment(POLY_OPA_DISP++, 0x00, NULL);
    Gfx_SetupFrame(gfxCtx, 0, 0, 0);
    SET_FULLSCREEN_VIEWPORT(&this->view);
    View_Apply(&this->view, VIEW_ALL);
    Gfx_SetupDL_28Opa(gfxCtx);

    GfxPrint_Init(&printer);
    GfxPrint_Open(&printer, POLY_OPA_DISP);

    GfxPrint_SetPos(&printer, 2, 1);
    GfxPrint_SetColor(&printer, 255, 255, (sSfxOption == 0 ? 0 : 255), 255);
    GfxPrint_Printf(&printer, "FREQ: %.1ff", sSfxFreq, sSfxPage + 1, sSfxMaxPage);

    GfxPrint_SetPos(&printer, 2, 1);
    GfxPrint_SetColor(&printer, 255, 255, (sSfxOption == 1 ? 0 : 255), 255);
    GfxPrint_Printf(&printer, "           VOL: %.1ff", sSfxVol, sSfxPage + 1, sSfxMaxPage);

    GfxPrint_SetPos(&printer, 2, 1);
    GfxPrint_SetColor(&printer, 255, 255, (sSfxOption == 2 ? 0 : 255), 255);
    GfxPrint_Printf(&printer, "                     REVERB: %i", sSfxReverb, sSfxPage + 1, sSfxMaxPage);

    GfxPrint_SetPos(&printer, 2, 2);
    GfxPrint_SetColor(&printer, 255, 255, 255, 255);
    GfxPrint_Printf(&printer, "SHOWING %i   PAGE %i OF %i", sSfxPerPage, sSfxPage + 1, sSfxMaxPage);

    for (u16 i = 0; i < sSfxPerPage; i++) {
        u16 realIndex = ((sSfxPage * sSfxPerPage) + i);

        if (realIndex > (sSfxCount - 1)) {
            continue;
        }

        GfxPrint_SetColor(&printer, 255, 255, (realIndex == sSfxIndex ? 0 : 255), 255);
        GfxPrint_SetPos(&printer, 2, 3 + i);
        GfxPrint_Printf(&printer, "%s", sSfxList[realIndex].name);
    }

    POLY_OPA_DISP = GfxPrint_Close(&printer);
    GfxPrint_Destroy(&printer);

    CLOSE_DISPS_AUTO(this->state);
}

void SFXList_Main(GameState* thisx) {
    SFXListState* this = (SFXListState*)thisx;

    SFXList_Draw(this);

    if (CHECK_BTN_ALL(this->state.input[0].press.button, BTN_DDOWN) || this->state.input[0].rel.stick_y <= -30) {
        sSfxIndex = MIN(sSfxIndex + 1, sSfxCount + 1);

        u8 fakeIndex = sSfxIndex % sSfxPerPage;
        if (sSfxIndex > (sSfxCount - 1) || (fakeIndex == 0 && sSfxIndex > 0)) {
            sSfxPage = MIN(sSfxPage + 1, sSfxMaxPage);
            if (sSfxPage >= sSfxMaxPage) {
                sSfxPage = 0;
                sSfxIndex = 0;
            }
        }
    } else if (CHECK_BTN_ALL(this->state.input[0].press.button, BTN_DUP) || this->state.input[0].rel.stick_y >= 30) {
        sSfxIndex = MAX(sSfxIndex - 1, -1);

        if (sSfxIndex < (sSfxPage * sSfxPerPage)) {
            sSfxPage = MAX(sSfxPage - 1, -1);
            if (sSfxPage < 0) {
                sSfxPage = sSfxMaxPage - 1;
                sSfxIndex = sSfxCount - 1;
            }
        }
    } else if (CHECK_BTN_ALL(this->state.input[0].press.button, BTN_DRIGHT) || this->state.input[0].rel.stick_x >= 30) {
        sSfxPage = MIN(sSfxPage + 1, sSfxMaxPage + 1);
        sSfxIndex += sSfxPerPage;

        if (sSfxPage > (sSfxMaxPage - 1)) {
            sSfxPage = 0;
            sSfxIndex = 0;
        }
    } else if (CHECK_BTN_ALL(this->state.input[0].press.button, BTN_DLEFT) || this->state.input[0].rel.stick_x <= -30) {
        sSfxPage = MAX(sSfxPage - 1, -1);
        sSfxIndex -= sSfxPerPage;

        if (sSfxPage < 0) {
            sSfxPage = sSfxMaxPage - 1;
            sSfxIndex = sSfxCount - 1;
        }
    }

    if (CHECK_BTN_ALL(this->state.input[0].press.button, BTN_A)) {
        AudioMgr_StopAllSfx();
        Audio_PlaySfxGeneral(sSfxList[sSfxIndex].id, &gSfxDefaultPos, 4, &sSfxFreq, &sSfxVol, &sSfxReverb);
    }

    /*if (CHECK_BTN_ALL(this->state.input[0].press.button, BTN_B)) {
        this->state.running = false;
        SET_NEXT_GAMESTATE(&this->state, MapSelect_Init, MapSelectState);

    }*/

    if (CHECK_BTN_ALL(this->state.input[0].press.button, BTN_CRIGHT)) {
        sSfxOption++;
        if (sSfxOption > 2) {
            sSfxOption = 0;
        }
    } else if (CHECK_BTN_ALL(this->state.input[0].press.button, BTN_CLEFT)) {
        sSfxOption--;
        if (sSfxOption < 0) {
            sSfxOption = 2;
        }
    }

    if (CHECK_BTN_ALL(this->state.input[0].press.button, BTN_CUP)) {
        f32 step = (CHECK_BTN_ALL(this->state.input[0].cur.button, BTN_Z) ? 1.0f : 0.1f);

        switch (sSfxOption) {
            case 0:
                sSfxFreq += step;
                break;
            case 1:
                sSfxVol += step;
                break;
            case 2:
                sSfxReverb += 1;
                break;
            default:
                break;
        }
    } else if (CHECK_BTN_ALL(this->state.input[0].press.button, BTN_CDOWN)) {
        f32 step = (CHECK_BTN_ALL(this->state.input[0].cur.button, BTN_Z) ? 1.0f : 0.1f);

        switch (sSfxOption) {
            case 0:
                sSfxFreq -= step;
                break;
            case 1:
                sSfxVol -= step;
                break;
            case 2:
                sSfxReverb -= 1;
                break;
            default:
                break;
        }
    }
}

void SFXList_Destroy(GameState* thisx) {
    SFXListState* this = (SFXListState*)thisx;
}

void SFXList_Init(GameState* thisx) {
    SFXListState* this = (SFXListState*)thisx;

    osSyncPrintf("z_sfxlist.c\n");

    R_UPDATE_RATE = 2;

    Matrix_Init(&this->state);
    View_Init(&this->view, this->state.gfxCtx);

    this->state.main = SFXList_Main;
    this->state.destroy = SFXList_Destroy;

    sSfxMaxPage = Math_FCeilF((f32)sSfxCount / (f32)sSfxPerPage);
}