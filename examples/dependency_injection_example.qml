import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * @brief 协议适配器QML使用示例 - 依赖注入版本
 *
 * 展示如何在QML中使用重构后的ProtocolAdapter和传输层
 */
ApplicationWindow {
    id: root
    width: 900
    height: 700
    visible: true
    title: "ERNC Protocol Adapter v3.0 - Dependency Injection Architecture"

    // 协议适配器实例（需要在C++中注册到QML）
    property var protocolAdapter: null
    property var serialTransport: null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // 标题
        Text {
            text: "ERNC 协议适配器 v3.0 - 依赖注入架构"
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }
        
        // 新版本特性
        Text {
            text: "新增: 18种消息类型 | ProtoID: 0-158 | 车辆状态集成 | 层次化参数"
            font.pixelSize: 12
            color: "blue"
            Layout.alignment: Qt.AlignHCenter
        }

        // 架构说明
        GroupBox {
            title: "架构特点"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 5

                Text {
                    text: "✅ 传输层与协议层完全分离"
                    color: "green"
                }
                Text {
                    text: "✅ 支持多种传输方式（串口、TCP、UDP、模拟传输）"
                    color: "green"
                }
                Text {
                    text: "✅ 18种新消息类型支持（ANC、车辆状态、通道配置等）"
                    color: "green"
                }
                Text {
                    text: "✅ 层次化参数结构与完整的ProtoID映射"
                    color: "green"
                }
                Text {
                    text: "✅ 运行时可切换传输层及参数验证"
                    color: "green"
                }
            }
        }

        // 传输层控制区域
        GroupBox {
            title: "1. 传输层管理"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                Text {
                    text: "步骤1: 创建并配置传输层"
                    font.bold: true
                }

                RowLayout {
                    TextField {
                        id: portNameField
                        placeholderText: "串口名称 (如: COM3)"
                        text: "COM3"
                        Layout.fillWidth: true
                    }

                    SpinBox {
                        id: baudRateSpinBox
                        from: 9600
                        to: 921600
                        value: 115200
                        stepSize: 9600
                    }

                    Button {
                        text: "创建SerialTransport"
                        enabled: !serialTransport
                        onClicked: {
                            // 在实际应用中，这里会调用C++代码创建SerialTransport
                            console.log("Creating SerialTransport:", portNameField.text, baudRateSpinBox.value)
                            transportStatus.text = "SerialTransport已创建: " + portNameField.text + "@" + baudRateSpinBox.value
                            transportStatus.color = "blue"
                            // serialTransport = SerialTransportFactory.create(portNameField.text, baudRateSpinBox.value)
                        }
                    }
                }

                RowLayout {
                    Button {
                        text: "连接传输层"
                        enabled: serialTransport
                        onClicked: {
                            console.log("Opening transport connection...")
                            transportStatus.text = "传输层已连接: " + portNameField.text
                            transportStatus.color = "green"
                            // serialTransport.open()
                        }
                    }

                    Button {
                        text: "断开传输层"
                        onClicked: {
                            console.log("Closing transport connection...")
                            transportStatus.text = "传输层已断开"
                            transportStatus.color = "orange"
                            // serialTransport.close()
                        }
                    }

                    Button {
                        text: "删除传输层"
                        onClicked: {
                            console.log("Destroying transport...")
                            transportStatus.text = "传输层已删除"
                            transportStatus.color = "red"
                            // serialTransport = null
                        }
                    }
                }

                Text {
                    id: transportStatus
                    text: "未创建传输层"
                    color: "gray"
                    font.italic: true
                }
            }
        }

        // 协议适配器控制区域
        GroupBox {
            title: "2. 协议适配器管理"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                Text {
                    text: "步骤2: 将传输层注入到协议适配器"
                    font.bold: true
                }

                RowLayout {
                    Button {
                        text: "注入传输层"
                        enabled: serialTransport && protocolAdapter
                        onClicked: {
                            console.log("Injecting transport into protocol adapter...")
                            adapterStatus.text = "传输层已注入到协议适配器"
                            adapterStatus.color = "green"
                            // protocolAdapter.setTransport(serialTransport)
                        }
                    }

                    Button {
                        text: "移除传输层"
                        onClicked: {
                            console.log("Removing transport from protocol adapter...")
                            adapterStatus.text = "协议适配器无传输层"
                            adapterStatus.color = "orange"
                            // protocolAdapter.setTransport(null)
                        }
                    }

                    Button {
                        text: "查询传输信息"
                        onClicked: {
                            console.log("Querying transport information...")
                            // var desc = protocolAdapter.transportDescription()
                            adapterStatus.text = "传输层描述: SerialTransport COM3@115200"
                            adapterStatus.color = "blue"
                        }
                    }
                }

                Text {
                    id: adapterStatus
                    text: "协议适配器未连接传输层"
                    color: "gray"
                    font.italic: true
                }
            }
        }

        // 参数操作区域
        GroupBox {
            title: "3. 参数操作示例"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                Text {
                    text: "步骤3: 使用协议适配器进行参数操作"
                    font.bold: true
                }

                // ANC/ENC/RNC控制 (新的参数结构)
                RowLayout {
                    Text { text: "ANC/ENC/RNC控制 (ProtoID 151):" }
                    Switch {
                        id: ancSwitch
                        onToggled: {
                            console.log("Sending ANC_SWITCH parameter with new structure:")
                            var ancParams = {
                                "anc_off": !checked,
                                "enc_off": !encSwitch.checked,
                                "rnc_off": !rncSwitch.checked
                            }
                            console.log("ANC params:", JSON.stringify(ancParams))
                            // protocolAdapter.sendParameterUpdate("anc.enabled", ancParams)
                        }
                    }
                    Switch {
                        id: encSwitch
                        onToggled: {
                            console.log("ENC toggled:", checked)
                        }
                    }
                    Switch {
                        id: rncSwitch
                        onToggled: {
                            console.log("RNC toggled:", checked)
                        }
                    }
                    Text { text: "ANC:" + (ancSwitch.checked ? "ON" : "OFF") + " ENC:" + (encSwitch.checked ? "ON" : "OFF") + " RNC:" + (rncSwitch.checked ? "ON" : "OFF") }
                }

                // 车辆状态参数 (新增)
                RowLayout {
                    Text { text: "车辆状态 (ProtoID 138):" }
                    Button {
                        text: "发送车辆数据"
                        onClicked: {
                            console.log("Sending VEHICLE_STATE parameter with new structure...")
                            var vehicleParams = {
                                "speed": 75,
                                "engine_speed": 1800
                            }
                            console.log("Vehicle params:", JSON.stringify(vehicleParams))
                            // protocolAdapter.sendParameterUpdate("vehicle.speed", vehicleParams)
                        }
                    }
                }
                
                // RNC Alpha参数组 (更新)
                RowLayout {
                    Text { text: "RNC Alpha参数 (ProtoID 158):" }
                    Button {
                        text: "发送RNC Alpha组"
                        onClicked: {
                            console.log("Sending ALPHA_PARAMS with new structure...")
                            var rncParams = {
                                "alpha1": 110,
                                "alpha2": 160,
                                "alpha3": 210
                            }
                            console.log("RNC Alpha params:", JSON.stringify(rncParams))
                            // protocolAdapter.sendParameterUpdate("rnc.alpha1", rncParams)
                        }
                    }
                }
                
                // 通道配置参数 (新增)
                RowLayout {
                    Text { text: "通道配置 (ProtoID 0):" }
                    Button {
                        text: "配置通道"
                        onClicked: {
                            console.log("Sending CHANNEL_NUMBER parameter...")
                            var channelParams = {
                                "refer_num": 6,
                                "error_num": 12
                            }
                            console.log("Channel params:", JSON.stringify(channelParams))
                            // protocolAdapter.sendParameterUpdate("channel.refer_num", channelParams)
                        }
                    }
                }

                // ERNC协议信息
                RowLayout {
                    Text { text: "ERNC协议信息:" }
                    Button {
                        text: "查询协议版本"
                        onClicked: {
                            console.log("ERNC Protocol version: 3.0.0")
                            console.log("Supported message types: 18")
                            console.log("ProtoID range: 0-158")
                            // var version = protocolAdapter.getProtocolVersion()
                        }
                    }
                    Button {
                        text: "查询新参数"
                        onClicked: {
                            console.log("New ERNC parameters: 70+ hierarchical parameters")
                            console.log("Key categories: ANC control, Vehicle state, Channel config, RNC params, ENC params")
                            // var params = protocolAdapter.getSupportedParameters()
                        }
                    }
                }
            }
        }

        // 高级功能区域
        GroupBox {
            title: "4. 高级功能演示"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                Text {
                    text: "步骤4: 运行时传输层切换"
                    font.bold: true
                }

                RowLayout {
                    Button {
                        text: "切换到TCP传输"
                        onClicked: {
                            console.log("Switching to TCP transport...")
                            // var tcpTransport = TcpTransportFactory.create("192.168.1.100", 8080)
                            // protocolAdapter.setTransport(tcpTransport)
                            transportStatus.text = "已切换到TCP传输: 192.168.1.100:8080"
                            transportStatus.color = "purple"
                        }
                    }

                    Button {
                        text: "切换到模拟传输"
                        onClicked: {
                            console.log("Switching to mock transport...")
                            // var mockTransport = MockTransportFactory.create()
                            // protocolAdapter.setTransport(mockTransport)
                            transportStatus.text = "已切换到模拟传输（用于测试）"
                            transportStatus.color = "cyan"
                        }
                    }
                }

                Text {
                    text: "💡 新版本提示: ERNC v3.0支持18种消息类型，传输层可动态切换，层次化参数结构支持复杂配置"
                    color: "blue"
                    font.italic: true
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        // 日志区域
        GroupBox {
            title: "操作日志"
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollView {
                anchors.fill: parent
                TextArea {
                    id: logArea
                    readOnly: true
                    text: "=== ERNC v3.0 依赖注入架构日志 ===\n" +
                          "✅ ERNC协议系统初始化完成\n" +
                          "📋 18种消息类型已加载 (ProtoID: 0-158)\n" +
                          "📋 支持层次化参数结构\n" +
                          "📋 等待用户操作...\n"
                    wrapMode: TextArea.Wrap
                }
            }
        }
    }

    // 连接状态管理
    QtObject {
        id: connectionStatus
        property bool connected: false
        property string status: "未连接"
    }

    Component.onCompleted: {
        console.log("ERNC v3.0 Dependency Injection Example loaded")
        logArea.append("🎯 ERNC v3.0 依赖注入架构示例已加载")
        logArea.append("💫 新特性: 18种消息类型, 车辆状态集成, 层次化参数")
        logArea.append("📍 ProtoID范围: 0-158, 支持ANC/ENC/RNC/车辆/通道等全部参数")
        logArea.append("📖 请按照步骤1-4的顺序操作")
    }
}
