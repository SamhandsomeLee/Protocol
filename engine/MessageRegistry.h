#ifndef ERNC_PROTOCOL_V2_MESSAGE_REGISTRY_H
#define ERNC_PROTOCOL_V2_MESSAGE_REGISTRY_H

// 单一数据源：在此登记所有支持的消息
// 格式：X(消息ID枚举后缀, 结构体类型后缀, handler短名=oneof字段后缀)
// 说明：
//   - ProtoID 使用 ProtoID_MSG_ 前缀拼接第1列
//   - 结构体类型使用 MSG_ 前缀拼接第2列
//   - oneof 字段短名要与 MsgRequestResponse 中的字段保持一致（去掉 msg_ 前缀）

#define ERNC_MESSAGES \
    /* 实时数据流 */ \
    X(CHANNEL_NUMBER,    ChannelNumber,    channel_number) \
    X(CHANNEL_AMPLITUDE, ChannelAmplitude, channel_amplitude) \
    X(CHANNEL_SWITCH,    ChannelSwitch,    channel_switch) \
    X(CHECK_MOD,         CheckMod,         check_mod) \
    \
    /* 车辆状态 */ \
    X(ANC_SWITCH,        AncSwitch,        anc_switch) \
    X(VEHICLE_STATE,     VehicleState,     vehicle_state) \
    \
    /* 传函标定 */ \
    X(TRAN_FUNC_FLAG,    TranFuncFlag,     tran_func_flag) \
    X(TRAN_FUNC_STATE,   TranFuncState,    tran_func_state) \
    X(FILTER_RANGES,     FilterRanges,     filter_ranges) \
    \
    /* 系统配置 */ \
    X(SYSTEM_RANGES,     SystemRanges,     system_ranges) \
    \
    /* ENC 标定 */ \
    X(ORDER_FLAG,        OrderFlag,        order_flag) \
    X(ORDER2_PARAMS,     Order2Params,     order2_params) \
    X(ORDER4_PARAMS,     Order4Params,     order4_params) \
    X(ORDER6_PARAMS,     Order6Params,     order6_params) \
    \
    /* RNC 标定 */ \
    X(ALPHA_PARAMS,      AlphaParams,      alpha_params) \
    X(FREQ_DIVISION,     FreqDivision,     freq_division) \
    X(THRESHOLDS,        Thresholds,       thresholds)

// 注意：ProtoID 中还有 MSG_GRAPH_DATA，但在 oneof 中未定义对应字段，故不登记

#endif // ERNC_PROTOCOL_V2_MESSAGE_REGISTRY_H


