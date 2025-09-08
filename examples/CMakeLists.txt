# Protocol Library Examples

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
)

# 安装示例程序
install(TARGETS
    protocol_basic_example
    protocol_advanced_example
    protocol_di_example
    RUNTIME DESTINATION bin/examples
    COMPONENT Examples
)
