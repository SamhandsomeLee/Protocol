import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * 协议适配器使用示例
 * 展示如何在QML中使用ProtocolAdapter进行参数控制
 */
ApplicationWindow {
    id: root
    width: 800
    height: 600
    title: "ERNC Protocol Adapter Example"
    visible: true

    // 连接状态指示
    Rectangle {
        id: connectionStatus
        width: parent.width
        height: 40
        color: protocolAdapter.isConnected() ? "#4CAF50" : "#F44336"

        Text {
            anchors.centerIn: parent
            text: protocolAdapter.isConnected() ? "已连接" : "未连接"
            color: "white"
            font.bold: true
        }
    }

    ScrollView {
        anchors.top: connectionStatus.bottom
        anchors.bottom: controlButtons.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10

        ColumnLayout {
            width: root.width - 20
            spacing: 20

            // 连接控制组
            GroupBox {
                title: "连接控制"
                Layout.fillWidth: true

                RowLayout {
                    TextField {
                        id: portNameField
                        placeholderText: "COM端口 (如: COM3)"
                        text: "COM3"
                        Layout.preferredWidth: 150
                    }

                    SpinBox {
                        id: baudRateSpinBox
                        from: 9600
                        to: 921600
                        value: 115200
                        stepSize: 9600
                    }

                    Button {
                        text: protocolAdapter.isConnected() ? "断开连接" : "连接"
                        onClicked: {
                            if (protocolAdapter.isConnected()) {
                                protocolAdapter.closeConnection()
                            } else {
                                protocolAdapter.openConnection(portNameField.text, baudRateSpinBox.value)
                            }
                        }
                    }
                }
            }

            // 噪声控制组
            GroupBox {
                title: "噪声控制"
                Layout.fillWidth: true

                ColumnLayout {
                    RowLayout {
                        Switch {
                            id: ancSwitch
                            text: "ANC 主动降噪"
                            onToggled: {
                                // 使用新的ANC_SWITCH消息类型参数结构
                                var ancParams = {
                                    "anc_off": !checked,
                                    "enc_off": !encSwitch.checked,
                                    "rnc_off": !rncSwitch.checked
                                }
                                protocolAdapter.sendParameterUpdate("anc.enabled", ancParams)
                            }
                        }
                        Text {
                            text: "(ProtoID: 151)"
                            color: "gray"
                            font.pixelSize: 10
                        }
                    }

                    RowLayout {
                        Switch {
                            id: encSwitch
                            text: "ENC 发动机噪声消除"
                            onToggled: {
                                var ancParams = {
                                    "anc_off": !ancSwitch.checked,
                                    "enc_off": !checked,
                                    "rnc_off": !rncSwitch.checked
                                }
                                protocolAdapter.sendParameterUpdate("anc.enabled", ancParams)
                            }
                        }
                        Text {
                            text: "(ProtoID: 151)"
                            color: "gray"
                            font.pixelSize: 10
                        }
                    }

                    RowLayout {
                        Switch {
                            id: rncSwitch
                            text: "RNC 路噪消除"
                            onToggled: {
                                var ancParams = {
                                    "anc_off": !ancSwitch.checked,
                                    "enc_off": !encSwitch.checked,
                                    "rnc_off": !checked
                                }
                                protocolAdapter.sendParameterUpdate("anc.enabled", ancParams)
                            }
                        }
                        Text {
                            text: "(ProtoID: 151)"
                            color: "gray"
                            font.pixelSize: 10
                        }
                    }
                }
            }

            // 车辆状态监控组 (新增)
            GroupBox {
                title: "车辆状态监控 (VEHICLE_STATE - ProtoID: 138)"
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    columnSpacing: 20
                    rowSpacing: 10

                    Label { text: "车速 (km/h):" }
                    SpinBox {
                        id: vehicleSpeedSpinBox
                        from: 0
                        to: 250
                        value: 60
                        onValueChanged: {
                            var vehicleParams = {
                                "speed": value,
                                "engine_speed": engineSpeedSpinBox.value
                            }
                            protocolAdapter.sendParameterUpdate("vehicle.speed", vehicleParams)
                        }
                    }

                    Label { text: "发动机转速 (rpm):" }
                    SpinBox {
                        id: engineSpeedSpinBox
                        from: 0
                        to: 5000
                        value: 1500
                        onValueChanged: {
                            var vehicleParams = {
                                "speed": vehicleSpeedSpinBox.value,
                                "engine_speed": value
                            }
                            protocolAdapter.sendParameterUpdate("vehicle.speed", vehicleParams)
                        }
                    }
                }
            }

            // RNC Alpha参数组 (更新)
            GroupBox {
                title: "RNC Alpha 参数 (ALPHA_PARAMS - ProtoID: 158)"
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    columnSpacing: 20
                    rowSpacing: 10

                    Label { text: "RNC Alpha1:" }
                    SpinBox {
                        id: alpha1SpinBox
                        from: 0
                        to: 500
                        value: 100
                        onValueChanged: {
                            var rncParams = {
                                "alpha1": value,
                                "alpha2": alpha2SpinBox.value,
                                "alpha3": alpha3SpinBox.value
                            }
                            protocolAdapter.sendParameterUpdate("rnc.alpha1", rncParams)
                        }
                    }

                    Label { text: "RNC Alpha2:" }
                    SpinBox {
                        id: alpha2SpinBox
                        from: 0
                        to: 500
                        value: 150
                        onValueChanged: {
                            var rncParams = {
                                "alpha1": alpha1SpinBox.value,
                                "alpha2": value,
                                "alpha3": alpha3SpinBox.value
                            }
                            protocolAdapter.sendParameterUpdate("rnc.alpha1", rncParams)
                        }
                    }

                    Label { text: "RNC Alpha3:" }
                    SpinBox {
                        id: alpha3SpinBox
                        from: 0
                        to: 500
                        value: 200
                        onValueChanged: {
                            var rncParams = {
                                "alpha1": alpha1SpinBox.value,
                                "alpha2": alpha2SpinBox.value,
                                "alpha3": value
                            }
                            protocolAdapter.sendParameterUpdate("rnc.alpha1", rncParams)
                        }
                    }
                }
            }

            // 通道配置组 (新增)
            GroupBox {
                title: "通道配置 (CHANNEL_NUMBER - ProtoID: 0)"
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    columnSpacing: 20
                    rowSpacing: 10

                    Label { text: "参考通道数:" }
                    SpinBox {
                        id: referChannelSpinBox
                        from: 1
                        to: 16
                        value: 4
                        onValueChanged: {
                            var channelParams = {
                                "refer_num": value,
                                "error_num": errorChannelSpinBox.value
                            }
                            protocolAdapter.sendParameterUpdate("channel.refer_num", channelParams)
                        }
                    }

                    Label { text: "误差通道数:" }
                    SpinBox {
                        id: errorChannelSpinBox
                        from: 1
                        to: 32
                        value: 8
                        onValueChanged: {
                            var channelParams = {
                                "refer_num": referChannelSpinBox.value,
                                "error_num": value
                            }
                            protocolAdapter.sendParameterUpdate("channel.refer_num", channelParams)
                        }
                    }
                }
            }

            // ENC 2阶参数组 (新增)
            GroupBox {
                title: "ENC 2阶参数 (ORDER2_PARAMS - ProtoID: 78)"
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    columnSpacing: 20
                    rowSpacing: 10

                    Label { text: "2阶参数1:" }
                    SpinBox {
                        id: order2Param1SpinBox
                        from: 0
                        to: 1000
                        value: 50
                        onValueChanged: {
                            var order2Params = {
                                "param1": value,
                                "param2": order2Param2SpinBox.value
                            }
                            protocolAdapter.sendParameterUpdate("order2.params", order2Params)
                        }
                    }

                    Label { text: "2阶参数2:" }
                    SpinBox {
                        id: order2Param2SpinBox
                        from: 0
                        to: 1000
                        value: 75
                        onValueChanged: {
                            var order2Params = {
                                "param1": order2Param1SpinBox.value,
                                "param2": value
                            }
                            protocolAdapter.sendParameterUpdate("order2.params", order2Params)
                        }
                    }
                }
            }

            // 系统控制 (更新)
            GroupBox {
                title: "系统控制 (CHECK_MOD - ProtoID: 150)"
                Layout.fillWidth: true

                Switch {
                    id: checkModeSwitch
                    text: "实时数据流检测模式"
                    onToggled: {
                        var checkParams = {
                            "check_enabled": checked
                        }
                        protocolAdapter.sendParameterUpdate("system.check_mode", checkParams)
                    }
                }
            }
        }
    }

    // 底部控制按钮
    RowLayout {
        id: controlButtons
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        height: 50

        Button {
            text: "批量发送RNC Alpha参数"
            Layout.fillWidth: true
            onClicked: {
                var paths = ["rnc.alpha1"]
                var rncParams = {
                    "alpha1": alpha1SpinBox.value,
                    "alpha2": alpha2SpinBox.value,
                    "alpha3": alpha3SpinBox.value
                }
                var values = {
                    "rnc.alpha1": rncParams
                }
                protocolAdapter.sendParameterGroup(paths, values)
            }
        }

        Button {
            text: "批量发送车辆+通道参数"
            Layout.fillWidth: true
            onClicked: {
                var paths = ["vehicle.speed", "channel.refer_num"]
                var vehicleParams = {
                    "speed": vehicleSpeedSpinBox.value,
                    "engine_speed": engineSpeedSpinBox.value
                }
                var channelParams = {
                    "refer_num": referChannelSpinBox.value,
                    "error_num": errorChannelSpinBox.value
                }
                var values = {
                    "vehicle.speed": vehicleParams,
                    "channel.refer_num": channelParams
                }
                protocolAdapter.sendParameterGroup(paths, values)
            }
        }

        Button {
            text: "获取新支持的参数"
            Layout.fillWidth: true
            onClicked: {
                var params = protocolAdapter.getSupportedParameters()
                console.log("ERNC协议支持的参数:", params.join(", "))
                console.log("新增的消息类型:", "ANC_SWITCH(151), VEHICLE_STATE(138), CHANNEL_NUMBER(0), ALPHA_PARAMS(158), ORDER2_PARAMS(78)")
            }
        }
    }

    // 连接协议适配器的信号
    Connections {
        target: protocolAdapter

        function onParameterAcknowledged(path) {
            console.log("参数确认:", path)
        }

        function onCommunicationError(error) {
            console.log("通信错误:", error)
        }

        function onConnectionStatusChanged(connected) {
            console.log("连接状态变化:", connected)
        }

        function onProtocolVersionMismatch(expected, actual) {
            console.log("ERNC协议版本不匹配 - 期望:", expected, "实际:", actual)
            console.log("当前支持18种消息类型，ProtoID范围: 0-158")
        }

        function onDataReceived(data) {
            console.log("接收到数据，长度:", data.length)
        }
    }
}
