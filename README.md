# ERNC Protocol v2

基于 nanopb 的表驱动协议引擎与 Qt 串口通信实现。

## 架构概述

该模块实现了一个轻量级的协议栈，用于 ERNC 系统的通信需求：

- **表驱动设计**：通过集中式消息注册表自动生成消息描述表与映射关系
- **零拷贝处理**：直接通过 union 偏移进行内存操作，减少数据拷贝
- **高效分发**：O(1) 查表分发（异常情况下回退到线性扫描）
- **简化协议**：直接传输裸 protobuf 字节，无需额外的帧头/帧尾
- **Qt 集成**：提供与 Qt SerialPort 的无缝集成

## 目录结构

```
protocol_v2/
├── engine/              # 协议引擎核心
│   ├── ProtocolEngine.h/cpp    # 表驱动编码/解码与分发引擎
│   ├── MessageRegistry.h       # 消息类型统一注册表
│   ├── ProtocolTables.h/cpp    # 自动生成的消息描述表与映射
│   └── Handlers.cpp            # 消息处理函数实现
├── transport/           # 传输层实现
│   └── QSerialTransport.h/cpp  # Qt6 串口传输适配器
├── messages/            # 消息定义
│   ├── ERNC_praram.proto       # 原始 protobuf 定义
│   └── ERNC_praram.pb.h/c      # nanopb 生成的消息定义
├── nanopb/              # nanopb 库
├── examples/            # 示例程序
│   └── serial_basic_demo.cpp   # 串口监听与消息解析示例
└── CMakeLists.txt       # 构建配置
```

## 快速开始

### 构建

```bash
mkdir build && cd build
cmake ..
make
```

Windows 下会自动使用 windeployqt 部署必要的 Qt 库。

### 运行示例

```bash
# Windows
serial_basic_demo.exe COM3 115200

# Linux
./serial_basic_demo /dev/ttyUSB0 115200
```

## 使用方法

### 1. 创建传输层

```cpp
#include "transport/QSerialTransport.h"

using namespace ernc::v2;

// 创建串口传输层
QSerialTransport transport("COM3", 115200);
```

### 2. 创建协议引擎

```cpp
#include "engine/ProtocolEngine.h"
#include "engine/ProtocolTables.h"

// 设置回调钩子
EngineHooks hooks;
hooks.on_error = [](int code, const char* msg) {
    std::cerr << "Error: " << code << " - " << msg << std::endl;
};
hooks.on_after_recv = [](ProtoID id, const void* payload) {
    std::cout << "Received message: " << static_cast<int>(id) << std::endl;
};

// 创建协议引擎
ProtocolEngine engine(MESSAGE_TABLE, MESSAGE_COUNT,
                      UNION_TABLE, UNION_COUNT,
                      &transport, hooks);
```

### 3. 设置接收处理

```cpp
transport.setReceiveHandler([&](const uint8_t* data, size_t len) {
    engine.onReceive(data, len);
});

if (!transport.open()) {
    std::cerr << "Failed to open serial port" << std::endl;
    return 1;
}
```

### 4. 发送消息

```cpp
// 创建并初始化消息
MSG_CheckMod msg = MSG_CheckMod_init_zero;
msg.check_mod = 1;

// 发送消息
bool ok = engine.sendMessage(ProtoID_MSG_CHECK_MOD, FunCode_FUN_REQUEST, &msg);
```

## 已知限制

- **无分帧机制**：串口接收时将当前可读数据视为单条消息，发送端需一次只发一条
- **无校验/重传**：无校验和、流控或重传机制，仅适用于本地实验环境
- **调试输出问题**：`ProtocolEngine::sendMessage()` 中的二进制打印方式不正确
- **消息注册不完整**：未注册 `MSG_GRAPH_DATA`（在 oneof 中无对应字段）

## 扩展开发

### 添加新消息类型

1. 在 `messages/ERNC_praram.proto` 中定义新消息
2. 在 `ProtoID` 枚举中添加新消息ID
3. 在 `MsgRequestResponse` 的 `oneof payload` 中添加新字段
4. 重新生成 nanopb 文件
5. 在 `engine/MessageRegistry.h` 中注册新消息

### 实现新的传输层

继承 `ITransport` 接口并实现 `send` 方法：

```cpp
class MyTransport : public ITransport {
public:
    bool send(const uint8_t* data, size_t len) override {
        // 实现发送逻辑
        return true;
    }
};
```
