#include "supartest_scene.h"

/**
 * Header Child Day (Default)
*/
#define LENGTH_SUPARTEST_ROOM_0_HEADER00_OBJECTLIST 1
#define LENGTH_SUPARTEST_ROOM_0_HEADER00_ACTORLIST 1
SceneCmd supartest_room_0_header00[] = {
    SCENE_CMD_ROOM_SHAPE(&supartest_room_0_shapeHeader),
    SCENE_CMD_ECHO_SETTINGS(0x00),
    SCENE_CMD_ROOM_BEHAVIOR(0x00, 0x00, false, false),
    SCENE_CMD_SKYBOX_DISABLES(false, false),
    SCENE_CMD_TIME_SETTINGS(255, 255, 10),
    SCENE_CMD_OBJECT_LIST(LENGTH_SUPARTEST_ROOM_0_HEADER00_OBJECTLIST, supartest_room_0_header00_objectList),
    SCENE_CMD_ACTOR_LIST(LENGTH_SUPARTEST_ROOM_0_HEADER00_ACTORLIST, supartest_room_0_header00_actorList),
    SCENE_CMD_END(),
};

s16 supartest_room_0_header00_objectList[LENGTH_SUPARTEST_ROOM_0_HEADER00_OBJECTLIST] = {
    OBJECT_VM,
};

ActorEntry supartest_room_0_header00_actorList[LENGTH_SUPARTEST_ROOM_0_HEADER00_ACTORLIST] = {
    // Beamos
    {
        /* Actor ID   */ ACTOR_EN_VM,
        /* Position   */ { 0, 0, -600 },
        /* Rotation   */ { DEG_TO_BINANG(0.000), DEG_TO_BINANG(0.000), DEG_TO_BINANG(0.000) },
        /* Parameters */ (((0x05 << 8) & 0xFF00))
    },
};

RoomShapeNormal supartest_room_0_shapeHeader = {
    ROOM_SHAPE_TYPE_NORMAL,
    ARRAY_COUNT(supartest_room_0_shapeDListsEntry),
    supartest_room_0_shapeDListsEntry,
    supartest_room_0_shapeDListsEntry + ARRAY_COUNT(supartest_room_0_shapeDListsEntry)
};

RoomShapeDListsEntry supartest_room_0_shapeDListsEntry[1] = {
    { supartest_room_0_shapeHeader_entry_0_opaque, NULL }
};

Gfx supartest_room_0_shapeHeader_entry_0_opaque[] = {
	gsSPDisplayList(supartest_room_0_dl_Plane_mesh_layer_Opaque),
	gsSPEndDisplayList(),
};

Vtx supartest_room_0_dl_Plane_mesh_layer_Opaque_vtx_cull[8] = {
	{{ {-1000, 0, 1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-1000, 0, 1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-1000, 0, -1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-1000, 0, -1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {1000, 0, 1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {1000, 0, 1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {1000, 0, -1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {1000, 0, -1000}, 0, {0, 0}, {0, 0, 0, 0} }},
};

Vtx supartest_room_0_dl_Plane_mesh_layer_Opaque_vtx_0[4] = {
	{{ {-1000, 0, 1000}, 0, {-16, 1008}, {0, 127, 0, 255} }},
	{{ {1000, 0, 1000}, 0, {1008, 1008}, {0, 127, 0, 255} }},
	{{ {1000, 0, -1000}, 0, {1008, -16}, {0, 127, 0, 255} }},
	{{ {-1000, 0, -1000}, 0, {-16, -16}, {0, 127, 0, 255} }},
};

Gfx supartest_room_0_dl_Plane_mesh_layer_Opaque_tri_0[] = {
	gsSPVertex(supartest_room_0_dl_Plane_mesh_layer_Opaque_vtx_0 + 0, 4, 0),
	gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
	gsSPEndDisplayList(),
};

Gfx mat_supartest_room_0_dl_f3dlite_material_001_layerOpaque[] = {
	gsSPLoadGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
	gsDPPipeSync(),
	gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, 1, COMBINED, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED),
	gsSPSetOtherMode(G_SETOTHERMODE_H, 4, 20, G_AD_NOISE | G_CD_MAGICSQ | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE | G_TD_CLAMP | G_TP_PERSP | G_CYC_2CYCLE | G_PM_NPRIMITIVE),
	gsSPSetOtherMode(G_SETOTHERMODE_L, 0, 32, G_AC_NONE | G_ZS_PIXEL | G_RM_FOG_SHADE_A | G_RM_AA_ZB_OPA_SURF2),
	gsSPTexture(65535, 65535, 0, 0, 1),
	gsDPSetPrimColor(0, 0, 146, 132, 99, 255),
	gsSPEndDisplayList(),
};

Gfx supartest_room_0_dl_Plane_mesh_layer_Opaque[] = {
	gsSPClearGeometryMode(G_LIGHTING),
	gsSPVertex(supartest_room_0_dl_Plane_mesh_layer_Opaque_vtx_cull + 0, 8, 0),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPCullDisplayList(0, 7),
	gsSPDisplayList(mat_supartest_room_0_dl_f3dlite_material_001_layerOpaque),
	gsSPDisplayList(supartest_room_0_dl_Plane_mesh_layer_Opaque_tri_0),
	gsSPEndDisplayList(),
};

