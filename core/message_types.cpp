#include "message_types.h"
#include <QHash>

namespace Protocol {

// 静态映射表定义
const QHash<MessageType, QString>& MessageTypeUtils::getTypeNameMap() {
    static const QHash<MessageType, QString> typeMap = {
        // 实时数据流相关
        {MessageType::CHANNEL_NUMBER, "CHANNEL_NUMBER"},
        {MessageType::CHANNEL_AMPLITUDE, "CHANNEL_AMPLITUDE"},
        {MessageType::CHANNEL_SWITCH, "CHANNEL_SWITCH"},
        {MessageType::CHECK_MOD, "CHECK_MOD"},

        // 车辆CAN信息相关
        {MessageType::ANC_SWITCH, "ANC_SWITCH"},
        {MessageType::VEHICLE_STATE, "VEHICLE_STATE"},

        // 传函标定相关
        {MessageType::TRAN_FUNC_FLAG, "TRAN_FUNC_FLAG"},
        {MessageType::TRAN_FUNC_STATE, "TRAN_FUNC_STATE"},
        {MessageType::FILTER_RANGES, "FILTER_RANGES"},

        // 系统配置相关
        {MessageType::SYSTEM_RANGES, "SYSTEM_RANGES"},

        // ENC标定相关
        {MessageType::ORDER_FLAG, "ORDER_FLAG"},
        {MessageType::ORDER2_PARAMS, "ORDER2_PARAMS"},
        {MessageType::ORDER4_PARAMS, "ORDER4_PARAMS"},
        {MessageType::ORDER6_PARAMS, "ORDER6_PARAMS"},

        // RNC标定相关
        {MessageType::ALPHA_PARAMS, "ALPHA_PARAMS"},
        {MessageType::FREQ_DIVISION, "FREQ_DIVISION"},
        {MessageType::THRESHOLDS, "THRESHOLDS"},

        // 图形数据
        {MessageType::GRAPH_DATA, "GRAPH_DATA"}
    };
    return typeMap;
}

const QHash<MessageType, QString>& MessageTypeUtils::getTypeDescriptionMap() {
    static const QHash<MessageType, QString> descMap = {
        // 实时数据流相关
        {MessageType::CHANNEL_NUMBER, "通道数量（acc/mic/spk）"},
        {MessageType::CHANNEL_AMPLITUDE, "通道幅值（mic/acc/spk）"},
        {MessageType::CHANNEL_SWITCH, "通道开关（ACC/MIC/SPK）"},
        {MessageType::CHECK_MOD, "读取实时数据流"},

        // 车辆CAN信息相关
        {MessageType::ANC_SWITCH, "ANC/ENC/RNC开关状态"},
        {MessageType::VEHICLE_STATE, "车辆状态（车速/转速/空调等）"},

        // 传函标定相关
        {MessageType::TRAN_FUNC_FLAG, "传函功能标志"},
        {MessageType::TRAN_FUNC_STATE, "传函标定状态"},
        {MessageType::FILTER_RANGES, "滤波器范围配置"},

        // 系统配置相关
        {MessageType::SYSTEM_RANGES, "系统阈值配置（RNC/ENC）"},

        // ENC标定相关
        {MessageType::ORDER_FLAG, "阶次标志开关"},
        {MessageType::ORDER2_PARAMS, "2阶参数集"},
        {MessageType::ORDER4_PARAMS, "4阶参数集"},
        {MessageType::ORDER6_PARAMS, "6阶参数集"},

        // RNC标定相关
        {MessageType::ALPHA_PARAMS, "RNC步长参数"},
        {MessageType::FREQ_DIVISION, "RNC分频参数"},
        {MessageType::THRESHOLDS, "RNC阈值参数"},

        // 图形数据
        {MessageType::GRAPH_DATA, "图形数据（预留）"}
    };
    return descMap;
}

const QHash<MessageType, int>& MessageTypeUtils::getProtoIDMap() {
    static const QHash<MessageType, int> protoIDMap = {
        {MessageType::CHANNEL_NUMBER, 0},
        {MessageType::CHANNEL_AMPLITUDE, 25},
        {MessageType::CHANNEL_SWITCH, 119},
        {MessageType::CHECK_MOD, 150},
        {MessageType::ANC_SWITCH, 151},
        {MessageType::VEHICLE_STATE, 138},
        {MessageType::TRAN_FUNC_FLAG, 153},
        {MessageType::TRAN_FUNC_STATE, 154},
        {MessageType::FILTER_RANGES, 155},
        {MessageType::SYSTEM_RANGES, 157},
        {MessageType::ORDER_FLAG, 77},
        {MessageType::ORDER2_PARAMS, 78},
        {MessageType::ORDER4_PARAMS, 86},
        {MessageType::ORDER6_PARAMS, 87},
        {MessageType::ALPHA_PARAMS, 158},
        {MessageType::FREQ_DIVISION, 27},
        {MessageType::THRESHOLDS, 33},
        {MessageType::GRAPH_DATA, 156}
    };
    return protoIDMap;
}

QString MessageTypeUtils::toString(MessageType type) {
    return getTypeNameMap().value(type, "UNKNOWN");
}

MessageType MessageTypeUtils::fromString(const QString& typeStr) {
    const auto& nameMap = getTypeNameMap();
    for (auto it = nameMap.begin(); it != nameMap.end(); ++it) {
        if (it.value() == typeStr.toUpper()) {
            return it.key();
        }
    }
    return MessageType::CHANNEL_NUMBER; // 默认返回第一个有效值
}

MessageType MessageTypeUtils::fromProtoID(int protoID) {
    const auto& protoIDMap = getProtoIDMap();
    for (auto it = protoIDMap.begin(); it != protoIDMap.end(); ++it) {
        if (it.value() == protoID) {
            return it.key();
        }
    }
    return MessageType::CHANNEL_NUMBER; // 默认返回第一个有效值
}

int MessageTypeUtils::toProtoID(MessageType type) {
    return getProtoIDMap().value(type, 0); // 默认返回0
}

bool MessageTypeUtils::isValid(MessageType type) {
    return getTypeNameMap().contains(type);
}

QString MessageTypeUtils::getDescription(MessageType type) {
    return getTypeDescriptionMap().value(type, "未知消息类型");
}

QString MessageTypeUtils::toString(FunctionCode code) {
    static const QHash<FunctionCode, QString> codeMap = {
        {FunctionCode::REQUEST, "REQUEST"},
        {FunctionCode::RESPONSE, "RESPONSE"}
    };
    return codeMap.value(code, "UNKNOWN");
}

FunctionCode MessageTypeUtils::functionCodeFromString(const QString& codeStr) {
    static const QHash<QString, FunctionCode> stringMap = {
        {"REQUEST", FunctionCode::REQUEST},
        {"RESPONSE", FunctionCode::RESPONSE}
    };
    return stringMap.value(codeStr.toUpper(), FunctionCode::REQUEST);
}

} // namespace Protocol
