#ifndef SUPARTEST_SCENE_H
#define SUPARTEST_SCENE_H

#include "ultra64.h"
#include "romfile.h"
#include "array_count.h"
#include "sequence.h"
#include "z64actor_profile.h"
#include "z64bgcheck.h"
#include "z64camera.h"
#include "z64cutscene.h"
#include "z64cutscene_commands.h"
#include "z64environment.h"
#include "z64math.h"
#include "z64object.h"
#include "z64ocarina.h"
#include "z64path.h"
#include "z64player.h"
#include "z64room.h"
#include "z64scene.h"

extern SceneCmd supartest_scene_header00[];
extern RomFile supartest_scene_roomList[];
extern u8 _supartest_room_0SegmentRomStart[];
extern u8 _supartest_room_0SegmentRomEnd[];
extern ActorEntry supartest_scene_header00_playerEntryList[];
extern Spawn supartest_scene_header00_entranceList[];
extern EnvLightSettings supartest_scene_header00_lightSettings[4];
extern SurfaceType supartest_scene_polygonTypes[1];
extern Vec3s supartest_scene_vertices[4];
extern CollisionPoly supartest_scene_polygons[2];
extern CollisionHeader supartest_scene_collisionHeader;
extern SceneCmd supartest_room_0_header00[];
extern s16 supartest_room_0_header00_objectList[];
extern ActorEntry supartest_room_0_header00_actorList[];
extern Gfx supartest_room_0_shapeHeader_entry_0_opaque[];
extern Vtx supartest_room_0_dl_Plane_mesh_layer_Opaque_vtx_cull[8];
extern Vtx supartest_room_0_dl_Plane_mesh_layer_Opaque_vtx_0[4];
extern Gfx supartest_room_0_dl_Plane_mesh_layer_Opaque_tri_0[];
extern Gfx mat_supartest_room_0_dl_f3dlite_material_001_layerOpaque[];
extern Gfx supartest_room_0_dl_Plane_mesh_layer_Opaque[];
extern RoomShapeNormal supartest_room_0_shapeHeader;
extern RoomShapeDListsEntry supartest_room_0_shapeDListsEntry[1];

#endif
