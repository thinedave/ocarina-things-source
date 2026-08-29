#ifndef RR_TEST_SCENE_H
#define RR_TEST_SCENE_H

#include "ultra64.h"
#include "romfile.h"
#include "array_count.h"
#include "sequence.h"
#include "actor_profile.h"
#include "bgcheck.h"
#include "camera.h"
#include "cutscene.h"
#include "cutscene_commands.h"
#include "environment.h"
#include "z_math.h"
#include "object.h"
#include "ocarina.h"
#include "path.h"
#include "player.h"
#include "room.h"
#include "scene.h"

// For older decomp versions
#ifndef SCENE_CMD_PLAYER_ENTRY_LIST
#define SCENE_CMD_PLAYER_ENTRY_LIST(length, playerEntryList) \
    { SCENE_CMD_ID_SPAWN_LIST, length, CMD_PTR(playerEntryList) }
#undef SCENE_CMD_SPAWN_LIST
#define SCENE_CMD_SPAWN_LIST(spawnList) \
    { SCENE_CMD_ID_ENTRANCE_LIST, 0, CMD_PTR(spawnList) }
#endif

#ifndef BLEND_RATE_AND_FOG_NEAR
#define BLEND_RATE_AND_FOG_NEAR(blendRate, fogNear) (s16)((((blendRate) / 4) << 10) | (fogNear))
#endif

extern SceneCmd rr_test_scene_header00[];
extern RomFile rr_test_scene_roomList[];
extern u8 _rr_test_room_0SegmentRomStart[];
extern u8 _rr_test_room_0SegmentRomEnd[];
extern ActorEntry rr_test_scene_header00_playerEntryList[];
extern Spawn rr_test_scene_header00_entranceList[];
extern EnvLightSettings rr_test_scene_header00_lightSettings[4];
extern BgCamInfo rr_test_scene_bgCamInfo[];
extern SurfaceType rr_test_scene_polygonTypes[1];
extern Vec3s rr_test_scene_vertices[4];
extern CollisionPoly rr_test_scene_polygons[2];
extern CollisionHeader rr_test_scene_collisionHeader;
extern SceneCmd rr_test_room_0_header00[];
extern s16 rr_test_room_0_header00_objectList[];
extern ActorEntry rr_test_room_0_header00_actorList[];
extern Gfx rr_test_room_0_shapeHeader_entry_0_opaque[];
extern Vtx rr_test_room_0_dl_Floor_mesh_layer_Opaque_vtx_cull[8];
extern Vtx rr_test_room_0_dl_Floor_mesh_layer_Opaque_vtx_0[4];
extern Gfx rr_test_room_0_dl_Floor_mesh_layer_Opaque_tri_0[];
extern Gfx mat_rr_test_room_0_dl_floor_mat_layerOpaque[];
extern Gfx rr_test_room_0_dl_Floor_mesh_layer_Opaque[];
extern RoomShapeNormal rr_test_room_0_shapeHeader;
extern RoomShapeDListsEntry rr_test_room_0_shapeDListsEntry[1];

#endif
