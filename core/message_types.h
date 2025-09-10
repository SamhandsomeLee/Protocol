#ifndef MESSAGE_TYPES_H
#define MESSAGE_TYPES_H

#include <QString>
#include <QHash>

namespace Protocol {

/**
 * @brief 消息类型枚举 - 基于ERNC_praram.proto
 *
 * 定义了所有支持的协议消息类型，与ProtoID枚举对应
 */
enum class MessageType {
    // 实时数据流相关
    CHANNEL_NUMBER = 0,           // 通道数量（acc/mic/spk）
    CHANNEL_AMPLITUDE = 25,       // 通道幅值（mic/acc/spk）
    CHANNEL_SWITCH = 119,         // 通道开关（ACC/MIC/SPK）
    CHECK_MOD = 150,              // 读取实时数据流

    // 车辆CAN信息相关
    ANC_SWITCH = 151,             // ANC/ENC/RNC开关状态
    VEHICLE_STATE = 138,          // 车辆状态（车速/转速/空调等）

    // 传函标定相关
    TRAN_FUNC_FLAG = 153,         // 传函功能标志
    TRAN_FUNC_STATE = 154,        // 传函标定状态
    FILTER_RANGES = 155,          // 滤波器范围配置

    // 系统配置相关
    SYSTEM_RANGES = 157,          // 系统阈值配置（RNC/ENC）

    // ENC标定相关
    ORDER_FLAG = 77,              // 阶次标志开关
    ORDER2_PARAMS = 78,           // 2阶参数集
    ORDER4_PARAMS = 86,           // 4阶参数集
    ORDER6_PARAMS = 87,           // 6阶参数集

    // RNC标定相关
    ALPHA_PARAMS = 158,           // RNC步长参数
    FREQ_DIVISION = 27,           // RNC分频参数
    THRESHOLDS = 33,              // RNC阈值参数

    // 预留的图形数据
    GRAPH_DATA = 156              // 图形数据（预留）
};

/**
 * @brief 功能码枚举 - 对应ERNC_praram.proto中的FunCode
 */
enum class FunctionCode {
    REQUEST = 0,                  // 请求
    RESPONSE = 1                  // 响应
};

/**
 * @brief 消息类型工具函数
 */
class MessageTypeUtils {
public:
    static QString toString(MessageType type);
    static MessageType fromString(const QString& typeStr);
    static MessageType fromProtoID(int protoID);
    static int toProtoID(MessageType type);
    static bool isValid(MessageType type);
    static QString getDescription(MessageType type);

    // 功能码相关
    static QString toString(FunctionCode code);
    static FunctionCode functionCodeFromString(const QString& codeStr);

private:
    static const QHash<MessageType, QString>& getTypeNameMap();
    static const QHash<MessageType, QString>& getTypeDescriptionMap();
    static const QHash<MessageType, int>& getProtoIDMap();
};

} // namespace Protocol

#endif // MESSAGE_TYPES_H
