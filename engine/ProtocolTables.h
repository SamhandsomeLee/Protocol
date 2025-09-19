#ifndef ERNC_PROTOCOL_V2_PROTOCOL_TABLES_H
#define ERNC_PROTOCOL_V2_PROTOCOL_TABLES_H

#include "ProtocolEngine.h"
#include "MessageRegistry.h"
#include "ERNC_praram.pb.h"
#include <cstddef>

namespace ernc::v2 {

// 处理函数声明（弱类型，保持简单）
#define X(id, type, h) void handle_##h(const void*);
ERNC_MESSAGES
#undef X

extern const MessageDescriptor MESSAGE_TABLE[];
extern const size_t MESSAGE_COUNT;

extern const UnionMapping UNION_TABLE[];
extern const size_t UNION_COUNT;

} // namespace ernc::v2

#endif // ERNC_PROTOCOL_V2_PROTOCOL_TABLES_H


