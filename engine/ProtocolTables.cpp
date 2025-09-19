#include "ProtocolTables.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace ernc::v2 {

// 消息描述表
#define X(id, type, h) \
    { ProtoID_MSG_##id, #type, sizeof(MSG_##type), &MSG_##type##_msg, handle_##h },

const MessageDescriptor MESSAGE_TABLE[] = {
    ERNC_MESSAGES
};
#undef X

const size_t MESSAGE_COUNT = sizeof(MESSAGE_TABLE) / sizeof(MESSAGE_TABLE[0]);

// Union 映射表
#define X(id, type, h) \
    { ProtoID_MSG_##id, MsgRequestResponse_msg_##h##_tag, \
      offsetof(MsgRequestResponse, payload), \
      sizeof(MSG_##type) },

const UnionMapping UNION_TABLE[] = {
    ERNC_MESSAGES
};
#undef X

const size_t UNION_COUNT = sizeof(UNION_TABLE) / sizeof(UNION_TABLE[0]);

static_assert(sizeof(MESSAGE_TABLE)/sizeof(MESSAGE_TABLE[0]) == sizeof(UNION_TABLE)/sizeof(UNION_TABLE[0]), "table size mismatch");

} // namespace ernc::v2


