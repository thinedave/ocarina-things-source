#include "rr_test_scene.h"

/**
 * Header Child Day (Default)
*/
#define LENGTH_RR_TEST_ROOM_0_HEADER00_OBJECTLIST 1
#define LENGTH_RR_TEST_ROOM_0_HEADER00_ACTORLIST 1
SceneCmd rr_test_room_0_header00[] = {
    SCENE_CMD_ROOM_SHAPE(&rr_test_room_0_shapeHeader),
    SCENE_CMD_ECHO_SETTINGS(0x00),
    SCENE_CMD_ROOM_BEHAVIOR(0x00, 0x00, false, false),
    SCENE_CMD_SKYBOX_DISABLES(false, false),
    SCENE_CMD_TIME_SETTINGS(255, 255, 10),
    SCENE_CMD_OBJECT_LIST(LENGTH_RR_TEST_ROOM_0_HEADER00_OBJECTLIST, rr_test_room_0_header00_objectList),
    SCENE_CMD_ACTOR_LIST(LENGTH_RR_TEST_ROOM_0_HEADER00_ACTORLIST, rr_test_room_0_header00_actorList),
    SCENE_CMD_END(),
};

s16 rr_test_room_0_header00_objectList[LENGTH_RR_TEST_ROOM_0_HEADER00_OBJECTLIST] = {
    OBJECT_RR,
};

ActorEntry rr_test_room_0_header00_actorList[LENGTH_RR_TEST_ROOM_0_HEADER00_ACTORLIST] = {
    // Like-Like
    {
        /* Actor ID   */ ACTOR_EN_RR,
        /* Position   */ { -200, -120, 0 },
        /* Rotation   */ { DEG_TO_BINANG(0.000), DEG_TO_BINANG(0.000), DEG_TO_BINANG(0.000) },
        /* Parameters */ 0x0
    },
};

