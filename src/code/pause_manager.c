/**
 * @file pause_manager.c
 * @author thinedave
 *
 * Replaces Kaleido as the pause menu.
 */

#include "pause_manager.h"
#include "assets/textures/icon_item_static/icon_item_static.h"
#include "assets/textures/parameter_static/parameter_static.h"
#include "attributes.h"
#include "gfx_setupdl.h"
#include "libc64/math64.h"
#include "libc64/qrand.h"
#include "overlays/misc/ovl_kaleido_scope/z_kaleido_scope.h"
#include "player.h"
#include "printf.h"
#include "radial_menu.h"
#include "controller.h"
#include "gfx.h"
#include "item.h"
#include "save.h"
#include "sfx.h"
#include "ultra64/gbi.h"
#include "play_state.h"
#include "game.h"

#define ITEM_TO_TEX(itemIconStatic, item) ((itemIconStatic) + ((item) * ITEM_ICON_SIZE))
#define TEX_TO_ITEM(itemIconStatic, tex) \
    (((uintptr_t)(tex) - (uintptr_t)(itemIconStatic)) / ITEM_ICON_SIZE)
#define ITEM_STATIC_TEX(itemIconStatic, tex) \
    ((u8*)(itemIconStatic) + ((uintptr_t)(tex) - (uintptr_t)gItemIconDekuStickTex))
#define QUEST_ITEM_TO_TEX(itemIcon24Static, item) \
    ((itemIcon24Static) + (((item) - ITEM_MEDALLION_FOREST) * QUEST_ICON_SIZE))

extern u64 gLButtonTex[];
extern u64 gRButtonTex[];
extern u64 gPauseUnusedCursorTex[];

#define INVENTORY_ORDER_SIZE 24
static u8 sInventoryOrder[INVENTORY_ORDER_SIZE] = {
    SLOT_OCARINA,
    SLOT_LENS_OF_TRUTH,
    SLOT_BOTTLE_1,
    SLOT_BOTTLE_2,
    SLOT_BOTTLE_3,
    SLOT_BOTTLE_4,
    SLOT_DEKU_NUT,
    SLOT_BOMB,
    SLOT_BOMBCHU,
    SLOT_DEKU_STICK,
    SLOT_SLINGSHOT,
    SLOT_BOOMERANG,
    SLOT_BOW,
    SLOT_ARROW_FIRE,
    SLOT_ARROW_ICE,
    SLOT_ARROW_LIGHT,
    SLOT_HOOKSHOT,
    SLOT_HAMMER,
    SLOT_DINS_FIRE,
    SLOT_FARORES_WIND,
    SLOT_NAYRUS_LOVE,
    SLOT_MAGIC_BEAN,
    SLOT_TRADE_CHILD,
    SLOT_TRADE_ADULT,
};

#define EQUIPMENT_ORDER_SIZE 16
static u8 sEquipmentOrder[EQUIPMENT_ORDER_SIZE] = {
    ITEM_SWORD_KOKIRI,
    ITEM_SWORD_MASTER,
    ITEM_SWORD_BIGGORON,
    ITEM_NONE,
    ITEM_SHIELD_DEKU,
    ITEM_SHIELD_HYLIAN,
    ITEM_SHIELD_MIRROR,
    ITEM_NONE,
    ITEM_TUNIC_KOKIRI,
    ITEM_TUNIC_GORON,
    ITEM_TUNIC_ZORA,
    ITEM_NONE,
    ITEM_BOOTS_KOKIRI,
    ITEM_BOOTS_IRON,
    ITEM_BOOTS_HOVER,
    ITEM_NONE,
};

static struct {
    EquipmentType type;
    u8 inv;
    u8 value;
} sItemToEquip[] = {
    [ITEM_SWORD_KOKIRI] = {EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_KOKIRI, EQUIP_VALUE_SWORD_KOKIRI},
    [ITEM_SWORD_MASTER] = {EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_MASTER, EQUIP_VALUE_SWORD_MASTER},
    [ITEM_SWORD_BIGGORON] = {EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_BIGGORON, EQUIP_VALUE_SWORD_BIGGORON},
    [ITEM_SHIELD_DEKU] = {EQUIP_TYPE_SHIELD, EQUIP_INV_SHIELD_DEKU, EQUIP_VALUE_SHIELD_DEKU,},
    [ITEM_SHIELD_HYLIAN] = {EQUIP_TYPE_SHIELD, EQUIP_INV_SHIELD_HYLIAN, EQUIP_VALUE_SHIELD_HYLIAN,},
    [ITEM_SHIELD_MIRROR] = {EQUIP_TYPE_SHIELD, EQUIP_INV_SHIELD_MIRROR, EQUIP_VALUE_SHIELD_MIRROR,},
    [ITEM_TUNIC_KOKIRI] = {EQUIP_TYPE_TUNIC, EQUIP_INV_TUNIC_KOKIRI, EQUIP_VALUE_TUNIC_KOKIRI},
    [ITEM_TUNIC_GORON] = {EQUIP_TYPE_TUNIC, EQUIP_INV_TUNIC_GORON, EQUIP_VALUE_TUNIC_GORON},
    [ITEM_TUNIC_ZORA] = {EQUIP_TYPE_TUNIC, EQUIP_INV_TUNIC_ZORA, EQUIP_VALUE_TUNIC_ZORA},
    [ITEM_BOOTS_KOKIRI] = {EQUIP_TYPE_BOOTS, EQUIP_INV_BOOTS_KOKIRI, EQUIP_VALUE_BOOTS_KOKIRI},
    [ITEM_BOOTS_IRON] = {EQUIP_TYPE_BOOTS, EQUIP_INV_BOOTS_IRON, EQUIP_VALUE_BOOTS_IRON},
    [ITEM_BOOTS_HOVER] = {EQUIP_TYPE_BOOTS, EQUIP_INV_BOOTS_HOVER, EQUIP_VALUE_BOOTS_HOVER},
};

#define CHECK_AGE_REQ_ITEM_PAUSE(item) \
    ((sItemAgeReqs[item] == AGE_REQ_NONE) || (sItemAgeReqs[item] == ((void)0, gSaveContext.save.linkAge)))

static char sItemAgeReqs[] = {
    AGE_REQ_CHILD, // ITEM_DEKU_STICK
    AGE_REQ_NONE,  // ITEM_DEKU_NUT
    AGE_REQ_NONE,  // ITEM_BOMB
    AGE_REQ_ADULT, // ITEM_BOW
    AGE_REQ_ADULT, // ITEM_ARROW_FIRE
    AGE_REQ_NONE,  // ITEM_DINS_FIRE
    AGE_REQ_CHILD, // ITEM_SLINGSHOT
    AGE_REQ_NONE,  // ITEM_OCARINA_FAIRY
    AGE_REQ_NONE,  // ITEM_OCARINA_OF_TIME
    AGE_REQ_NONE,  // ITEM_BOMBCHU
    AGE_REQ_ADULT, // ITEM_HOOKSHOT
    AGE_REQ_ADULT, // ITEM_LONGSHOT
    AGE_REQ_ADULT, // ITEM_ARROW_ICE
    AGE_REQ_NONE,  // ITEM_FARORES_WIND
    AGE_REQ_CHILD, // ITEM_BOOMERANG
    AGE_REQ_NONE,  // ITEM_LENS_OF_TRUTH
    AGE_REQ_CHILD, // ITEM_MAGIC_BEAN
    AGE_REQ_ADULT, // ITEM_HAMMER
    AGE_REQ_ADULT, // ITEM_ARROW_LIGHT
    AGE_REQ_NONE,  // ITEM_NAYRUS_LOVE
    AGE_REQ_NONE,  // ITEM_BOTTLE_EMPTY
    AGE_REQ_NONE,  // ITEM_BOTTLE_POTION_RED
    AGE_REQ_NONE,  // ITEM_BOTTLE_POTION_GREEN
    AGE_REQ_NONE,  // ITEM_BOTTLE_POTION_BLUE
    AGE_REQ_NONE,  // ITEM_BOTTLE_FAIRY
    AGE_REQ_NONE,  // ITEM_BOTTLE_FISH
    AGE_REQ_NONE,  // ITEM_BOTTLE_MILK_FULL
    AGE_REQ_NONE,  // ITEM_BOTTLE_RUTOS_LETTER
    AGE_REQ_NONE,  // ITEM_BOTTLE_BLUE_FIRE
    AGE_REQ_NONE,  // ITEM_BOTTLE_BUG
    AGE_REQ_NONE,  // ITEM_BOTTLE_BIG_POE
    AGE_REQ_NONE,  // ITEM_BOTTLE_MILK_HALF
    AGE_REQ_NONE,  // ITEM_BOTTLE_POE
    AGE_REQ_CHILD, // ITEM_WEIRD_EGG
    AGE_REQ_CHILD, // ITEM_CHICKEN
    AGE_REQ_CHILD, // ITEM_ZELDAS_LETTER
    AGE_REQ_CHILD, // ITEM_MASK_KEATON
    AGE_REQ_CHILD, // ITEM_MASK_SKULL
    AGE_REQ_CHILD, // ITEM_MASK_SPOOKY
    AGE_REQ_CHILD, // ITEM_MASK_BUNNY_HOOD
    AGE_REQ_CHILD, // ITEM_MASK_GORON
    AGE_REQ_CHILD, // ITEM_MASK_ZORA
    AGE_REQ_CHILD, // ITEM_MASK_GERUDO
    AGE_REQ_CHILD, // ITEM_MASK_TRUTH
    AGE_REQ_CHILD, // ITEM_SOLD_OUT
    AGE_REQ_ADULT, // ITEM_POCKET_EGG
    AGE_REQ_ADULT, // ITEM_POCKET_CUCCO
    AGE_REQ_ADULT, // ITEM_COJIRO
    AGE_REQ_ADULT, // ITEM_ODD_MUSHROOM
    AGE_REQ_ADULT, // ITEM_ODD_POTION
    AGE_REQ_ADULT, // ITEM_POACHERS_SAW
    AGE_REQ_ADULT, // ITEM_BROKEN_GORONS_SWORD
    AGE_REQ_ADULT, // ITEM_PRESCRIPTION
    AGE_REQ_ADULT, // ITEM_EYEBALL_FROG
    AGE_REQ_ADULT, // ITEM_EYE_DROPS
    AGE_REQ_ADULT, // ITEM_CLAIM_CHECK
    AGE_REQ_ADULT, // ITEM_BOW_FIRE
    AGE_REQ_ADULT, // ITEM_BOW_ICE
    AGE_REQ_ADULT, // ITEM_BOW_LIGHT
    AGE_REQ_CHILD, // ITEM_SWORD_KOKIRI
    AGE_REQ_ADULT, // ITEM_SWORD_MASTER
    AGE_REQ_ADULT, // ITEM_SWORD_BIGGORON
    AGE_REQ_CHILD, // ITEM_SHIELD_DEKU
    AGE_REQ_NONE,  // ITEM_SHIELD_HYLIAN
    AGE_REQ_ADULT, // ITEM_SHIELD_MIRROR
    AGE_REQ_NONE,  // ITEM_TUNIC_KOKIRI
    AGE_REQ_ADULT, // ITEM_TUNIC_GORON
    AGE_REQ_ADULT, // ITEM_TUNIC_ZORA
    AGE_REQ_NONE,  // ITEM_BOOTS_KOKIRI
    AGE_REQ_ADULT, // ITEM_BOOTS_IRON
    AGE_REQ_ADULT, // ITEM_BOOTS_HOVER
    AGE_REQ_CHILD, // ITEM_BULLET_BAG_30
    AGE_REQ_CHILD, // ITEM_BULLET_BAG_40
    AGE_REQ_CHILD, // ITEM_BULLET_BAG_50
    AGE_REQ_ADULT, // ITEM_QUIVER_30
    AGE_REQ_ADULT, // ITEM_QUIVER_40
    AGE_REQ_ADULT, // ITEM_QUIVER_50
    AGE_REQ_NONE,  // ITEM_BOMB_BAG_20
    AGE_REQ_NONE,  // ITEM_BOMB_BAG_30
    AGE_REQ_NONE,  // ITEM_BOMB_BAG_40
    AGE_REQ_CHILD, // ITEM_STRENGTH_GORONS_BRACELET
    AGE_REQ_ADULT, // ITEM_STRENGTH_SILVER_GAUNTLETS
    AGE_REQ_ADULT, // ITEM_STRENGTH_GOLD_GAUNTLETS
    AGE_REQ_NONE,  // ITEM_SCALE_SILVER
    AGE_REQ_NONE,  // ITEM_SCALE_GOLDEN
    AGE_REQ_ADULT, // ITEM_GIANTS_KNIFE
};

static u8 sChildUpgrades[] = {
    UPG_BULLET_BAG, // EQUIP_QUAD_UPG_BULLETBAG_QUIVER
    UPG_BOMB_BAG,   // EQUIP_QUAD_UPG_BOMB_BAG
    UPG_STRENGTH,   // EQUIP_QUAD_UPG_STRENGTH
    UPG_SCALE,      // EQUIP_QUAD_UPG_SCALE
};
static u8 sAdultUpgrades[] = {
    UPG_QUIVER,   // EQUIP_QUAD_UPG_BULLETBAG_QUIVER
    UPG_BOMB_BAG, // EQUIP_QUAD_UPG_BOMB_BAG
    UPG_STRENGTH, // EQUIP_QUAD_UPG_STRENGTH
    UPG_SCALE,    // EQUIP_QUAD_UPG_SCALE
};

static u8 sChildUpgradeItemBases[] = {
    ITEM_BULLET_BAG_30,            // EQUIP_QUAD_UPG_BULLETBAG_QUIVER
    ITEM_BOMB_BAG_20,              // EQUIP_QUAD_UPG_BOMB_BAG
    ITEM_STRENGTH_GORONS_BRACELET, // EQUIP_QUAD_UPG_STRENGTH
    ITEM_SCALE_SILVER,             // EQUIP_QUAD_UPG_SCALE
};
static u8 sAdultUpgradeItemBases[] = {
    ITEM_QUIVER_30,                // EQUIP_QUAD_UPG_BULLETBAG_QUIVER
    ITEM_BOMB_BAG_20,              // EQUIP_QUAD_UPG_BOMB_BAG
    ITEM_STRENGTH_GORONS_BRACELET, // EQUIP_QUAD_UPG_STRENGTH
    ITEM_SCALE_SILVER,             // EQUIP_QUAD_UPG_SCALE
};

static u8 sUpgradeItemOffsets[] = {
    0,                                              // unused
    ITEM_BOMB_BAG_20 - ITEM_QUIVER_30,              // UPG_BOMB_BAG
    ITEM_STRENGTH_GORONS_BRACELET - ITEM_QUIVER_30, // UPG_STRENGTH
    ITEM_SCALE_SILVER - ITEM_QUIVER_30,             // UPG_SCALE
};

typedef struct {
    s16 x;
    s16 y;
    s16 width;
    s16 height;
} PauseQuestRect;

/* These are the Kaleido quest vertices converted to 320x240 screen rectangles. */
static PauseQuestRect sQuestIconRects[] = {
    { 234, 82, 24, 24 },  // QUEST_MEDALLION_FOREST
    { 234, 114, 24, 24 }, // QUEST_MEDALLION_FIRE
    { 206, 132, 24, 24 }, // QUEST_MEDALLION_WATER
    { 178, 114, 24, 24 }, // QUEST_MEDALLION_SPIRIT
    { 178, 82, 24, 24 },  // QUEST_MEDALLION_SHADOW
    { 206, 64, 24, 24 },  // QUEST_MEDALLION_LIGHT
    { 54, 142, 12, 20 },  // QUEST_SONG_MINUET
    { 72, 142, 12, 20 },  // QUEST_SONG_BOLERO
    { 90, 142, 12, 20 },  // QUEST_SONG_SERENADE
    { 108, 142, 12, 20 }, // QUEST_SONG_REQUIEM
    { 126, 142, 12, 20 }, // QUEST_SONG_NOCTURNE
    { 144, 142, 12, 20 }, // QUEST_SONG_PRELUDE
    { 54, 120, 12, 20 },  // QUEST_SONG_LULLABY
    { 72, 120, 12, 20 },  // QUEST_SONG_EPONA
    { 90, 120, 12, 20 },  // QUEST_SONG_SARIA
    { 108, 120, 12, 20 }, // QUEST_SONG_SUN
    { 126, 120, 12, 20 }, // QUEST_SONG_TIME
    { 144, 120, 12, 20 }, // QUEST_SONG_STORMS
    { 182, 168, 20, 20 }, // QUEST_KOKIRI_EMERALD
    { 208, 168, 20, 20 }, // QUEST_GORON_RUBY
    { 234, 168, 20, 20 }, // QUEST_ZORA_SAPPHIRE
    { 52, 64, 20, 20 },   // QUEST_STONE_OF_AGONY
    { 76, 64, 20, 20 },   // QUEST_GERUDOS_CARD
    { 52, 88, 20, 20 },   // QUEST_SKULL_TOKEN
    { 106, 62, 48, 48 },  // QUEST_HEART_PIECE
};

static PauseQuestRect sQuestSkullDigitShadowRects[] = {
    { 72, 92, 8, 16 },
    { 79, 92, 8, 16 },
    { 88, 92, 8, 16 },
};

static PauseQuestRect sQuestSkullDigitRects[] = {
    { 70, 90, 8, 16 },
    { 77, 90, 8, 16 },
    { 86, 90, 8, 16 },
};

static u8 sQuestSongPrimRed[] = {
    150, 255, 100, 255, 255, 255, 255, 255, 255, 255, 255, 255,
};

static u8 sQuestSongPrimGreen[] = {
    255, 80, 150, 160, 100, 240, 255, 255, 255, 255, 255, 255,
};

static u8 sQuestSongPrimBlue[] = {
    100, 40, 255, 0, 255, 100, 255, 255, 255, 255, 255, 255,
};

Gfx* Gfx_TextureRGBA32(Gfx* displayListHead, void* texture, s16 textureWidth, s16 textureHeight, s16 rectLeft, s16 rectTop,
                    s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy) {
    gDPLoadTextureBlock(displayListHead++, texture, G_IM_FMT_RGBA, G_IM_SIZ_32b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSPTextureRectangle(displayListHead++, rectLeft << 2, rectTop << 2, (rectLeft + rectWidth) << 2,
                        (rectTop + rectHeight) << 2, G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return displayListHead;
}

void PauseManager_Init(PauseManager* this) {
    this->page = PAUSE_PAGE_NONE;
    this->inventory = NULL;
    this->equipment = NULL;
    this->menuCleanupPending = 0;
    this->playerInputBlocked = 0;
    this->questShouldDraw = false;
}

#define CLOSE_MENU(menu) _DW({              \
    if ((menu) != NULL) {                   \
        RadialMenu_Close((menu));           \
    }                                       \
})

void PauseManager_Close(PauseManager* this, PlayState* play) {
    this->playerInputBlocked = 0;
    this->menuCleanupPending = 1;
    this->questShouldDraw = false;
    
    CLOSE_MENU(this->inventory);
    CLOSE_MENU(this->equipment);

    SFX_PLAY_CENTERED(NA_SE_SY_CAMERA_ZOOM_DOWN_2);
}

void PauseManager_InventorySelect(u16 btn, RadialMenu* menu, RadialMenuItem* item, GameState* state) {
    PlayState* play = (PlayState*)state;
    u8 cButton;

    switch (btn) {
        case BTN_CDOWN:
            cButton = 1;
            break;
        case BTN_CRIGHT:
            cButton = 2;
            break;
        case BTN_CLEFT:
            FALLTHROUGH;
        default:
            cButton = 0;
            break;
    }

    u8 interfaceButton = (cButton + 1);
    ItemID itemID = TEX_TO_ITEM(gItemIconStatic, item->texture);

    switch (itemID) {
        case ITEM_ARROW_FIRE:
            itemID = ITEM_BOW_FIRE;
            break;
        case ITEM_ARROW_ICE:
            itemID = ITEM_BOW_ICE;
            break;
        case ITEM_ARROW_LIGHT:
            itemID = ITEM_BOW_LIGHT;
            break;
        default:
            break;
    }

    for (u8 i = 1; i < 4; i++) {
        ItemID buttonItem = gSaveContext.save.info.equips.buttonItems[i];

        if ((buttonItem == itemID &&
             gSaveContext.save.info.equips.cButtonSlots[i - 1] == item->inventorySlot) ||
            ((itemID == ITEM_BOW || itemID == ITEM_BOW_FIRE || itemID == ITEM_BOW_ICE || itemID == ITEM_BOW_LIGHT) &&
             (buttonItem == ITEM_BOW || buttonItem == ITEM_BOW_FIRE || buttonItem == ITEM_BOW_ICE ||
              buttonItem == ITEM_BOW_LIGHT))) {
            if (gSaveContext.save.info.equips.buttonItems[interfaceButton] == ITEM_NONE) {
                gSaveContext.save.info.equips.buttonItems[i] = ITEM_NONE;
                gSaveContext.save.info.equips.cButtonSlots[i - 1] = SLOT_NONE;
            } else {
                gSaveContext.save.info.equips.buttonItems[i] = gSaveContext.save.info.equips.buttonItems[interfaceButton];
                gSaveContext.save.info.equips.cButtonSlots[i - 1] = gSaveContext.save.info.equips.cButtonSlots[cButton];
            }

            Interface_LoadItemIcon1(play, i);
            break;
        }
    }

    gSaveContext.save.info.equips.buttonItems[interfaceButton] = itemID;
    gSaveContext.save.info.equips.cButtonSlots[cButton] = item->inventorySlot;
    Interface_LoadItemIcon1(play, interfaceButton);

    SFX_PLAY_CENTERED(NA_SE_SY_DECIDE);
}

void PauseManager_EquipmentSelect(u16 btn, RadialMenu* menu, RadialMenuItem* item, GameState* state) {
    PlayState* play = (PlayState*)state;
    ItemID itemID = TEX_TO_ITEM(gItemIconStatic, item->texture);

    if (!CHECK_AGE_REQ_ITEM_PAUSE(itemID)) {
        return;
    }

    Inventory_ChangeEquipment(sItemToEquip[itemID].type, sItemToEquip[itemID].value);
    Player_SetEquipmentData(play, GET_PLAYER(play));

    if (sItemToEquip[itemID].type == EQUIP_TYPE_SWORD) {
        gSaveContext.save.info.infTable[INFTABLE_INDEX_1DX] = 0;
        gSaveContext.save.info.equips.buttonItems[0] = itemID;

        if ((sItemToEquip[itemID].value == EQUIP_VALUE_SWORD_BIGGORON) &&
            gSaveContext.save.info.playerData.bgsFlag) {
            gSaveContext.save.info.equips.buttonItems[0] = ITEM_SWORD_BIGGORON;
            gSaveContext.save.info.playerData.swordHealth = 8;
        } else {
            if (gSaveContext.save.info.equips.buttonItems[0] == ITEM_HEART_PIECE_2) {
                gSaveContext.save.info.equips.buttonItems[0] = ITEM_SWORD_BIGGORON;
            }
            if ((gSaveContext.save.info.equips.buttonItems[0] == ITEM_SWORD_BIGGORON) &&
                !gSaveContext.save.info.playerData.bgsFlag &&
                CHECK_OWNED_EQUIP_ALT(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_BROKENGIANTKNIFE)) {
                gSaveContext.save.info.equips.buttonItems[0] = ITEM_GIANTS_KNIFE;
            }
        }
    }

    Interface_LoadItemIcon1(play, 0);

    SFX_PLAY_CENTERED(NA_SE_SY_DECIDE);
}

void PauseManager_DrawAmmoCount(RadialMenu* menu, RadialMenuItem* item, GameState* state, Gfx** gfxP) {
    OPEN_DISPS(state->gfxCtx, __FILE__, __LINE__);

    Gfx* gfx = *gfxP;
    s16 itemIDAndDigits = TEX_TO_ITEM(gItemIconStatic, item->texture);

    if ((itemIDAndDigits == ITEM_DEKU_STICK) || (itemIDAndDigits == ITEM_DEKU_NUT) || (itemIDAndDigits == ITEM_BOMB) || (itemIDAndDigits == ITEM_BOW) ||
        ((itemIDAndDigits >= ITEM_BOW_FIRE) && (itemIDAndDigits <= ITEM_BOW_LIGHT)) || (itemIDAndDigits == ITEM_SLINGSHOT) || (itemIDAndDigits == ITEM_BOMBCHU) ||
        (itemIDAndDigits == ITEM_MAGIC_BEAN)
    ) {

        if ((itemIDAndDigits >= ITEM_BOW_FIRE) && (itemIDAndDigits <= ITEM_BOW_LIGHT)) {
            itemIDAndDigits = ITEM_BOW;
        }

        s16 ammo = AMMO(itemIDAndDigits);

        gDPPipeSync(gfx++);

        if (((itemIDAndDigits == ITEM_BOW) && (AMMO(itemIDAndDigits) == CUR_CAPACITY(UPG_QUIVER))) ||
            ((itemIDAndDigits) && (AMMO(itemIDAndDigits) == CUR_CAPACITY(UPG_BOMB_BAG))) ||
            ((itemIDAndDigits == ITEM_SLINGSHOT) && (AMMO(itemIDAndDigits) == CUR_CAPACITY(UPG_BULLET_BAG))) ||
            ((itemIDAndDigits == ITEM_DEKU_STICK) && (AMMO(itemIDAndDigits) == CUR_CAPACITY(UPG_DEKU_STICKS))) ||
            ((itemIDAndDigits == ITEM_DEKU_NUT) && (AMMO(itemIDAndDigits) == CUR_CAPACITY(UPG_DEKU_NUTS))) ||
            ((itemIDAndDigits == ITEM_BOMBCHU) && (ammo == 50)) || ((itemIDAndDigits == ITEM_MAGIC_BEAN) && (ammo == 15))
        ) {
            gDPSetPrimColor(gfx++, 0, 0, 120, 255, 0, menu->alpha);
        }

        if (ammo == 0) {
            gDPSetPrimColor(gfx++, 0, 0, 100, 100, 100, menu->alpha);
        }

        itemIDAndDigits = 0;
        while (ammo >= 10) {
            itemIDAndDigits++;
            ammo -= 10;
        }

        s16 x = item->x - 10;
        s16 y = item->y + 6;

        if (itemIDAndDigits != 0) {
            gfx = Gfx_TextureIA8(gfx, ((u8*)gAmmoDigit0Tex + ((8 * 8) * itemIDAndDigits)), 8, 8, x, y, 8, 8, 1 << 10, 1 << 10);
        }

        gfx = Gfx_TextureIA8(gfx, ((u8*)gAmmoDigit0Tex + ((8 * 8) * ammo)), 8, 8, x + 6, y, 8, 8, 1 << 10, 1 << 10);
    }

    *gfxP = gfx;

    CLOSE_DISPS(state->gfxCtx, __FILE__, __LINE__);
}

void PauseManager_InventoryPostDraw(RadialMenu* menu, RadialMenuItem* item, GameState* state, Gfx** gfxP) {
    OPEN_DISPS(state->gfxCtx, __FILE__, __LINE__);
    
    PauseManager_DrawAmmoCount(menu, item, state, gfxP);

    CLOSE_DISPS(state->gfxCtx, __FILE__, __LINE__);
}

void PauseManager_DrawEquippedBackground(RadialMenu* menu, RadialMenuItem* item, GameState* state, Gfx** gfxP) {
    OPEN_DISPS(state->gfxCtx, __FILE__, __LINE__);

    Gfx* gfx = *gfxP;
    PlayState* play = (PlayState*)state;
    s16 itemID = TEX_TO_ITEM(gItemIconStatic, item->texture);

    u8 onCButton = false;
    for (u8 i = 1; i < 4; i++) {
        if ((gSaveContext.save.info.equips.buttonItems[i] == itemID &&
             gSaveContext.save.info.equips.cButtonSlots[i - 1] == item->inventorySlot) ||
            ((itemID == ITEM_BOW || itemID == ITEM_ARROW_FIRE) &&
             gSaveContext.save.info.equips.buttonItems[i] == ITEM_BOW_FIRE) ||
            ((itemID == ITEM_BOW || itemID == ITEM_ARROW_ICE) &&
             gSaveContext.save.info.equips.buttonItems[i] == ITEM_BOW_ICE) ||
            ((itemID == ITEM_BOW || itemID == ITEM_ARROW_LIGHT) &&
             gSaveContext.save.info.equips.buttonItems[i] == ITEM_BOW_LIGHT)) {
            onCButton = true;
        }
    }

    if ((itemID >= ITEM_SWORD_KOKIRI && itemID <= ITEM_BOOTS_HOVER && CUR_EQUIP_VALUE(sItemToEquip[itemID].type) == sItemToEquip[itemID].value) ||
        gSaveContext.save.info.equips.buttonItems[0] == itemID ||
        onCButton
    ) {
        gDPPipeSync(gfx++);
        gDPSetPrimColor(gfx++, 0, 0, 150, 150, 150, 200 * (menu->alpha / 255.0f));

        gfx = Gfx_TextureIA8(gfx, ITEM_STATIC_TEX(gItemIconStatic, gPauseUnusedCursorTex), 24, 24, item->x - 12, item->y - 12, 24, 24, 1 << 10, 1 << 10);
    }

    *gfxP = gfx;

    CLOSE_DISPS(state->gfxCtx, __FILE__, __LINE__);
}

void PauseManager_InventoryEquipmentPreDraw(RadialMenu* menu, RadialMenuItem* item, GameState* state, Gfx** gfxP) {
    OPEN_DISPS(state->gfxCtx, __FILE__, __LINE__);

    PauseManager_DrawEquippedBackground(menu, item, state, gfxP);
    
    CLOSE_DISPS(state->gfxCtx, __FILE__, __LINE__);
}

void PauseManager_InitInventory(PauseManager* this, PlayState* play) {
    u8 index = RadialMenu_Init(&play->radialMenuCtx, SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) + 15);

    if (index == RADIAL_COUNT_MAX) {
        this->inventory = NULL;
        return;
    }

    this->inventory = play->radialMenuCtx.elements[index];

    for (u8 i = 0; i < INVENTORY_ORDER_SIZE; i++) {
        ItemID itemID = gSaveContext.save.info.inventory.items[sInventoryOrder[i]];

        if (itemID == ITEM_NONE || !CHECK_AGE_REQ_ITEM_PAUSE(itemID) || itemID == ITEM_SOLD_OUT) {
            continue;
        }

        u8 itemIndex = RadialMenu_AddItem(this->inventory, ITEM_TO_TEX(gItemIconStatic, itemID), G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, PauseManager_InventorySelect, BTN_CLEFT | BTN_CDOWN | BTN_CRIGHT, PauseManager_InventoryPostDraw, PauseManager_InventoryEquipmentPreDraw);

        if (itemIndex != RADIAL_ITEM_INVALID) {
            this->inventory->items.elements[itemIndex].inventorySlot = sInventoryOrder[i];
        }
    }

    RadialMenu_Open(this->inventory, 150);
}

void PauseManager_InitEquipment(PauseManager* this, PlayState* play) {
    u8 index = RadialMenu_Init(&play->radialMenuCtx, -(SCREEN_WIDTH * 2), (SCREEN_HEIGHT / 2) + 15);

    if (index == RADIAL_COUNT_MAX) {
        this->equipment = NULL;
        return;
    }

    this->equipment = play->radialMenuCtx.elements[index];

    for (u8 i = 0; i < EQUIPMENT_ORDER_SIZE; i++) {
        ItemID itemID = sEquipmentOrder[i];

        if (itemID == ITEM_NONE) {
            RadialMenu_AddItem(this->equipment, NULL, G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, NULL, 0, NULL, PauseManager_InventoryEquipmentPreDraw);
            continue;
        }

        if (!CHECK_OWNED_EQUIP(sItemToEquip[itemID].type, sItemToEquip[itemID].inv)) {
            continue;
        }

        u8 itemIndex = RadialMenu_AddItem(this->equipment, ITEM_TO_TEX(gItemIconStatic, itemID), G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, PauseManager_EquipmentSelect, BTN_A, NULL, PauseManager_InventoryEquipmentPreDraw);

        if (!CHECK_AGE_REQ_ITEM_PAUSE(itemID)) {
            this->equipment->items.elements[itemIndex].alpha = 100;
        }
    }

    RadialMenu_Open(this->equipment, 145);
}

static s32 PauseManager_MenuIsTracked(PlayState* play, RadialMenu* menu) {
    if (menu == NULL) {
        return false;
    }

    for (u8 i = 0; i < play->radialMenuCtx.count; i++) {
        if (play->radialMenuCtx.elements[i] == menu) {
            return true;
        }
    }

    return false;
}

static s32 PauseManager_MenuIsActive(PlayState* play, RadialMenu* menu) {
    return PauseManager_MenuIsTracked(play, menu) && menu->update != RadialMenu_Closing &&
           menu->update != RadialMenu_Closed;
}

static void PauseManager_ClearUntrackedMenus(PauseManager* this, PlayState* play) {
    if (this->inventory != NULL && !PauseManager_MenuIsTracked(play, this->inventory)) {
        this->inventory = NULL;
    }

    if (this->equipment != NULL && !PauseManager_MenuIsTracked(play, this->equipment)) {
        this->equipment = NULL;
    }
}

u8 PauseManager_CanOpen(PauseManager* this, PlayState* play) {
    return (play->gameOverCtx.state == GAMEOVER_INACTIVE &&
        play->transitionTrigger == TRANS_TRIGGER_OFF && play->transitionMode == TRANS_MODE_OFF &&
        gSaveContext.save.cutsceneIndex < CS_INDEX_0 && gSaveContext.nextCutsceneIndex < CS_INDEX_0 &&
        !Play_InCsMode(play) && play->shootingGalleryStatus <= 1 &&
        gSaveContext.magicState != MAGIC_STATE_STEP_CAPACITY && gSaveContext.magicState != MAGIC_STATE_FILL &&
        (play->sceneId != SCENE_BOMBCHU_BOWLING_ALLEY || !Flags_GetSwitch(play, 0x38)));
}

void PauseManager_PollOpen(PauseManager* this, PlayState* play) {
    if (CHECK_BTN_ANY(play->state.input[0].press.button, BTN_START) &&
        !this->menuCleanupPending &&
        play->radialMenuCtx.count == 0 &&
        !PauseManager_MenuIsTracked(play, this->inventory) &&
        !PauseManager_MenuIsTracked(play, this->equipment) &&
        PauseManager_CanOpen(this, play)
    ) {
        SFX_PLAY_CENTERED(NA_SE_SY_CAMERA_ZOOM_UP_2);

        PauseManager_InitInventory(this, play);
        PauseManager_InitEquipment(this, play);

        if (this->inventory == NULL || this->equipment == NULL) {
            PauseManager_Close(this, play);
            this->page = PAUSE_PAGE_NONE;
            return;
        }

        this->playerInputBlocked = 1;
        this->page = PAUSE_PAGE_INVENTORY;
    }
}

s32 PauseManager_IsPlayerInputBlocked(PauseManager* this, PlayState* play) {
    if (this->playerInputBlocked) {
        if (CHECK_BTN_ANY(play->state.input[0].press.button, BTN_START) &&
            PauseManager_MenuIsActive(play, this->inventory) &&
            PauseManager_MenuIsActive(play, this->equipment)) {
            return false;
        }

        return true;
    }

    return (this->page == PAUSE_PAGE_NONE) && !this->menuCleanupPending && (play->radialMenuCtx.count == 0) &&
           CHECK_BTN_ANY(play->state.input[0].press.button, BTN_START);
}

void PauseManager_PollClose(PauseManager* this, PlayState* play) {
    if (CHECK_BTN_ANY(play->state.input[0].press.button, BTN_START | BTN_B) &&
        PauseManager_MenuIsActive(play, this->inventory) &&
        PauseManager_MenuIsActive(play, this->equipment)
    ) {
        PauseManager_Close(this, play);
    }
}

void PauseManager_UpdateInventory(PauseManager* this, PlayState* play) {
    if (!PauseManager_MenuIsActive(play, this->inventory) || !PauseManager_MenuIsActive(play, this->equipment)) {
        return;
    }

    if (this->questShouldDraw) {
        if (ABS(this->inventory->x - (SCREEN_WIDTH / 2)) <= 1.0f) {
            this->questShouldDraw = false;
        }
    }

    if (CHECK_BTN_ANY(play->state.input[0].press.button, BTN_Z)) {
        this->page = PAUSE_PAGE_EQUIPMENT;
        this->inventory->acceptingInput = false;
        this->inventory->targetX = (SCREEN_WIDTH + (SCREEN_WIDTH / 2));
        this->equipment->shouldDraw = true;
        this->equipment->acceptingInput = true;
        this->equipment->targetX = (SCREEN_WIDTH / 2);
        this->questShouldDraw = false;

        SFX_PLAY_CENTERED(NA_SE_SY_DUMMY_18);
    } else if (CHECK_BTN_ANY(play->state.input[0].press.button, BTN_R)) {
        this->page = PAUSE_PAGE_QUEST;
        this->inventory->acceptingInput = false;
        this->inventory->targetX = -(SCREEN_WIDTH / 2);
        this->equipment->acceptingInput = false;
        this->equipment->shouldDraw = false;
        this->questShouldDraw = true;

        SFX_PLAY_CENTERED(NA_SE_SY_DUMMY_17);
    }

    PauseManager_PollClose(this, play);
}

void PauseManager_UpdateQuest(PauseManager* this, PlayState* play) {
    if (!PauseManager_MenuIsActive(play, this->inventory) || !PauseManager_MenuIsActive(play, this->equipment)) {
        return;
    }

    if (this->inventory->shouldDraw && ABS(this->inventory->x - this->inventory->targetX) <= 1.0f) {
        this->inventory->shouldDraw = false;
    }

    if (CHECK_BTN_ANY(play->state.input[0].press.button, BTN_Z)) {
        this->page = PAUSE_PAGE_INVENTORY;
        this->inventory->shouldDraw = true;
        this->inventory->acceptingInput = true;
        this->inventory->targetX = (SCREEN_WIDTH / 2);
        this->equipment->shouldDraw = false;
        this->equipment->acceptingInput = false;

        SFX_PLAY_CENTERED(NA_SE_SY_DUMMY_18);
    }

    PauseManager_PollClose(this, play);
}

void PauseManager_UpdateEquipment(PauseManager* this, PlayState* play) {
    if (!PauseManager_MenuIsActive(play, this->inventory) || !PauseManager_MenuIsActive(play, this->equipment)) {
        return;
    }

    if (CHECK_BTN_ANY(play->state.input[0].press.button, BTN_R)) {
        this->page = PAUSE_PAGE_INVENTORY;
        this->equipment->acceptingInput = false;
        this->equipment->targetX = -(SCREEN_WIDTH / 2);
        this->inventory->acceptingInput = true;
        this->inventory->targetX = (SCREEN_WIDTH / 2);

        SFX_PLAY_CENTERED(NA_SE_SY_DUMMY_17);
    }

    PauseManager_PollClose(this, play);
}

void PauseManager_DrawL(PauseManager* this, PlayState* play, Gfx** gfxP) {
    if (this->inventory == NULL || !PauseManager_MenuIsTracked(play, this->inventory) ||
        (!this->inventory->shouldDraw && (this->page != PAUSE_PAGE_QUEST))) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx, __FILE__, __LINE__);

    Gfx* gfx = *gfxP;

    f32 scale = 0.7f;
    f32 sizeX = (24 * scale);
    f32 sizeY = (32 * scale);
    u16 radiusS = ((24 / sizeX) * 1024.0f);
    u16 radiusT = ((32 / sizeY) * 1024.0f);

    gDPPipeSync(gfx++);
    gDPSetCombineMode(gfx++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, this->inventory->alpha);
    gDPSetEnvColor(gfx++, 255, 255, 255, this->inventory->alpha);

    s16 x = (31 - ((play->gameplayFrames / 2) % 5));

    gfx = Gfx_TextureIA8(gfx, ITEM_STATIC_TEX(gItemIconStatic, gLButtonTex), 24, 32, x, 111, sizeX, sizeY, radiusS, radiusT);

    *gfxP = gfx;

    CLOSE_DISPS(play->state.gfxCtx, __FILE__, __LINE__);
}

void PauseManager_DrawR(PauseManager* this, PlayState* play, Gfx** gfxP) {
    if (this->inventory == NULL || !PauseManager_MenuIsTracked(play, this->inventory) ||
        (!this->inventory->shouldDraw && (this->page != PAUSE_PAGE_QUEST))) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx, __FILE__, __LINE__);

    Gfx* gfx = *gfxP;

    f32 scale = 0.7f;
    f32 sizeX = (24 * scale);
    f32 sizeY = (32 * scale);
    u16 radiusS = ((24 / sizeX) * 1024.0f);
    u16 radiusT = ((32 / sizeY) * 1024.0f);

    gDPPipeSync(gfx++);
    gDPSetCombineMode(gfx++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, this->inventory->alpha);
    gDPSetEnvColor(gfx++, 255, 255, 255, this->inventory->alpha);

    s16 x = (265 + ((play->gameplayFrames / 2) % 5));

    gfx = Gfx_TextureIA8(gfx, ITEM_STATIC_TEX(gItemIconStatic, gRButtonTex), 24, 32, x, 111, sizeX, sizeY, radiusS, radiusT);

    *gfxP = gfx;

    CLOSE_DISPS(play->state.gfxCtx, __FILE__, __LINE__);
}

void PauseManager_DrawUpgrades(PauseManager* this, PlayState* play, Gfx** gfxP) {
    if (this->equipment == NULL || !PauseManager_MenuIsTracked(play, this->equipment) || !this->equipment->shouldDraw) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx, __FILE__, __LINE__);

    Gfx* gfx = *gfxP;

    s16 x = (this->equipment->x - 126);
    s16 y = 64;
    f32 scale = 0.7f;
    f32 sizeX = (ITEM_ICON_WIDTH * scale);
    f32 sizeY = (ITEM_ICON_HEIGHT * scale);
    u16 radiusS = ((ITEM_ICON_WIDTH / sizeX) * 1024.0f);
    u16 radiusT = ((ITEM_ICON_HEIGHT / sizeY) * 1024.0f);

    gDPPipeSync(gfx++);
    gDPSetCombineMode(gfx++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, this->equipment->alpha);
    gDPSetEnvColor(gfx++, 255, 255, 255, this->equipment->alpha);

    for (u8 i = 0; i < 4; i++, y += (137 / 4)) {
        if (LINK_IS_CHILD) {
            u16 point = CUR_UPG_VALUE(sChildUpgrades[i]);
            if (point != 0 && CUR_UPG_VALUE(sChildUpgrades[i]) != 0) {
                gfx = Gfx_TextureRGBA32(gfx, ITEM_STATIC_TEX(gItemIconStatic, gItemIcons[sChildUpgradeItemBases[i] + point - 1]), ITEM_ICON_WIDTH, ITEM_ICON_HEIGHT, x, y, sizeX, sizeY, radiusS, radiusT);
            }
        } else {
            if (i == 0 && CUR_UPG_VALUE(sAdultUpgrades[i]) == 0) {
                gfx = Gfx_TextureRGBA32(gfx, ITEM_STATIC_TEX(gItemIconStatic, gItemIcons[sChildUpgradeItemBases[i] + CUR_UPG_VALUE(sChildUpgrades[i]) - 1]), ITEM_ICON_WIDTH, ITEM_ICON_HEIGHT, x, y, sizeX, sizeY, radiusS, radiusT);
            } else if (CUR_UPG_VALUE(sAdultUpgrades[i]) != 0) {
                gfx = Gfx_TextureRGBA32(gfx, ITEM_STATIC_TEX(gItemIconStatic, gItemIcons[sAdultUpgradeItemBases[i] + CUR_UPG_VALUE(sAdultUpgrades[i]) - 1]), ITEM_ICON_WIDTH, ITEM_ICON_HEIGHT, x, y, sizeX, sizeY, radiusS, radiusT);
            }
        }
    }

    *gfxP = gfx;

    CLOSE_DISPS(play->state.gfxCtx, __FILE__, __LINE__);
}

static Gfx* PauseManager_DrawQuestIcon(Gfx* gfx, void* texture, PauseQuestRect* rect, s16 xOffset, u8 alpha) {
    gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, alpha);

    return Gfx_TextureRGBA32(gfx, texture, QUEST_ICON_WIDTH, QUEST_ICON_HEIGHT, rect->x + xOffset, rect->y, rect->width,
                             rect->height, (QUEST_ICON_WIDTH * 1024) / rect->width,
                             (QUEST_ICON_HEIGHT * 1024) / rect->height);
}

void PauseManager_DrawQuest(PauseManager* this, PlayState* play, Gfx** gfxP) {
    Gfx* gfx = *gfxP;
    u32 questItems = gSaveContext.save.info.inventory.questItems;
    u8 alpha = (this->inventory != NULL) ? this->inventory->alpha : 255;
    s16 xOffset = (this->inventory != NULL) ? (s16)(this->inventory->x + (SCREEN_WIDTH / 2)) : 0;
    u8 i;

    if (!play->radialMenuCtx.itemIcon24StaticValid || (gItemIcon24Static == NULL) ||
        !play->radialMenuCtx.itemIconStaticValid || (gItemIconStatic == NULL)) {
        return;
    }

    gDPPipeSync(gfx++);
    gDPSetCombineMode(gfx++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    for (i = 0; i < QUEST_SONG_MINUET; i++) {
        if (CHECK_QUEST_ITEM(QUEST_MEDALLION_FOREST + i)) {
            gfx = PauseManager_DrawQuestIcon(gfx,
                                              QUEST_ITEM_TO_TEX(gItemIcon24Static, ITEM_MEDALLION_FOREST + i),
                                              &sQuestIconRects[i], xOffset, alpha);
        }
    }

    for (i = 0; i < (QUEST_KOKIRI_EMERALD - QUEST_SONG_MINUET); i++) {
        if (CHECK_QUEST_ITEM(QUEST_SONG_MINUET + i)) {
            PauseQuestRect* rect = &sQuestIconRects[QUEST_SONG_MINUET + i];

            gDPSetPrimColor(gfx++, 0, 0, sQuestSongPrimRed[i], sQuestSongPrimGreen[i], sQuestSongPrimBlue[i], alpha);
            gfx = Gfx_TextureIA8(gfx, ITEM_STATIC_TEX(gItemIconStatic, gSongNoteTex), gSongNoteTex_WIDTH,
                                 gSongNoteTex_HEIGHT, rect->x + xOffset, rect->y, rect->width, rect->height,
                                 (gSongNoteTex_WIDTH * 1024) / rect->width,
                                 (gSongNoteTex_HEIGHT * 1024) / rect->height);
        }
    }

    for (i = 0; i < (QUEST_STONE_OF_AGONY - QUEST_KOKIRI_EMERALD); i++) {
        if (CHECK_QUEST_ITEM(QUEST_KOKIRI_EMERALD + i)) {
            gfx = PauseManager_DrawQuestIcon(gfx,
                                              QUEST_ITEM_TO_TEX(gItemIcon24Static, ITEM_KOKIRI_EMERALD + i),
                                              &sQuestIconRects[QUEST_KOKIRI_EMERALD + i], xOffset, alpha);
        }
    }

    for (i = 0; i < (QUEST_HEART_PIECE - QUEST_STONE_OF_AGONY); i++) {
        if (CHECK_QUEST_ITEM(QUEST_STONE_OF_AGONY + i)) {
            gfx = PauseManager_DrawQuestIcon(gfx,
                                              QUEST_ITEM_TO_TEX(gItemIcon24Static, ITEM_STONE_OF_AGONY + i),
                                              &sQuestIconRects[QUEST_STONE_OF_AGONY + i], xOffset, alpha);
        }
    }

    if ((questItems >> QUEST_HEART_PIECE_COUNT) != 0) {
        u8 heartPieceCount = (questItems >> QUEST_HEART_PIECE_COUNT) & 0xF;
        void* heartPieceTextures[] = { gHeartPieceIcon1Tex, gHeartPieceIcon2Tex, gHeartPieceIcon3Tex };

        if (heartPieceCount > 3) {
            heartPieceCount = 3;
        }

        gDPSetPrimColor(gfx++, 0, 0, 255, 0, 0, alpha);
        gfx = Gfx_TextureIA8(gfx, ITEM_STATIC_TEX(gItemIconStatic, heartPieceTextures[heartPieceCount - 1]),
                             HEART_PIECE_ICON_TEX_WIDTH, HEART_PIECE_ICON_TEX_HEIGHT,
                             sQuestIconRects[QUEST_HEART_PIECE].x + xOffset, sQuestIconRects[QUEST_HEART_PIECE].y,
                             sQuestIconRects[QUEST_HEART_PIECE].width, sQuestIconRects[QUEST_HEART_PIECE].height,
                             1 << 10, 1 << 10);
    }

    if (CHECK_QUEST_ITEM(QUEST_SKULL_TOKEN)) {
        u16 tokenCount = gSaveContext.save.info.inventory.gsTokens;
        u8 digits[3];
        u8 shown = false;
        u8 pass;

        if (tokenCount > 999) {
            tokenCount = 999;
        }

        digits[0] = tokenCount / 100;
        digits[1] = (tokenCount / 10) % 10;
        digits[2] = tokenCount % 10;

        for (pass = 0; pass < 2; pass++) {
            shown = false;
            gDPSetPrimColor(gfx++, 0, 0, (pass == 0) ? 0 : ((tokenCount == 100) ? 200 : 255),
                            (pass == 0) ? 0 : ((tokenCount == 100) ? 50 : 255),
                            (pass == 0) ? 0 : ((tokenCount == 100) ? 50 : 255), alpha);

            for (i = 0; i < 3; i++) {
                if ((i >= 2) || (digits[i] != 0) || shown) {
                    PauseQuestRect* rect = (pass == 0) ? &sQuestSkullDigitShadowRects[i] : &sQuestSkullDigitRects[i];

                    gfx = Gfx_TextureI8(gfx, ((u8*)gCounterDigit0Tex + (8 * 16 * digits[i])), 8, 16,
                                        rect->x + xOffset,
                                        rect->y, rect->width, rect->height, 1 << 10, 1 << 10);
                    shown = true;
                }
            }
        }
    }

    *gfxP = gfx;
}

void PauseManager_Draw(PauseManager* this, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx, __FILE__, __LINE__);

    Gfx* gfx = (POLY_OPA_DISP + 1);

    gSPDisplayList(OVERLAY_DISP++, gfx);

    Gfx_SetupDL_39Ptr(&gfx);

    gDPSetAlphaCompare(gfx++, G_AC_NONE);

    if (this->page == PAUSE_PAGE_QUEST || this->questShouldDraw) {
        PauseManager_DrawQuest(this, play, &gfx);
    }

    switch (this->page) {
        case PAUSE_PAGE_EQUIPMENT:
            PauseManager_DrawUpgrades(this, play, &gfx);
            break;
        default:
            break;
    }

    if (this->page == PAUSE_PAGE_INVENTORY || this->page == PAUSE_PAGE_EQUIPMENT) {
        PauseManager_DrawR(this, play, &gfx);
    }

    if (this->page == PAUSE_PAGE_INVENTORY || this->page == PAUSE_PAGE_QUEST) {
        PauseManager_DrawL(this, play, &gfx);
    }

    gSPEndDisplayList(gfx++);
    gSPBranchList(POLY_OPA_DISP, gfx);
    POLY_OPA_DISP = gfx;

    CLOSE_DISPS(play->state.gfxCtx, __FILE__, __LINE__);
}

#define POLL_MENU_DESTROY(this, play, menu) _DW({                   \
    if (PauseManager_MenuIsTracked((play), (menu)) &&               \
        (menu)->update == RadialMenu_Closed) {                      \
        (menu)->state |= RADIAL_MENU_DESTROY;                       \
        (this)->menuCleanupPending = 1;                              \
    }                                                               \
})

void PauseManager_Update(PauseManager* this, PlayState* play) {
    u8 cleanupPendingAtStart = this->menuCleanupPending;

    PauseManager_ClearUntrackedMenus(this, play);

    if (this->page != PAUSE_PAGE_NONE &&
        (!PauseManager_MenuIsTracked(play, this->inventory) || !PauseManager_MenuIsTracked(play, this->equipment))) {
        this->page = PAUSE_PAGE_NONE;
        this->playerInputBlocked = 0;
        this->menuCleanupPending = (play->radialMenuCtx.count != 0);

        CLOSE_MENU(this->inventory);
        CLOSE_MENU(this->equipment);
    }

    if (this->menuCleanupPending && play->radialMenuCtx.count == 0) {
        this->menuCleanupPending = 0;
        this->playerInputBlocked = 0;
        this->page = PAUSE_PAGE_NONE;
    }

    POLL_MENU_DESTROY(this, play, this->inventory);
    POLL_MENU_DESTROY(this, play, this->equipment);

    switch (this->page) {
        case PAUSE_PAGE_NONE:
            if (!cleanupPendingAtStart) {
                PauseManager_PollOpen(this, play);
            }
            break;
        case PAUSE_PAGE_INVENTORY:
            PauseManager_UpdateInventory(this, play);
            break;
        case PAUSE_PAGE_QUEST:
            PauseManager_UpdateQuest(this, play);
            break;
        case PAUSE_PAGE_EQUIPMENT:
            PauseManager_UpdateEquipment(this, play);
            break;
        default:
            PauseManager_PollClose(this, play);
            break;
    }

    PauseManager_Draw(this, play);
}
