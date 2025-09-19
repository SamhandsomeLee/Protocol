#ifndef ERNC_PROTOCOL_V2_PROTOCOL_ENGINE_H
#define ERNC_PROTOCOL_V2_PROTOCOL_ENGINE_H

#include <cstdint>
#include <cstddef>
#include <functional>
#include "ERNC_praram.pb.h"

// 轻量协议引擎：负责编码/解码与表驱动分发

namespace ernc::v2 {

struct MessageDescriptor {
    ProtoID proto_id;
    const char* name;
    size_t struct_size;
    const pb_msgdesc_t* pb_desc;
    void (*handler)(const void* msg);
};

struct UnionMapping {
    ProtoID proto_id;
    pb_size_t which_tag;
    size_t union_offset;
    size_t struct_size;
};

struct EngineHooks {
    std::function<void(ProtoID, const void*)> on_before_send;
    std::function<void(ProtoID, const void*)> on_after_recv;
    std::function<void(int, const char*)> on_error;
};

class ITransport {
public:
    virtual ~ITransport() = default;
    virtual bool send(const uint8_t* data, size_t len) = 0;
};

class ProtocolEngine {
public:
    ProtocolEngine(const MessageDescriptor* table,
                   size_t count,
                   const UnionMapping* union_table,
                   size_t union_count,
                   ITransport* transport,
                   EngineHooks hooks = {})
        : message_table_(table), message_count_(count),
          union_table_(union_table), union_count_(union_count),
          transport_(transport), hooks_(std::move(hooks)) {
        init_lookup();
    }

    bool sendMessage(ProtoID proto_id, FunCode fun_code, const void* message);
    void onReceive(const uint8_t* data, size_t len);

private:
    const MessageDescriptor* findMessage(ProtoID id) const;
    const UnionMapping* findUnion(ProtoID id) const;
    void init_lookup();

private:
    const MessageDescriptor* message_table_;
    size_t message_count_;
    const UnionMapping* union_table_;
    size_t union_count_;
    ITransport* transport_;
    EngineHooks hooks_;
    uint8_t id_to_index_[256];
};

} // namespace ernc::v2

#endif // ERNC_PROTOCOL_V2_PROTOCOL_ENGINE_H


