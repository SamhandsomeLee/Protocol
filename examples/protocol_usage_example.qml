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
                    Switch {
                        id: ancSwitch
                        text: "ANC 主动降噪"
                        onToggled: {
                            protocolAdapter.sendParameterUpdate("anc.enabled", checked)
                        }
                    }

                    Switch {
                        id: encSwitch
                        text: "ENC 发动机噪声消除"
                        onToggled: {
                            protocolAdapter.sendParameterUpdate("enc.enabled", checked)
                        }
                    }

                    Switch {
                        id: rncSwitch
                        text: "RNC 路噪消除"
                        onToggled: {
                            protocolAdapter.sendParameterUpdate("rnc.enabled", checked)
                        }
                    }
                }
            }

            // Alpha参数组
            GroupBox {
                title: "Alpha 参数"
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    columnSpacing: 20
                    rowSpacing: 10

                    Label { text: "Alpha1:" }
                    SpinBox {
                        id: alpha1SpinBox
                        from: 0
                        to: 4294967295
                        onValueChanged: {
                            protocolAdapter.sendParameterUpdate("tuning.alpha.alpha1", value)
                        }
                    }

                    Label { text: "Alpha2:" }
                    SpinBox {
                        id: alpha2SpinBox
                        from: 0
                        to: 4294967295
                        onValueChanged: {
                            protocolAdapter.sendParameterUpdate("tuning.alpha.alpha2", value)
                        }
                    }

                    Label { text: "Alpha3:" }
                    SpinBox {
                        id: alpha3SpinBox
                        from: 0
                        to: 4294967295
                        onValueChanged: {
                            protocolAdapter.sendParameterUpdate("tuning.alpha.alpha3", value)
                        }
                    }

                    Label { text: "Alpha4:" }
                    SpinBox {
                        id: alpha4SpinBox
                        from: 0
                        to: 4294967295
                        onValueChanged: {
                            protocolAdapter.sendParameterUpdate("tuning.alpha.alpha4", value)
                        }
                    }

                    Label { text: "Alpha5:" }
                    SpinBox {
                        id: alpha5SpinBox
                        from: 0
                        to: 4294967295
                        onValueChanged: {
                            protocolAdapter.sendParameterUpdate("tuning.alpha.alpha5", value)
                        }
                    }
                }
            }

            // Set1参数组
            GroupBox {
                title: "Set1 参数"
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    columnSpacing: 20
                    rowSpacing: 10

                    Label { text: "Gamma:" }
                    SpinBox {
                        id: gammaSpinBox
                        from: 0
                        to: 4294967295
                        onValueChanged: {
                            protocolAdapter.sendParameterUpdate("tuning.set1.gamma", value)
                        }
                    }

                    Label { text: "Eta:" }
                    SpinBox {
                        id: etaSpinBox
                        from: 0
                        to: 4294967295
                        onValueChanged: {
                            protocolAdapter.sendParameterUpdate("tuning.set1.eta", value)
                        }
                    }

                    Label { text: "Delta:" }
                    SpinBox {
                        id: deltaSpinBox
                        from: 0
                        to: 4294967295
                        onValueChanged: {
                            protocolAdapter.sendParameterUpdate("tuning.set1.delta", value)
                        }
                    }

                    Label { text: "扬声器数量:" }
                    SpinBox {
                        id: spkNumSpinBox
                        from: 0
                        to: 16
                        onValueChanged: {
                            protocolAdapter.sendParameterUpdate("tuning.set1.spk_num", value)
                        }
                    }

                    Label { text: "输出幅度:" }
                    SpinBox {
                        id: outputAmpSpinBox
                        from: 0
                        to: 4294967295
                        onValueChanged: {
                            protocolAdapter.sendParameterUpdate("tuning.set1.output_amplitude", value)
                        }
                    }
                }
            }

            // 系统控制
            GroupBox {
                title: "系统控制"
                Layout.fillWidth: true

                Switch {
                    id: checkModeSwitch
                    text: "检测模式"
                    onToggled: {
                        protocolAdapter.sendParameterUpdate("system.check_mode", checked)
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
            text: "批量发送Alpha参数"
            Layout.fillWidth: true
            onClicked: {
                var paths = [
                    "tuning.alpha.alpha1",
                    "tuning.alpha.alpha2",
                    "tuning.alpha.alpha3",
                    "tuning.alpha.alpha4",
                    "tuning.alpha.alpha5"
                ]
                var values = {
                    "tuning.alpha.alpha1": alpha1SpinBox.value,
                    "tuning.alpha.alpha2": alpha2SpinBox.value,
                    "tuning.alpha.alpha3": alpha3SpinBox.value,
                    "tuning.alpha.alpha4": alpha4SpinBox.value,
                    "tuning.alpha.alpha5": alpha5SpinBox.value
                }
                protocolAdapter.sendParameterGroup(paths, values)
            }
        }

        Button {
            text: "批量发送Set1参数"
            Layout.fillWidth: true
            onClicked: {
                var paths = [
                    "tuning.set1.gamma",
                    "tuning.set1.eta",
                    "tuning.set1.delta",
                    "tuning.set1.spk_num",
                    "tuning.set1.output_amplitude"
                ]
                var values = {
                    "tuning.set1.gamma": gammaSpinBox.value,
                    "tuning.set1.eta": etaSpinBox.value,
                    "tuning.set1.delta": deltaSpinBox.value,
                    "tuning.set1.spk_num": spkNumSpinBox.value,
                    "tuning.set1.output_amplitude": outputAmpSpinBox.value
                }
                protocolAdapter.sendParameterGroup(paths, values)
            }
        }

        Button {
            text: "获取支持的参数"
            Layout.fillWidth: true
            onClicked: {
                var params = protocolAdapter.getSupportedParameters()
                console.log("支持的参数:", params.join(", "))
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
            console.log("协议版本不匹配 - 期望:", expected, "实际:", actual)
        }

        function onDataReceived(data) {
            console.log("接收到数据，长度:", data.length)
        }
    }
}
