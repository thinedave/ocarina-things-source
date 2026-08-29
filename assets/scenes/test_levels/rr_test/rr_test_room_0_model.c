#include "rr_test_scene.h"

Gfx rr_test_room_0_shapeHeader_entry_0_opaque[] = {
	gsSPDisplayList(rr_test_room_0_dl_Floor_mesh_layer_Opaque),
	gsSPEndDisplayList(),
};

Vtx rr_test_room_0_dl_Floor_mesh_layer_Opaque_vtx_cull[8] = {
	{{ {-300, -120, 300}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-300, -120, 300}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-300, -120, -300}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-300, -120, -300}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {300, -120, 300}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {300, -120, 300}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {300, -120, -300}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {300, -120, -300}, 0, {0, 0}, {0, 0, 0, 0} }},
};

Vtx rr_test_room_0_dl_Floor_mesh_layer_Opaque_vtx_0[4] = {
	{{ {-300, -120, 300}, 0, {-16, 1008}, {0, 127, 0, 255} }},
	{{ {300, -120, 300}, 0, {1008, 1008}, {0, 127, 0, 255} }},
	{{ {300, -120, -300}, 0, {1008, -16}, {0, 127, 0, 255} }},
	{{ {-300, -120, -300}, 0, {-16, -16}, {0, 127, 0, 255} }},
};

Gfx rr_test_room_0_dl_Floor_mesh_layer_Opaque_tri_0[] = {
	gsSPVertex(rr_test_room_0_dl_Floor_mesh_layer_Opaque_vtx_0 + 0, 4, 0),
	gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
	gsSPEndDisplayList(),
};

Gfx mat_rr_test_room_0_dl_floor_mat_layerOpaque[] = {
	gsSPLoadGeometryMode(G_ZBUFFER | G_LIGHTING | G_SHADING_SMOOTH | G_SHADE | G_CULL_BACK | G_FOG),
	gsDPPipeSync(),
	gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, 1, COMBINED, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED),
	gsSPSetOtherMode(G_SETOTHERMODE_H, 4, 20, G_TL_TILE | G_CYC_2CYCLE | G_TF_BILERP | G_TC_FILT | G_CK_NONE | G_TT_NONE | G_CD_MAGICSQ | G_TP_PERSP | G_PM_NPRIMITIVE | G_AD_NOISE | G_TD_CLAMP),
	gsSPSetOtherMode(G_SETOTHERMODE_L, 0, 32, G_AC_NONE | G_RM_FOG_SHADE_A | G_RM_AA_ZB_OPA_SURF2 | G_ZS_PIXEL),
	gsSPTexture(65535, 65535, 0, 0, 1),
	gsDPSetPrimColor(0, 0, 255, 255, 255, 255),
	gsSPEndDisplayList(),
};

Gfx rr_test_room_0_dl_Floor_mesh_layer_Opaque[] = {
	gsSPClearGeometryMode(G_LIGHTING),
	gsSPVertex(rr_test_room_0_dl_Floor_mesh_layer_Opaque_vtx_cull + 0, 8, 0),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPCullDisplayList(0, 7),
	gsSPDisplayList(mat_rr_test_room_0_dl_floor_mat_layerOpaque),
	gsSPDisplayList(rr_test_room_0_dl_Floor_mesh_layer_Opaque_tri_0),
	gsSPEndDisplayList(),
};

