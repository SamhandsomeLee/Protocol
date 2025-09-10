# ERNC Protocol v3.0 动态库 CMake 配置
# 支持18种消息类型、ProtoID 0-158映射、层次化参数结构
cmake_minimum_required(VERSION 3.21)

# 设置ERNC协议动态库项目
project(ProtocolLib VERSION 3.0.0 LANGUAGES CXX C)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用 Qt MOC 支持
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找 Qt6 组件
find_package(Qt6 REQUIRED COMPONENTS Core SerialPort)

# 添加 common_utils 支持
if(NOT TARGET common_utils)
    # 如果在独立构建中，添加common子目录
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../common/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../common" "${CMAKE_CURRENT_BINARY_DIR}/common")
    else()
        message(WARNING "common_utils not found and common directory not available")
        # 创建一个空的interface库作为占位符
        add_library(common_utils INTERFACE)
        target_include_directories(common_utils INTERFACE
            "${CMAKE_CURRENT_SOURCE_DIR}/../common"
        )
    endif()
endif()

# 定义版本信息
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version/version_config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version/version_config.h"
    @ONLY
)

# ================================
# 源文件定义
# ================================

# nanopb 核心文件
set(NANOPB_SOURCES
    nanopb/pb_common.c
    nanopb/pb_decode.c
    nanopb/pb_encode.c
)

# ERNC Protocol v3.0 protobuf 消息文件
set(PROTOBUF_SOURCES
    messages/ERNC_praram.pb.c  # 主要的ERNC参数协议文件
    messages/ernc.pb.c         # 传统ERNC协议支持
    messages/basic.pb.c        # 基础消息支持
)

# 传输层文件
set(TRANSPORT_SOURCES
    transport/itransport.h
    transport/serial_transport.h
    transport/serial_transport.cpp
)

# 核心组件文件
set(CORE_SOURCES
    core/message_types.h
    core/message_types.cpp
    core/imessage_handler.h
)

# 映射和序列化文件
set(MAPPING_SOURCES
    mapping/parameter_mapper.h
    mapping/parameter_mapper.cpp
)

set(SERIALIZATION_SOURCES
    serialization/message_serializer.h
    serialization/message_serializer.cpp
    serialization/message_factory.h
    serialization/message_factory.cpp
)

# ERNC v3.0 消息处理器文件 (支持18种消息类型)
set(HANDLER_SOURCES
    # 核心ANC/ENC/RNC控制处理器
    handlers/alpha_message_handler.h
    handlers/alpha_message_handler.cpp
    handlers/anc_message_handler.h
    handlers/anc_message_handler.cpp
    handlers/enc_message_handler.h
    handlers/enc_message_handler.cpp
    
    # 新增的车辆状态处理器
    handlers/vehicle_message_handler.h
    handlers/vehicle_message_handler.cpp
    
    # 新增的通道配置处理器
    handlers/channel_message_handler.h
    handlers/channel_message_handler.cpp
    
    # 新增的RNC参数处理器
    handlers/rnc_message_handler.h
    handlers/rnc_message_handler.cpp
    
    # 新增的实时数据处理器
    handlers/realtime_data_handler.h
    handlers/realtime_data_handler.cpp
)

# 连接管理文件
set(CONNECTION_SOURCES
    connection/connection_manager.h
    connection/connection_manager.cpp
)

# 版本管理文件
set(VERSION_SOURCES
    version/version_manager.h
    version/version_manager.cpp
)

# 协议适配器文件
set(ADAPTER_SOURCES
    adapter/protocol_adapter.h
    adapter/protocol_adapter.cpp
    adapter/protocol_adapter_refactored.h
    adapter/protocol_adapter_refactored.cpp
)

# 缓冲区适配器文件
set(BUFFER_SOURCES
    buffer/protocol_buffer_adapter.h
    buffer/protocol_buffer_adapter.cpp
    buffer/producer_consumer_manager.h
    buffer/producer_consumer_manager.cpp
    buffer/protocol_system_integrator.h
    buffer/protocol_system_integrator.cpp
)

# 公共头文件
set(PUBLIC_HEADERS
    adapter/protocol_adapter.h
    adapter/protocol_adapter_refactored.h
    buffer/protocol_buffer_adapter.h
    buffer/producer_consumer_manager.h
    buffer/protocol_system_integrator.h
    transport/itransport.h
    transport/serial_transport.h
    mapping/parameter_mapper.h
    connection/connection_manager.h
    version/version_manager.h
    core/message_types.h
    "${CMAKE_CURRENT_BINARY_DIR}/version/version_config.h"
)

# 所有源文件
set(ALL_SOURCES
    ${NANOPB_SOURCES}
    ${PROTOBUF_SOURCES}
    ${TRANSPORT_SOURCES}
    ${CORE_SOURCES}
    ${MAPPING_SOURCES}
    ${SERIALIZATION_SOURCES}
    ${HANDLER_SOURCES}
    ${CONNECTION_SOURCES}
    ${VERSION_SOURCES}
    ${ADAPTER_SOURCES}
    ${BUFFER_SOURCES}
)

# ================================
# 创建动态库
# ================================

add_library(ProtocolLib SHARED ${ALL_SOURCES})

# 设置动态库属性
set_target_properties(ProtocolLib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "protocol"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# 定义导出宏
target_compile_definitions(ProtocolLib PRIVATE
    PROTOCOL_LIBRARY_BUILD
)

target_compile_definitions(ProtocolLib PUBLIC
    $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:PROTOCOL_STATIC_DEFINE>
)

# 包含目录
target_include_directories(ProtocolLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/nanopb>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接 Qt 库和通用工具模块
target_link_libraries(ProtocolLib
    PUBLIC
        Qt6::Core
        Qt6::SerialPort
        common_utils
)

# Windows 特定设置
if(WIN32)
    # 设置 DLL 导出
    target_compile_definitions(ProtocolLib PRIVATE
        PROTOCOL_DLL_EXPORT
    )

    # 设置调试信息
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(ProtocolLib PROPERTIES
            PDB_NAME "protocol_d"
            PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )
    endif()
endif()

# ================================
# 创建静态库版本（可选）
# ================================

option(BUILD_STATIC_LIB "Build static library version" ON)

if(BUILD_STATIC_LIB)
    add_library(ProtocolLibStatic STATIC ${ALL_SOURCES})

    set_target_properties(ProtocolLibStatic PROPERTIES
        OUTPUT_NAME "protocol_static"
        POSITION_INDEPENDENT_CODE ON
    )

    target_compile_definitions(ProtocolLibStatic PUBLIC
        PROTOCOL_STATIC_DEFINE
    )

    target_include_directories(ProtocolLibStatic
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/nanopb>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(ProtocolLibStatic
        PUBLIC
            Qt6::Core
            Qt6::SerialPort
            common_utils
    )
endif()

# ================================
# 示例程序（可选编译）
# ================================
option(BUILD_PROTOCOL_EXAMPLES "Build protocol examples" OFF)

if(BUILD_PROTOCOL_EXAMPLES)
    add_executable(ProducerConsumerExample
        examples/producer_consumer_integration_example.cpp
    )

    target_link_libraries(ProducerConsumerExample
        PRIVATE
            ProtocolLib
            Qt6::Core
    )

    target_include_directories(ProducerConsumerExample
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # 设置输出目录
    set_target_properties(ProducerConsumerExample PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )

    message(STATUS "Protocol examples will be built")
endif()

# ================================
# 安装配置
# ================================

# 安装动态库和依赖
install(TARGETS ProtocolLib common_utils
    EXPORT ProtocolLibTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# 安装静态库
if(BUILD_STATIC_LIB)
    install(TARGETS ProtocolLibStatic
        EXPORT ProtocolLibTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

# 安装头文件
install(FILES ${PUBLIC_HEADERS}
    DESTINATION include/protocol
    COMPONENT Development
)

# 安装 nanopb 头文件
install(FILES
    nanopb/pb.h
    nanopb/pb_common.h
    nanopb/pb_decode.h
    nanopb/pb_encode.h
    DESTINATION include/protocol/nanopb
    COMPONENT Development
)

# 安装 ERNC v3.0 protobuf 头文件
install(FILES
    messages/ERNC_praram.pb.h  # 主要的ERNC参数协议头文件
    messages/ernc.pb.h         # 传统ERNC协议头文件
    messages/basic.pb.h        # 基础消息头文件
    DESTINATION include/protocol/messages
    COMPONENT Development
)

# 创建和安装 CMake 配置文件
include(CMakePackageConfigHelpers)

# 生成版本文件
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ProtocolLibConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# 生成配置文件
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ProtocolLibConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ProtocolLibConfig.cmake"
    INSTALL_DESTINATION lib/cmake/ProtocolLib
)

# 安装配置文件
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ProtocolLibConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ProtocolLibConfigVersion.cmake"
    DESTINATION lib/cmake/ProtocolLib
)

# 安装 targets 文件
install(EXPORT ProtocolLibTargets
    FILE ProtocolLibTargets.cmake
    NAMESPACE ProtocolLib::
    DESTINATION lib/cmake/ProtocolLib
)

# ================================
# 示例和测试
# ================================

option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_TESTS "Build test applications" ON)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests directory has been removed
# if(BUILD_TESTS)
#     add_subdirectory(tests)
# endif()

# ================================
# 打包配置
# ================================

# ERNC v3.0 CPack 配置
set(CPACK_PACKAGE_NAME "ERNCProtocolLib")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ERNC Protocol Library v3.0 - 18 Message Types Support")
set(CPACK_PACKAGE_VENDOR "ERNC Project")
set(CPACK_PACKAGE_DESCRIPTION "Complete ERNC protocol implementation supporting 18 message types, ProtoID 0-158 mapping, hierarchical parameters, and vehicle state integration.")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "ERNC Protocol Library v3.0")
    set(CPACK_NSIS_PACKAGE_NAME "ERNCProtocolLib")
    set(CPACK_NSIS_HELP_LINK "https://github.com/ERNC/protocol")
    set(CPACK_NSIS_CONTACT "support@ernc.com")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ERNC Project <support@ernc.com>")
    set(CPACK_RPM_PACKAGE_VENDOR "ERNC Project")
endif()

include(CPack)

# ================================
# 开发者工具
# ================================

# 添加自定义目标用于代码格式化
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    add_custom_target(format-protocol
        COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_SOURCES} ${PUBLIC_HEADERS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting Protocol library code"
    )
endif()

# 输出ERNC v3.0配置信息
message(STATUS "ERNC Protocol Library v3.0 Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Protocol support: 18 message types, ProtoID 0-158")
message(STATUS "  New features: Vehicle state, Channel config, Hierarchical params")
message(STATUS "  Shared library: ON")
message(STATUS "  Static library: ${BUILD_STATIC_LIB}")
message(STATUS "  Examples: ${BUILD_EXAMPLES}")
message(STATUS "  Tests: ${BUILD_TESTS}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Supported message types: ANC/ENC/RNC, Vehicle, Channel, Alpha, Realtime")
