#include "ProtocolEngine.h"
#include <cstring>
#include <iostream>
#include <ostream>

#include "ERNC_praram.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

namespace ernc::v2 {

static inline uint8_t to_u8(size_t v) { return static_cast<uint8_t>(v & 0xFF); }

void ProtocolEngine::init_lookup() {
    std::memset(id_to_index_, 0xFF, sizeof(id_to_index_));
    for (size_t i = 0; i < message_count_; ++i) {
        uint8_t idx = to_u8(i);
        id_to_index_[static_cast<uint8_t>(message_table_[i].proto_id)] = idx;
    }
}

const MessageDescriptor* ProtocolEngine::findMessage(ProtoID id) const {
    uint8_t idx = id_to_index_[static_cast<uint8_t>(id)];
    if (idx != 0xFF && idx < message_count_) {
        const MessageDescriptor* d = &message_table_[idx];
        if (d->proto_id == id) return d;
    }
    // 退回线性扫描（防御）
    for (size_t i = 0; i < message_count_; ++i) {
        if (message_table_[i].proto_id == id) return &message_table_[i];
    }
    return nullptr;
}

const UnionMapping* ProtocolEngine::findUnion(ProtoID id) const {
    for (size_t i = 0; i < union_count_; ++i) {
        if (union_table_[i].proto_id == id) return &union_table_[i];
    }
    return nullptr;
}

bool ProtocolEngine::sendMessage(ProtoID proto_id, FunCode fun_code, const void* message) {
    if (!transport_ || !message) return false;

    const MessageDescriptor* desc = findMessage(proto_id);
    if (!desc) {
        if (hooks_.on_error) hooks_.on_error(-1, "Unknown message type");
        return false;
    }
    const UnionMapping* map = findUnion(proto_id);
    if (!map) {
        if (hooks_.on_error) hooks_.on_error(-2, "Union mapping not found");
        return false;
    }

    MsgRequestResponse m = MsgRequestResponse_init_zero;
    m.protoID = proto_id;
    m.funCode = fun_code;
    m.which_payload = map->which_tag;

    uint8_t* payload_ptr = reinterpret_cast<uint8_t*>(&m) + map->union_offset;
    std::memcpy(payload_ptr, message, map->struct_size);

    if (hooks_.on_before_send) hooks_.on_before_send(proto_id, message);

    uint8_t buffer[MsgRequestResponse_size];
    pb_ostream_t os = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!pb_encode(&os, &MsgRequestResponse_msg, &m)) {
        if (hooks_.on_error) hooks_.on_error(-3, "Protobuf encoding failed");
        return false;
    }

    // 发送原始protobuf编码字节（无任何前缀/后缀）
    std::cout<< "buff:" << std::endl << buffer << std::endl;
    return transport_->send(buffer, static_cast<size_t>(os.bytes_written));
}

void ProtocolEngine::onReceive(const uint8_t* data, size_t len) {
    if (!data || len == 0) return;

    // 将整个输入缓冲视为单个protobuf消息进行解码（无长度前缀）
    MsgRequestResponse m = MsgRequestResponse_init_zero;
    pb_istream_t is = pb_istream_from_buffer(data, len);
    if (!pb_decode(&is, &MsgRequestResponse_msg, &m)) {
        if (hooks_.on_error) hooks_.on_error(-4, "Protobuf decoding failed");
        return;
    }

    const MessageDescriptor* desc = findMessage(m.protoID);
    const UnionMapping* map = findUnion(m.protoID);
    if (!desc || !map) {
        if (hooks_.on_error) hooks_.on_error(-5, "Descriptor or mapping missing");
        return;
    }

    const uint8_t* payload_ptr = reinterpret_cast<const uint8_t*>(&m) + map->union_offset;
    if (desc->handler) desc->handler(payload_ptr);
    if (hooks_.on_after_recv) hooks_.on_after_recv(m.protoID, payload_ptr);
}

} // namespace ernc::v2


