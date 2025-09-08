# ERNC 协议适配器 - 依赖注入架构

本目录包含ERNC项目的协议适配器实现，采用依赖注入架构设计，基于nanopb进行protobuf消息的序列化和反序列化。

## 🎯 架构特点

- **🔧 依赖注入**: 传输层与协议层完全分离，支持运行时切换
- **🚀 多传输支持**: 串口、TCP、UDP、模拟传输等多种传输方式
- **🧪 易于测试**: 可注入模拟传输层进行单元测试
- **📦 职责分离**: 协议适配器专注协议逻辑，传输层专注通信逻辑
- **🔄 可扩展性**: 易于添加新的传输方式和协议功能

## 📁 目录结构

```
protocol/
├── adapter/                        # 协议适配器
│   ├── protocol_adapter.h          # 协议适配器头文件
│   └── protocol_adapter.cpp        # 协议适配器实现
├── transport/                      # 传输层抽象
│   ├── itransport.h                # 传输层抽象接口
│   ├── serial_transport.h          # 串口传输实现
│   └── serial_transport.cpp        # 串口传输实现
├── nanopb/                         # nanopb核心文件
│   ├── pb.h                        # nanopb核心头文件
│   ├── pb_common.h/c               # 通用定义
│   ├── pb_encode.h/c               # 编码器
│   └── pb_decode.h/c               # 解码器
├── messages/                       # protobuf消息定义
│   ├── ernc.pb.h/c                 # ERNC协议消息
│   └── basic.pb.h/c                # 基础消息类型
├── config/                         # 配置文件
│   └── parameter_mapping.json      # 参数映射配置
├── examples/                       # 使用示例
│   ├── dependency_injection_example.cpp    # C++依赖注入示例
│   ├── dependency_injection_example.qml    # QML依赖注入示例
│   └── protocol_usage_example.qml          # 原始使用示例
└── README.md                       # 本文件
```

## 🏗️ 架构设计

### 传输层抽象 (ITransport)

```cpp
class ITransport : public QObject {
    Q_OBJECT
public:
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    virtual bool send(const QByteArray& data) = 0;
    virtual QString description() const = 0;
    virtual QString transportType() const = 0;

signals:
    void dataReceived(const QByteArray& data);
    void connectionStatusChanged(bool connected);
    void transportError(const QString& error);
};
```

### 协议适配器 (ProtocolAdapter)

```cpp
class ProtocolAdapter : public QObject {
    Q_OBJECT
public:
    // 依赖注入构造函数
    explicit ProtocolAdapter(ITransport* transport, QObject* parent = nullptr);

    // 传输层管理
    void setTransport(ITransport* transport);
    ITransport* transport() const;

    // 协议操作
    bool sendParameterUpdate(const QString& path, const QVariant& value);
    bool sendParameterGroup(const QStringList& paths, const QVariantMap& values);

    // 协议信息
    QString getProtocolVersion() const;
    QStringList getSupportedParameters() const;
    bool isParameterSupported(const QString& path) const;
};
```

## 🚀 使用方法

### 1. 基本使用（C++）

```cpp
#include "protocol/adapter/protocol_adapter.h"
#include "protocol/transport/serial_transport.h"

// 创建传输层
SerialTransport* transport = new SerialTransport("COM3", 115200);

// 创建协议适配器并注入传输层
ProtocolAdapter* adapter = new ProtocolAdapter(transport);

// 打开连接
transport->open();

// 发送参数更新
adapter->sendParameterUpdate("anc.enabled", true);

// 发送参数组
QStringList paths = {"tuning.alpha.alpha1", "tuning.alpha.alpha2"};
QVariantMap values = {{"tuning.alpha.alpha1", 0.5}, {"tuning.alpha.alpha2", 0.7}};
adapter->sendParameterGroup(paths, values);
```

### 2. 运行时传输层切换

```cpp
// 创建新的传输层
TcpTransport* tcpTransport = new TcpTransport("192.168.1.100", 8080);

// 切换传输层（旧传输层会自动管理生命周期）
adapter->setTransport(tcpTransport);

// 新传输层立即生效
tcpTransport->open();
```

### 3. 单元测试支持

```cpp
// 创建模拟传输层
MockTransport* mockTransport = new MockTransport();

// 注入模拟传输层
ProtocolAdapter* adapter = new ProtocolAdapter(mockTransport);

// 模拟接收数据
mockTransport->simulateDataReceived(testData);

// 验证发送的数据
QCOMPARE(mockTransport->lastSentData(), expectedData);
```

## 📋 支持的传输层类型

### 1. SerialTransport（串口传输）

- 支持标准串口通信
- 自动重连功能
- 连接状态监控
- 可配置串口参数

```cpp
SerialTransport* serial = new SerialTransport("COM3", 115200);
serial->setAutoReconnect(true);
serial->setDataBits(QSerialPort::Data8);
serial->setParity(QSerialPort::NoParity);
```

### 2. TcpTransport（TCP传输）*

- 支持TCP网络通信
- 自动重连功能
- 支持IPv4/IPv6

*注：TCP传输层实现待添加*

### 3. MockTransport（模拟传输）*

- 用于单元测试
- 可模拟各种网络状态
- 数据收发验证

*注：模拟传输层实现待添加*

## 📊 与原架构对比

| 特性 | 原架构 | 依赖注入架构 |
|------|--------|-------------|
| 职责分离 | ❌ 协议+通信耦合 | ✅ 完全分离 |
| 可测试性 | ❌ 依赖真实串口 | ✅ 可注入模拟传输 |
| 扩展性 | ❌ 只支持串口 | ✅ 支持多种传输 |
| 运行时切换 | ❌ 不支持 | ✅ 动态切换传输层 |
| 代码复用 | ❌ 通信逻辑重复 | ✅ 传输层可复用 |

## 🔧 编译配置

CMakeLists.txt中已自动包含所有必要文件：

```cmake
set(PROTOCOL_SOURCES
    protocol/adapter/protocol_adapter.cpp
    protocol/transport/serial_transport.cpp
    # ... 其他源文件
)

set(PROTOCOL_HEADERS
    protocol/adapter/protocol_adapter.h
    protocol/transport/itransport.h
    protocol/transport/serial_transport.h
    # ... 其他头文件
)
```

## 🎮 示例程序

### C++示例
```bash
# 编译并运行C++示例
cd examples
g++ -o di_example dependency_injection_example.cpp -lQt6Core -lprotocol
./di_example
```

### QML示例
运行QML示例需要在C++中注册相关类型到QML引擎。

## 📚 API参考

详细的API文档请参考各头文件中的注释说明。

## 🐛 故障排除

### 1. 编译错误
- 确保包含了所有必要的头文件
- 检查Qt SerialPort模块是否正确链接

### 2. 运行时错误
- 检查传输层是否正确注入
- 验证传输层连接状态
- 查看错误信号的输出

### 3. 通信问题
- 检查串口参数设置
- 验证设备连接状态
- 查看传输层错误日志

## 🔮 未来扩展

1. **TCP传输层实现**
2. **UDP传输层实现**
3. **WebSocket传输层实现**
4. **模拟传输层实现**（用于单元测试）
5. **传输层连接池管理**
6. **传输层负载均衡**

## 📄 许可证

本项目采用与主项目相同的许可证。
