# ERNC Protocol Layer - nanopb集成

## 概述

本协议层基于设计文档 `doc/parameter_binding_design.md` 实现，提供了完整的nanopb协议适配器，实现了界面与protobuf协议的解耦。

## 目录结构

```
protocol/
├── nanopb/                 # nanopb核心运行时库
│   ├── pb.h               # 主头文件
│   ├── pb_common.h/c      # 公共功能
│   ├── pb_decode.h/c      # 解码功能
│   └── pb_encode.h/c      # 编码功能
├── messages/              # 协议消息定义和生成文件
│   ├── ernc.proto         # ERNC协议定义
│   ├── basic.proto        # 基础协议定义
│   ├── ernc.pb.h/c        # 生成的ERNC协议文件
│   └── basic.pb.h/c       # 生成的基础协议文件
├── adapter/               # 协议适配器
│   ├── protocol_adapter.h # 适配器头文件
│   └── protocol_adapter.cpp # 适配器实现
├── config/                # 配置文件
│   └── parameter_mapping.json # 参数映射配置
├── examples/              # 使用示例
│   └── protocol_usage_example.qml # QML使用示例
└── README.md              # 本文档
```

## 核心特性

### 1. 协议适配器 (ProtocolAdapter)

协议适配器提供了高级接口，封装了nanopb的底层操作：

- ✅ **参数操作接口**：支持单个参数和参数组的发送
- ✅ **协议信息接口**：提供协议版本和参数支持查询
- ✅ **通信管理接口**：封装串口连接管理
- ✅ **映射管理接口**：支持配置文件驱动的参数映射

### 2. 支持的消息类型

当前支持以下ERNC协议消息类型：

| 消息类型 | 描述 | 参数路径示例 |
|---------|------|-------------|
| ANC_OFF | ANC控制 | `anc.enabled` |
| ENC_OFF | ENC控制 | `enc.enabled` |
| RNC_OFF | RNC控制 | `rnc.enabled` |
| CHECK_MODE | 检测模式 | `system.check_mode` |
| ALPHA | Alpha参数组 | `tuning.alpha.alpha1` |
| SET1 | Set1参数组 | `tuning.set1.gamma` |
| CALIBRATION_AMP | 校准幅度 | (待实现) |
| CALIBRATION_OTHER | 其他校准 | (待实现) |

### 3. 参数路径映射

使用层次化的参数路径，实现逻辑参数与protobuf字段的映射：

```
# 基本参数
anc.enabled              -> MSG_AncOff.value
enc.enabled              -> MSG_EncOff.value
rnc.enabled              -> MSG_RncOff.value

# 复杂参数组
tuning.alpha.alpha1      -> MSG_Alpha.alpha1
tuning.set1.gamma        -> MSG_Set1.gamma
```

## 使用方法

### 1. C++中使用协议适配器

```cpp
#include "protocol/adapter/protocol_adapter.h"

// 创建适配器实例
ProtocolAdapter adapter;

// 加载配置文件
adapter.loadProtocolMapping("protocol/config/parameter_mapping.json");

// 打开串口连接
adapter.openConnection("COM3", 115200);

// 发送单个参数
adapter.sendParameterUpdate("anc.enabled", true);

// 发送参数组
QStringList paths = {"tuning.alpha.alpha1", "tuning.alpha.alpha2"};
QVariantMap values = {{"tuning.alpha.alpha1", 100}, {"tuning.alpha.alpha2", 200}};
adapter.sendParameterGroup(paths, values);
```

### 2. QML中使用协议适配器

```qml
// 连接串口
protocolAdapter.openConnection("COM3", 115200)

// 发送参数
protocolAdapter.sendParameterUpdate("anc.enabled", true)

// 批量发送
var paths = ["tuning.alpha.alpha1", "tuning.alpha.alpha2"]
var values = {"tuning.alpha.alpha1": 100, "tuning.alpha.alpha2": 200}
protocolAdapter.sendParameterGroup(paths, values)

// 监听信号
Connections {
    target: protocolAdapter
    function onParameterAcknowledged(path) {
        console.log("参数确认:", path)
    }
    function onCommunicationError(error) {
        console.log("通信错误:", error)
    }
}
```

### 3. 参数映射配置

在 `protocol/config/parameter_mapping.json` 中定义参数映射：

```json
{
  "version": "1.0.0",
  "protocolVersion": "2.1.0",
  "mappings": {
    "anc.enabled": {
      "protobufPath": "value",
      "fieldType": "bool",
      "defaultValue": false,
      "messageType": "ANC_OFF",
      "description": "ANC主动降噪开关"
    }
  }
}
```

## 构建配置

CMakeLists.txt已更新以支持protocol层：

```cmake
# Protocol layer - nanopb integration
set(PROTOCOL_SOURCES
    protocol/nanopb/pb_common.c
    protocol/nanopb/pb_decode.c
    protocol/nanopb/pb_encode.c
    protocol/messages/ernc.pb.c
    protocol/messages/basic.pb.c
    protocol/adapter/protocol_adapter.cpp
)

# Create protocol library
add_library(protocol STATIC ${PROTOCOL_SOURCES} ${PROTOCOL_HEADERS})

# Link to main executable
target_link_libraries(ERNC_Clion
    Qt6::SerialPort
    protocol
)
```

## 信号和错误处理

协议适配器提供完整的信号机制：

### 信号列表

- `parameterAcknowledged(QString path)` - 参数确认
- `communicationError(QString error)` - 通信错误
- `protocolVersionMismatch(QString expected, QString actual)` - 版本不匹配
- `connectionStatusChanged(bool connected)` - 连接状态变化
- `dataReceived(QByteArray data)` - 接收到数据

### 错误处理

```cpp
connect(&adapter, &ProtocolAdapter::communicationError, [](const QString& error) {
    qWarning() << "Protocol error:" << error;
    // 实现重连逻辑或用户提示
});
```

## 扩展开发

### 1. 添加新的消息类型

1. 在 `ernc.proto` 中定义新消息
2. 重新生成 `.pb.h/.pb.c` 文件
3. 在 `ProtocolAdapter::MessageType` 中添加新枚举
4. 实现对应的序列化/反序列化方法
5. 更新参数映射配置

### 2. 添加新的参数验证

参考设计文档中的验证器框架，可以扩展参数验证功能。

### 3. 支持新的通信方式

当前只支持串口通信，可以扩展支持TCP/UDP等其他通信方式。

## 性能特点

- ✅ **内存效率**：nanopb运行时占用内存小，适合嵌入式环境
- ✅ **编译时优化**：protobuf字段信息在编译时确定
- ✅ **异步通信**：支持非阻塞的串口通信
- ✅ **批量操作**：支持参数组批量发送，减少通信开销

## 依赖项

- Qt 6.x (Core, SerialPort)
- nanopb 1.0.0-dev
- C++17 标准

## 测试

运行示例程序测试协议适配器：

```bash
# 编译项目
mkdir build && cd build
cmake ..
make

# 运行示例（需要连接硬件设备）
./ERNC_Clion
```

## 已知限制

1. **数组参数**：`MSG_CalibrationAmp` 和 `MSG_CalibrationOther` 中的数组参数处理尚未完全实现
2. **协议版本协商**：版本管理功能待完善
3. **错误恢复**：通信错误的自动恢复机制有待加强

## 后续规划

- [ ] 完善数组参数的处理
- [ ] 实现协议版本管理和迁移
- [ ] 添加参数验证框架
- [ ] 支持配置文件热重载
- [ ] 添加单元测试和集成测试

---

**注意**：本实现基于 `doc/parameter_binding_design.md` 中的设计方案，实现了核心的协议适配功能。如需完整的参数管理和动态界面生成功能，请参考设计文档中的后续开发计划。
