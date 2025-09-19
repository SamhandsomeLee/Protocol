#include <cstdio>
#include "ProtocolTables.h"
#include "ERNC_praram.pb.h"

namespace ernc::v2 {

// 简单打印型 handler，后续可替换为业务逻辑

#define X(id, type, h) \
void handle_##h(const void* p) { \
    (void)p; \
}
ERNC_MESSAGES
#undef X

} // namespace ernc::v2


