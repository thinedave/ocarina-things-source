#include "rr_test_scene.h"

RoomShapeNormal rr_test_room_0_shapeHeader = {
    ROOM_SHAPE_TYPE_NORMAL,
    ARRAY_COUNT(rr_test_room_0_shapeDListsEntry),
    rr_test_room_0_shapeDListsEntry,
    rr_test_room_0_shapeDListsEntry + ARRAY_COUNT(rr_test_room_0_shapeDListsEntry)
};

RoomShapeDListsEntry rr_test_room_0_shapeDListsEntry[1] = {
    { rr_test_room_0_shapeHeader_entry_0_opaque, NULL }
};

