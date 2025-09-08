#ifndef MESSAGE_TYPES_H
#define MESSAGE_TYPES_H

#include <QString>

namespace Protocol {

/**
 * @brief 消息类型枚举
 *
 * 定义了所有支持的协议消息类型
 */
enum class MessageType {
    ANC_OFF,
    ENC_OFF,
    RNC_OFF,
    CHECK_MODE,
    CALIBRATION_AMP,
    CALIBRATION_OTHER,
    ALPHA,
    SET1,
    ANC_CONTROL,
    RNC_REFRESH,
    SPEAKER_CHECK,
    TRAN_FUNC_FLAG,
    BYPASS_MODE
};

/**
 * @brief 消息类型工具函数
 */
class MessageTypeUtils {
public:
    static QString toString(MessageType type);
    static MessageType fromString(const QString& typeStr);
    static bool isValid(MessageType type);
};

} // namespace Protocol

#endif // MESSAGE_TYPES_H
