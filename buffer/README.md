# 协议缓冲区和生产者消费者模型

本模块提供了高性能的协议数据缓冲和处理系统，基于环形缓冲区和生产者消费者模型。

## 📁 模块结构

```
protocol/buffer/
├── protocol_buffer_adapter.h/cpp      # 协议缓冲适配器（传统接口）
├── producer_consumer_manager.h/cpp    # 生产者消费者管理器
├── protocol_system_integrator.h/cpp   # 协议系统集成器
└── README.md                          # 本文档
```

## 🚀 快速开始

### 1. 基本使用

```cpp
#include "buffer/protocol_system_integrator.h"

// 创建集成器
auto integrator = ProtocolSystemIntegratorFactory::createStandardIntegrator();

// 集成现有组件
integrator->integrateProtocolAdapter(myProtocolAdapter);
integrator->integrateConnectionManager(myConnectionManager);

// 启动系统
integrator->startIntegration();
```

### 2. 自定义数据处理器

```cpp
// 设置入站数据处理器
integrator->setIncomingDataProcessor([](const QByteArray& data) {
    // 处理接收到的数据
    qDebug() << "Processing incoming data:" << data.size() << "bytes";

    // 自定义解析逻辑
    parseProtocolData(data);
});

// 设置出站数据处理器
integrator->setOutgoingDataProcessor([](const QByteArray& data) -> bool {
    // 处理要发送的数据
    qDebug() << "Sending data:" << data.size() << "bytes";

    // 执行实际发送
    return sendDataOverNetwork(data);
});
```

### 3. 直接使用生产者消费者管理器

```cpp
#include "buffer/producer_consumer_manager.h"

// 创建协议数据管理器
auto dataManager = std::make_unique<ProtocolDataManager>();

// 设置处理器
dataManager->setIncomingDataHandler([](const QByteArray& data) -> bool {
    return processIncomingData(data);
});

// 启动消费者
dataManager->startConsumers();

// 生产数据
dataManager->produceIncomingData(receivedData);
dataManager->produceOutgoingData(outgoingData, priority);
dataManager->produceControlData(controlData, highPriority);
```

## 🏗️ 架构设计

### 组件关系图

```
┌─────────────────────────────────────────────────────────────┐
│                Protocol System Integrator                   │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐                  │
│  │ Protocol        │  │ Connection      │                  │
│  │ Adapter         │  │ Manager         │                  │
│  └─────────────────┘  └─────────────────┘                  │
│           │                     │                          │
│           └─────────┬───────────┘                          │
│                     ▼                                      │
│  ┌─────────────────────────────────────────────────────────┤
│  │         Producer-Consumer Manager                       │
│  │  ┌─────────────────┐  ┌─────────────────┐              │
│  │  │   Ring Buffer   │  │   Work Thread   │              │
│  │  │   (Incoming)    │  │   (Consumer)    │              │
│  │  └─────────────────┘  └─────────────────┘              │
│  │  ┌─────────────────┐  ┌─────────────────┐              │
│  │  │   Ring Buffer   │  │   Statistics    │              │
│  │  │   (Outgoing)    │  │   Reporter      │              │
│  │  └─────────────────┘  └─────────────────┘              │
│  └─────────────────────────────────────────────────────────┤
│           │                                                │
│           ▼                                                │
│  ┌─────────────────────────────────────────────────────────┤
│  │           Legacy Buffer Adapter                        │
│  │           (Backward Compatibility)                     │
│  └─────────────────────────────────────────────────────────┤
└─────────────────────────────────────────────────────────────┘
```

### 数据流向

1. **接收数据流**：
   ```
   Transport → ConnectionManager → Integrator → ProducerConsumer → Processing
   ```

2. **发送数据流**：
   ```
   Application → ProducerConsumer → Integrator → ConnectionManager → Transport
   ```

3. **控制数据流**：
   ```
   Application → ProducerConsumer(高优先级) → 优先处理 → Execution
   ```

## ⚙️ 配置选项

### 集成配置

```cpp
ProtocolSystemIntegrator::IntegrationConfig config;
config.enableLegacyBuffer = true;         // 启用传统缓冲区兼容
config.enableProducerConsumer = true;     // 启用生产者消费者模型
config.enableDataForwarding = true;       // 启用数据转发
config.enableStatisticsReporting = true;  // 启用统计报告
config.statisticsReportInterval = 5000;   // 统计报告间隔（毫秒）

integrator->setIntegrationConfig(config);
```

### 流量控制配置

```cpp
ProducerConsumerManager::FlowControlConfig flowConfig;
flowConfig.maxQueueSize = 10000;        // 最大队列大小
flowConfig.highWaterMark = 8000;        // 高水位标记
flowConfig.lowWaterMark = 2000;         // 低水位标记
flowConfig.maxBatchSize = 100;          // 最大批处理大小
flowConfig.processingIntervalMs = 10;   // 处理间隔（毫秒）

dataManager->setFlowControlConfig(flowConfig);
```

### 处理策略

```cpp
// 设置处理策略
dataManager->setProcessingStrategy(ProcessingStrategy::FIFO);     // 先进先出
dataManager->setProcessingStrategy(ProcessingStrategy::LIFO);     // 后进先出
dataManager->setProcessingStrategy(ProcessingStrategy::PRIORITY); // 优先级处理
dataManager->setProcessingStrategy(ProcessingStrategy::BATCH);    // 批量处理
```

## 📊 性能监控

### 统计信息

```cpp
// 获取集成统计信息
auto stats = integrator->getIntegratedStatistics();

qInfo() << "Producer-Consumer Stats:";
qInfo() << "  Total Produced:" << stats.producerConsumerStats.totalProduced;
qInfo() << "  Total Consumed:" << stats.producerConsumerStats.totalConsumed;
qInfo() << "  Total Dropped:" << stats.producerConsumerStats.totalDropped;
qInfo() << "  Current Queue Size:" << stats.producerConsumerStats.currentQueueSize;
qInfo() << "  Average Processing Time:" << stats.producerConsumerStats.averageProcessingTime;

qInfo() << "System Stats:";
qInfo() << "  Total Data Received:" << stats.systemStats.totalDataReceived;
qInfo() << "  Total Data Sent:" << stats.systemStats.totalDataSent;
qInfo() << "  Total Errors:" << stats.systemStats.totalErrors;
qInfo() << "  Average Latency:" << stats.systemStats.averageLatency;
```

### 性能警告

```cpp
// 监听性能警告
connect(integrator, &ProtocolSystemIntegrator::performanceWarning,
        [](const QString& warning) {
            qWarning() << "Performance Warning:" << warning;

            // 实施性能优化措施
            optimizeSystemPerformance();
        });
```

## 🎛️ 预定义配置

### 1. 标准配置

适用于大多数应用场景，平衡了性能和兼容性。

```cpp
auto integrator = ProtocolSystemIntegratorFactory::createStandardIntegrator();
```

### 2. 高性能配置

适用于对性能要求极高的场景，禁用了一些兼容性功能。

```cpp
auto integrator = ProtocolSystemIntegratorFactory::createHighPerformanceIntegrator();
```

### 3. 兼容性配置

适用于需要与旧系统完全兼容的场景。

```cpp
auto integrator = ProtocolSystemIntegratorFactory::createCompatibilityIntegrator();
```

## 🔧 使用示例

完整的使用示例请参考：
- `protocol/examples/producer_consumer_integration_example.cpp`

该示例展示了：
- 如何设置和配置集成系统
- 如何处理不同类型和优先级的数据
- 如何监控系统性能
- 如何处理错误和异常情况
- 如何进行性能调优

## 🚨 注意事项

1. **线程安全**：所有公共接口都是线程安全的，可以从多个线程安全调用。

2. **内存管理**：使用智能指针管理内存，无需手动释放资源。

3. **性能调优**：
   - 根据数据量调整队列大小
   - 根据处理能力调整处理间隔
   - 根据优先级需求选择合适的处理策略

4. **错误处理**：
   - 设置适当的错误处理器
   - 监控统计信息识别问题
   - 使用性能警告进行预防性维护

5. **向后兼容**：
   - 可以与现有的协议适配器无缝集成
   - 支持渐进式迁移到新系统
   - 保留传统接口的完整功能

## 📈 性能特点

- **高吞吐量**：基于无锁环形缓冲区，支持高频数据处理
- **低延迟**：优化的数据路径，最小化处理延迟
- **可扩展**：支持多生产者多消费者模式
- **内存效率**：预分配内存，避免动态分配开销
- **优先级支持**：支持基于优先级的数据处理
- **流量控制**：自动流量控制，防止系统过载
- **统计监控**：详细的性能统计和监控功能
