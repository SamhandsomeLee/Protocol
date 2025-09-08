#include "message_types.h"
#include <QHash>

namespace Protocol {

QString MessageTypeUtils::toString(MessageType type) {
    static const QHash<MessageType, QString> typeMap = {
        {MessageType::ANC_OFF, "ANC_OFF"},
        {MessageType::ENC_OFF, "ENC_OFF"},
        {MessageType::RNC_OFF, "RNC_OFF"},
        {MessageType::CHECK_MODE, "CHECK_MODE"},
        {MessageType::CALIBRATION_AMP, "CALIBRATION_AMP"},
        {MessageType::CALIBRATION_OTHER, "CALIBRATION_OTHER"},
        {MessageType::ALPHA, "ALPHA"},
        {MessageType::SET1, "SET1"},
        {MessageType::ANC_CONTROL, "ANC_CONTROL"},
        {MessageType::RNC_REFRESH, "RNC_REFRESH"},
        {MessageType::SPEAKER_CHECK, "SPEAKER_CHECK"},
        {MessageType::TRAN_FUNC_FLAG, "TRAN_FUNC_FLAG"},
        {MessageType::BYPASS_MODE, "BYPASS_MODE"}
    };

    return typeMap.value(type, "UNKNOWN");
}

MessageType MessageTypeUtils::fromString(const QString& typeStr) {
    static const QHash<QString, MessageType> stringMap = {
        {"ANC_OFF", MessageType::ANC_OFF},
        {"ENC_OFF", MessageType::ENC_OFF},
        {"RNC_OFF", MessageType::RNC_OFF},
        {"CHECK_MODE", MessageType::CHECK_MODE},
        {"CALIBRATION_AMP", MessageType::CALIBRATION_AMP},
        {"CALIBRATION_OTHER", MessageType::CALIBRATION_OTHER},
        {"ALPHA", MessageType::ALPHA},
        {"SET1", MessageType::SET1},
        {"ANC_CONTROL", MessageType::ANC_CONTROL},
        {"RNC_REFRESH", MessageType::RNC_REFRESH},
        {"SPEAKER_CHECK", MessageType::SPEAKER_CHECK},
        {"TRAN_FUNC_FLAG", MessageType::TRAN_FUNC_FLAG},
        {"BYPASS_MODE", MessageType::BYPASS_MODE}
    };

    return stringMap.value(typeStr.toUpper(), MessageType::ANC_OFF); // 默认返回ANC_OFF
}

bool MessageTypeUtils::isValid(MessageType type) {
    return type >= MessageType::ANC_OFF && type <= MessageType::BYPASS_MODE;
}

} // namespace Protocol
