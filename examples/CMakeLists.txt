# ERNC Protocol Library Examples
# 支持新的18种消息类型和层次化参数结构

cmake_minimum_required(VERSION 3.16)

# 添加必要的包依赖
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 基础使用示例
add_executable(protocol_basic_example
    basic_usage_example.cpp
)

target_link_libraries(protocol_basic_example
    ProtocolLib
    Qt6::Core
)

set_target_properties(protocol_basic_example PROPERTIES
    OUTPUT_NAME "basic_example"
    AUTOMOC ON
)

# 高级使用示例
add_executable(protocol_advanced_example
    advanced_usage_example.cpp
)

target_link_libraries(protocol_advanced_example
    ProtocolLib
    Qt6::Core
)

set_target_properties(protocol_advanced_example PROPERTIES
    OUTPUT_NAME "advanced_example"
    AUTOMOC ON
)

# 用法示例（MockTransport）
add_executable(protocol_usage_example
    usage_example.cpp
)

target_link_libraries(protocol_usage_example
    ProtocolLib
    Qt6::Core
)

set_target_properties(protocol_usage_example PROPERTIES
    OUTPUT_NAME "usage_example"
    AUTOMOC ON
)

# 依赖注入示例
add_executable(protocol_di_example
    dependency_injection_example.cpp
)

target_link_libraries(protocol_di_example
    ProtocolLib
    Qt6::Core
)

set_target_properties(protocol_di_example PROPERTIES
    OUTPUT_NAME "di_example"
    AUTOMOC ON
)

# 生产者消费者集成示例
add_executable(protocol_producer_consumer_example
    producer_consumer_integration_example.cpp
)

target_link_libraries(protocol_producer_consumer_example
    ProtocolLib
    Qt6::Core
)

set_target_properties(protocol_producer_consumer_example PROPERTIES
    OUTPUT_NAME "producer_consumer_example"
    AUTOMOC ON
)

# ERNC协议集成测试
add_executable(protocol_integration_test
    protocol_integration_test.cpp
)

target_link_libraries(protocol_integration_test
    ProtocolLib
    Qt6::Core
)

set_target_properties(protocol_integration_test PROPERTIES
    OUTPUT_NAME "integration_test"
    AUTOMOC ON
)

# QML示例已移除

# 安装示例程序
install(TARGETS
    protocol_basic_example
    protocol_advanced_example
    protocol_usage_example
    protocol_di_example
    protocol_producer_consumer_example
    protocol_integration_test
    RUNTIME DESTINATION bin/examples
    COMPONENT Examples
)

# QML示例已移除，不再安装

# 安装示例配置文件
install(FILES
    ../config/parameter_mapping.json
    DESTINATION share/examples/config
    COMPONENT Examples
)

# QML源文件已移除，不再安装

# 创建示例程序测试目标
if(BUILD_TESTING)
    enable_testing()

    # 基础示例测试
    add_test(NAME basic_example_test
        COMMAND protocol_basic_example
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # 集成测试
    add_test(NAME integration_test
        COMMAND protocol_integration_test
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # 设置测试超时
    set_tests_properties(basic_example_test integration_test
        PROPERTIES TIMEOUT 30
    )
endif()

# 打印构建信息
message(STATUS "ERNC Protocol Examples:")
message(STATUS "  - Basic Example: ${CMAKE_CURRENT_BINARY_DIR}/basic_example")
message(STATUS "  - Advanced Example: ${CMAKE_CURRENT_BINARY_DIR}/advanced_example")
message(STATUS "  - Usage Example: ${CMAKE_CURRENT_BINARY_DIR}/usage_example")
message(STATUS "  - Dependency Injection: ${CMAKE_CURRENT_BINARY_DIR}/di_example")
message(STATUS "  - Producer Consumer: ${CMAKE_CURRENT_BINARY_DIR}/producer_consumer_example")
message(STATUS "  - Integration Test: ${CMAKE_CURRENT_BINARY_DIR}/integration_test")
message(STATUS "  - QML Examples: Removed")
message(STATUS "  - New Features: 18 message types, ProtoID 0-158, hierarchical parameters")