cmake_minimum_required(VERSION 3.21)

add_library(ernc_protocol_v2
    engine/ProtocolEngine.cpp
    engine/ProtocolTables.cpp
    engine/Handlers.cpp
    transport/QSerialTransport.cpp
    transport/QSerialTransport.h
    messages/ERNC_praram.pb.c
    nanopb/pb_common.c
    nanopb/pb_decode.c
    nanopb/pb_encode.c
)

target_include_directories(ernc_protocol_v2 PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/engine
    ${CMAKE_CURRENT_LIST_DIR}/transport
    ${CMAKE_CURRENT_LIST_DIR}/messages
    ${CMAKE_CURRENT_LIST_DIR}/nanopb
)

find_package(Qt6 REQUIRED COMPONENTS Core SerialPort)
target_link_libraries(ernc_protocol_v2 PUBLIC Qt6::Core Qt6::SerialPort)

target_compile_features(ernc_protocol_v2 PUBLIC cxx_std_17)

set_target_properties(ernc_protocol_v2 PROPERTIES AUTOMOC ON)



# examples
option(ERNC_PROTOCOL_V2_BUILD_EXAMPLES "Build protocol_v2 examples" ON)
if (ERNC_PROTOCOL_V2_BUILD_EXAMPLES)
    add_executable(serial_basic_demo examples/serial_basic_demo.cpp)
    target_link_libraries(serial_basic_demo PRIVATE ernc_protocol_v2 Qt6::Core Qt6::SerialPort)
    target_compile_features(serial_basic_demo PRIVATE cxx_std_17)

    if (WIN32)
        set(DEBUG_SUFFIX)
        if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
            set(DEBUG_SUFFIX "d")
        endif()
        set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
        if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
            if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
                set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
            endif()
        endif()

        # 优先使用 windeployqt 自动部署
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_INSTALL_PATH}/bin")
        if (WINDEPLOYQT_EXECUTABLE)
            add_custom_command(TARGET serial_basic_demo POST_BUILD
                COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:serial_basic_demo>"
                COMMENT "Running windeployqt for serial_basic_demo")
        else()
            # 手工拷贝（仅在源文件存在时执行）
            set(_qt_platform_plugin "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
            if (EXISTS "${_qt_platform_plugin}")
                add_custom_command(TARGET serial_basic_demo POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E make_directory
                        "$<TARGET_FILE_DIR:serial_basic_demo>/plugins/platforms/")
                add_custom_command(TARGET serial_basic_demo POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy
                        "${_qt_platform_plugin}"
                        "$<TARGET_FILE_DIR:serial_basic_demo>/plugins/platforms/")
            endif()

            foreach(QT_LIB Core SerialPort)
                set(_qt_bin_dll "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll")
                if (EXISTS "${_qt_bin_dll}")
                    add_custom_command(TARGET serial_basic_demo POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${_qt_bin_dll}"
                            "$<TARGET_FILE_DIR:serial_basic_demo>")
                endif()
            endforeach(QT_LIB)
        endif()
    endif()
endif()
