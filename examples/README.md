# ERNC Protocol Examples v3.0

本目录包含了适配新版ERNC_praram.proto的完整示例程序，展示了如何使用更新后的protocol模块。

## 🎯 **新版本特性**

### ✨ **核心更新**
- **18种新消息类型**: 从基础的3种扩展为完整的18种消息类型
- **完整ProtoID映射**: 支持ProtoID 0-158的完整映射关系
- **层次化参数结构**: 支持复杂的嵌套参数配置
- **车辆状态集成**: 新增完整的车辆状态监控功能
- **通道配置管理**: 支持多通道音频处理配置

### 📋 **支持的消息类型**
| 消息类型 | ProtoID | 功能描述 |
|---------|---------|----------|
| `CHANNEL_NUMBER` | 0 | 通道数量配置 |
| `CHANNEL_AMPLITUDE` | 25 | 通道幅值配置 |
| `ANC_SWITCH` | 151 | ANC/ENC/RNC开关控制 |
| `VEHICLE_STATE` | 138 | 车辆状态监控 |
| `ALPHA_PARAMS` | 158 | RNC步长参数 |
| `ORDER2_PARAMS` | 78 | ENC 2阶参数 |
| ...更多 | 0-158 | 完整的ERNC功能支持 |

---

## 📁 **示例程序目录**

### 🔧 **C++示例程序**

#### 1. **basic_usage_example.cpp** - 基础使用示例
```bash
./basic_example
```
**主要演示功能:**
- ✅ ANC/ENC/RNC开关控制（使用新的参数结构）
- ✅ 车辆状态参数发送（速度、发动机转速）
- ✅ 通道配置参数设置（参考通道、误差通道）
- ✅ RNC步长参数组发送
- ✅ 新支持的18种消息类型展示

#### 2. **advanced_usage_example.cpp** - 高级功能示例
```bash
./advanced_example
```
**主要演示功能:**
- ✅ 新消息类型系统详细信息
- ✅ 协议适配器状态监控
- ✅ 层次化参数结构序列化/反序列化
- ✅ ERNC协议版本管理
- ✅ 参数映射配置加载

#### 3. **usage_example.cpp** - MockTransport使用示例
```bash
./usage_example
```
**主要演示功能:**
- ✅ Mock传输层实现
- ✅ 新参数结构的发送和验证
- ✅ 协议适配器统计信息
- ✅ 消息类型覆盖率统计

#### 4. **dependency_injection_example.cpp** - 依赖注入架构示例
```bash
./di_example
```
**主要演示功能:**
- ✅ 传输层动态切换
- ✅ 新参数路径验证
- ✅ 运行时参数支持检查
- ✅ 资源管理和清理

#### 5. **producer_consumer_integration_example.cpp** - 生产者消费者集成示例
```bash
./producer_consumer_example
```
**主要演示功能:**
- ✅ ERNC协议消息生成（JSON格式）
- ✅ 多种消息类型的数据流处理
- ✅ 车辆状态和控制消息的实时处理
- ✅ 性能监控和统计

#### 6. **protocol_integration_test.cpp** - 完整集成测试
```bash
./integration_test
```
**主要演示功能:**
- ✅ 所有18种消息类型的功能验证
- ✅ 参数映射完整性测试
- ✅ 协议适配器集成测试
- ✅ 消息序列化器测试

### 🎨 **QML界面示例**

#### 1. **protocol_usage_example.qml** - QML协议控制界面
**主要特性:**
- ✅ 可视化的ANC/ENC/RNC开关控制
- ✅ 车辆状态实时监控界面
- ✅ 通道配置可视化设置
- ✅ RNC参数调节界面
- ✅ ENC 2阶参数配置

#### 2. **dependency_injection_example.qml** - QML依赖注入架构界面
**主要特性:**
- ✅ 传输层管理界面
- ✅ 协议适配器状态显示
- ✅ 新参数结构演示
- ✅ 运行时切换功能展示

---

## 🚀 **构建和运行**

### 📋 **前置要求**
```bash
- Qt 6.0+
- CMake 3.16+
- C++17 支持的编译器
- nanopb library
```

### 🔨 **构建步骤**
```bash
# 在项目根目录
mkdir build && cd build
cmake ..
make -j4

# 构建示例程序
cd protocol/examples
make -j4
```

### ▶️ **运行示例**
```bash
# C++示例程序
./basic_example           # 基础功能演示
./advanced_example        # 高级功能演示  
./usage_example           # MockTransport演示
./di_example              # 依赖注入演示
./producer_consumer_example # 生产者消费者演示
./integration_test        # 完整集成测试

# QML示例（如果构建了Qt Quick支持）
./qml_example             # QML界面演示
./qml_di_example          # QML依赖注入演示
```

---

## 💡 **新参数结构使用示例**

### 🔄 **ANC/ENC/RNC开关控制**
```cpp
// 旧版本（简单布尔值）
adapter->sendParameterUpdate("anc.enabled", true);

// 新版本（完整的开关结构）
QVariantMap ancParams;
ancParams["anc_off"] = false;  // 开启ANC
ancParams["enc_off"] = true;   // 关闭ENC  
ancParams["rnc_off"] = false;  // 开启RNC
adapter->sendParameterUpdate("anc.enabled", ancParams);
```

### 🚗 **车辆状态参数**
```cpp
QVariantMap vehicleParams;
vehicleParams["speed"] = 80;              // 车速80km/h
vehicleParams["engine_speed"] = 2000;     // 发动机转速2000rpm
adapter->sendParameterUpdate("vehicle.speed", vehicleParams);
```

### 📻 **通道配置参数**
```cpp
QVariantMap channelParams;
channelParams["refer_num"] = 4;           // 4个参考通道
channelParams["error_num"] = 8;           // 8个误差通道
adapter->sendParameterUpdate("channel.refer_num", channelParams);
```

### ⚙️ **RNC参数组**
```cpp
QVariantMap rncParams;
rncParams["alpha1"] = 100;
rncParams["alpha2"] = 150;  
rncParams["alpha3"] = 200;
adapter->sendParameterUpdate("rnc.alpha1", rncParams);
```

---

## 🎯 **测试用例**

### ✅ **自动化测试**
```bash
# 运行所有测试
ctest

# 运行特定测试
ctest -R basic_example_test
ctest -R integration_test
```

### 📊 **性能基准测试**
```bash
# 消息处理性能测试
./integration_test --benchmark

# 内存使用分析
valgrind --tool=memcheck ./basic_example

# 网络传输测试
./producer_consumer_example --network-test
```

---

## 🔍 **故障排除**

### ⚠️ **常见问题**

1. **参数映射文件未找到**
   ```
   解决: 确保parameter_mapping.json在正确路径
   默认路径: protocol/config/parameter_mapping.json
   ```

2. **消息类型不支持**
   ```
   检查: 确认使用的消息类型在18种支持类型范围内
   ProtoID范围: 0-158
   ```

3. **参数结构错误**
   ```
   验证: 使用新的层次化参数结构
   示例: QVariantMap而不是简单值
   ```

4. **编译错误**
   ```
   确认: Qt 6.0+, C++17, nanopb库正确安装
   检查: CMakeLists.txt中的依赖项
   ```

---

## 📞 **技术支持**

### 📋 **版本信息**
- **ERNC Protocol**: v3.0.0
- **支持的消息类型**: 18种
- **ProtoID范围**: 0-158
- **参数数量**: 70+ hierarchical parameters

### 🔗 **相关文档**
- `protocol/messages/ERNC_praram.proto` - Protocol Buffers定义
- `protocol/config/parameter_mapping.json` - 参数映射配置
- `protocol/core/message_types.h` - 消息类型定义

### 🐛 **问题报告**
如遇到问题，请提供以下信息：
1. 使用的示例程序名称
2. 错误信息和日志输出
3. 操作系统和Qt版本
4. 参数配置内容

---

**🎉 恭喜！您现在可以使用功能完整的ERNC Protocol v3.0系统了！**
