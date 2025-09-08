# ERNC Protocol 动态库

## 🎯 概述

ERNC Protocol 动态库提供了完整的协议适配器功能，支持 C++ 客户端直接使用原有的接口风格，同时享受动态库带来的模块化和版本管理优势。

## 🚀 主要优势

### ✅ **保持现有接口**
- 无需修改现有调用代码
- 完全兼容现有的 `ProtocolAdapter` 接口
- 支持所有 Qt 信号槽机制

### ✅ **模块化架构**
- 协议逻辑与主程序分离
- 支持独立更新和版本管理
- 减少主程序体积

### ✅ **类型安全**
- C++ 编译时类型检查
- RAII 自动内存管理
- STL 容器支持

### ✅ **插件化支持**
- 传输层可插拔设计
- 支持多种传输方式
- 易于扩展新功能

## 📁 目录结构

```
protocol/
├── CMakeLists.txt              # 动态库构建配置
├── version/
│   └── version_config.h.in     # 版本配置模板
├── cmake/
│   └── ProtocolLibConfig.cmake.in  # CMake查找配置
├── examples/                   # 使用示例
│   ├── CMakeLists.txt
│   ├── basic_usage_example.cpp     # 基础使用示例
│   ├── advanced_usage_example.cpp  # 高级使用示例
│   └── dependency_injection_example.cpp
├── adapter/                    # 协议适配器
├── transport/                  # 传输层抽象
├── mapping/                    # 参数映射
├── serialization/              # 消息序列化
├── connection/                 # 连接管理
├── version/                    # 版本管理
└── ...                        # 其他模块
```

## 🔧 构建动态库

### 1. 独立构建

```bash
# 进入 protocol 目录
cd protocol

# 创建构建目录
mkdir build && cd build

# 配置项目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build . --config Release

# 安装（可选）
cmake --install . --prefix ../install
```

### 2. 构建选项

```cmake
# CMake 配置选项
-DBUILD_STATIC_LIB=ON          # 同时构建静态库
-DBUILD_EXAMPLES=ON            # 构建示例程序
-DBUILD_TESTS=ON               # 构建测试程序
-DCMAKE_INSTALL_PREFIX=path    # 安装路径
```

### 3. 生成的文件

```
build/
├── protocol.dll               # Windows 动态库
├── libprotocol.so             # Linux 动态库
├── protocol_static.lib        # 静态库（可选）
├── examples/
│   ├── basic_example.exe      # 基础示例
│   └── advanced_example.exe   # 高级示例
└── ...
```

## 💻 在客户端项目中使用

### 1. CMake 集成

```cmake
# 在主项目的 CMakeLists.txt 中

# 查找 Protocol 库
find_package(ProtocolLib REQUIRED)

# 链接到你的目标
target_link_libraries(your_target
    PRIVATE
        ProtocolLib::ProtocolLib
)
```

### 2. 直接使用现有代码

```cpp
#include "protocol/adapter/protocol_adapter_refactored.h"
#include "protocol/transport/serial_transport.h"

using namespace Protocol;

// 现有代码无需修改
auto transport = new SerialTransport();
auto adapter = new ProtocolAdapterRefactored(transport);

// 使用现有的所有接口
connect(adapter, &ProtocolAdapterRefactored::parameterAcknowledged,
        [](const QString& path) { /* 处理确认 */ });

adapter->sendParameterUpdate("anc.enabled", true);
```

### 3. 部署动态库

```bash
# Windows
copy protocol.dll your_app_directory/
copy Qt6Core.dll your_app_directory/
copy Qt6SerialPort.dll your_app_directory/

# Linux
cp libprotocol.so your_app_directory/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:your_app_directory
```

## 📚 使用示例

### 基础使用

```cpp
#include "protocol/adapter/protocol_adapter_refactored.h"

class MyApplication : public QObject {
    Q_OBJECT

public:
    MyApplication() {
        // 创建传输层
        transport_ = new SerialTransport();
        transport_->setPortName("COM3");

        // 创建协议适配器
        adapter_ = new ProtocolAdapterRefactored(transport_, this);

        // 连接信号（与原来完全一样）
        connect(adapter_, &ProtocolAdapterRefactored::parameterAcknowledged,
                this, &MyApplication::onParameterAck);

        // 使用所有现有功能
        adapter_->sendParameterUpdate("anc.enabled", true);
    }

private slots:
    void onParameterAck(const QString& path) {
        qDebug() << "Parameter acknowledged:" << path;
    }

private:
    SerialTransport* transport_;
    ProtocolAdapterRefactored* adapter_;
};
```

### 高级使用

```cpp
// 直接访问内部组件
auto paramMapper = adapter_->parameterMapper();
auto connectionManager = adapter_->connectionManager();
auto versionManager = adapter_->versionManager();

// 获取详细信息
auto paramInfo = paramMapper->getParameterInfo("anc.enabled");
auto stats = connectionManager->getConnectionStats();
auto version = versionManager->getVersionSummary();

// 序列化操作
QVariantMap params = {{"anc.enabled", true}};
QByteArray data = adapter_->serializeParameters(params);
```

## 🔄 版本管理

### 版本号规则

- **主版本号**: 不兼容的 API 变更
- **次版本号**: 向后兼容的功能新增
- **补丁版本号**: 向后兼容的问题修正

### 兼容性检查

```cpp
#include "protocol/version/version_config.h"

// 编译时检查
#if PROTOCOL_VERSION_CHECK(1, 0, 0)
    // 使用 1.0.0 以上版本的功能
#endif

// 运行时检查
auto versionManager = adapter_->versionManager();
bool compatible = versionManager->isProtocolVersionCompatible("2.1.0");
```

## 🐛 故障排除

### 1. 链接错误

```bash
# 确保动态库在系统路径中
export LD_LIBRARY_PATH=/path/to/protocol/lib:$LD_LIBRARY_PATH  # Linux
set PATH=%PATH%;C:\path\to\protocol\lib                        # Windows
```

### 2. Qt 依赖问题

```bash
# 检查 Qt 版本
qmake --version

# 确保 Qt6Core 和 Qt6SerialPort 可用
ldd protocol.dll  # Linux
dumpbin /dependents protocol.dll  # Windows
```

### 3. 符号未找到

```cpp
// 确保包含了正确的头文件
#include "protocol/adapter/protocol_adapter_refactored.h"

// 检查链接配置
target_link_libraries(your_target ProtocolLib::ProtocolLib)
```

## 🔮 后续计划

- [ ] **传输插件系统** - 支持运行时加载传输插件
- [ ] **配置热重载** - 支持运行时更新参数映射
- [ ] **性能监控** - 内置性能分析工具
- [ ] **日志系统** - 统一的日志记录机制
- [ ] **单元测试** - 完整的测试覆盖

## 📞 技术支持

如有问题，请参考：
1. 示例程序：`examples/` 目录
2. 单元测试：`tests/` 目录
3. API 文档：各头文件中的详细注释

---

**注意**: 此动态库方案完全保持了现有的 C++ 接口，您的客户端代码无需任何修改即可使用。
