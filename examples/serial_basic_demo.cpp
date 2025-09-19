#include <QCoreApplication>
#include <QTimer>
#include <QString>
#include <iostream>

#include "engine/ProtocolEngine.h"
#include "engine/ProtocolTables.h"
#include "transport/QSerialTransport.h"
#include "messages/ERNC_praram.pb.h"

using namespace ernc::v2;

static void print_usage(const char* exe) {
    std::cout << "Usage: " << exe << " <serial_port> [baudrate]\n"
              << "  Example: " << exe << " COM3 115200\n"
              << "           " << exe << " /dev/ttyUSB0 115200\n";
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    std::cout<<"argv[1]="<<argv[1]<<std::endl;
    if (argc >= 3) {
        std::cout<<"argv[2]="<<argv[2]<<std::endl;
    }
    const QString portName = QString::fromLocal8Bit(argv[1]);
    const int baud = (argc >= 3) ? QString::fromLocal8Bit(argv[2]).toInt() : 115200;

    std::cout<<"portName="<<portName.toStdString()<<std::endl;
    std::cout<<"baud="<<baud<<std::endl;

    QSerialTransport transport(portName, baud);

    EngineHooks hooks;
    hooks.on_error = [](int code, const char* msg){
        std::cerr << "[EngineError] code=" << code << ", msg=" << (msg ? msg : "") << "\n";
    };
    hooks.on_after_recv = [](ProtoID id, const void* payload){
        std::cout << "[Recv] proto=" << static_cast<int>(id) << " -> ";
        switch (id) {
            case ProtoID_MSG_CHECK_MOD: {
                const MSG_CheckMod* m = reinterpret_cast<const MSG_CheckMod*>(payload);
                std::cout << "MSG_CheckMod{check_mod=" << m->check_mod << "}" << "\n";
                break;
            }
            case ProtoID_MSG_ANC_SWITCH: {
                const MSG_AncSwitch* m = reinterpret_cast<const MSG_AncSwitch*>(payload);
                std::cout << "MSG_AncSwitch{anc_off=" << (m->anc_off ? 1 : 0)
                          << ", enc_off=" << (m->enc_off ? 1 : 0)
                          << ", rnc_off=" << (m->rnc_off ? 1 : 0) << "}" << "\n";
                break;
            }
            case ProtoID_MSG_CHANNEL_NUMBER: {
                const MSG_ChannelNumber* m = reinterpret_cast<const MSG_ChannelNumber*>(payload);
                std::cout << "MSG_ChannelNumber{ReferNum=" << m->ReferNum
                          << ", ErrNum=" << m->ErrNum
                          << ", SpkNum=" << m->SpkNum << "}" << "\n";
                break;
            }
            case ProtoID_MSG_CHANNEL_AMPLITUDE: {
                const MSG_ChannelAmplitude* m = reinterpret_cast<const MSG_ChannelAmplitude*>(payload);
                std::cout << "MSG_ChannelAmplitude{InputAmplitude[0]="
                          << m->InputAmplitude[0]
                          << ", OutputAmplitude=" << m->OutputAmplitude << "}" << "\n";
                break;
            }
            case ProtoID_MSG_VEHICLE_STATE: {
                const MSG_VehicleState* m = reinterpret_cast<const MSG_VehicleState*>(payload);
                std::cout << "MSG_VehicleState{speed=" << m->speed
                          << ", EngineSpeed=" << m->EngineSpeed
                          << ", AC=" << m->AC
                          << ", gear=" << m->gear
                          << ", drive_mod=" << m->drive_mod << "}" << "\n";
                break;
            }
            default: {
                std::cout << "(no pretty-printer, raw payload received)" << "\n";
                break;
            }
        }
    };


    ProtocolEngine engine(MESSAGE_TABLE, MESSAGE_COUNT,
                          UNION_TABLE, UNION_COUNT,
                          &transport, hooks);

    transport.setReceiveHandler([&](const uint8_t* data, size_t len){
        engine.onReceive(data, len);
    });

    if (!transport.open()) {
        std::cerr << "Failed to open serial port: " << portName.toStdString() << "\n";
        return 2;
    }

    std::cout << "[Listening] " << portName.toStdString() << ", " << baud << " baud. Press Ctrl+C to exit." << std::endl;

    // std::cout<<"engine created"<<std::endl;
    // std::cout<<"send CHECK_MOD request"<<std::endl;
    // MSG_CheckMod msg = MSG_CheckMod_init_zero;
    // msg.check_mod = 1;
    // bool ok = engine.sendMessage(ProtoID_MSG_CHECK_MOD, FunCode_FUN_REQUEST, &msg);
    // std::cout << "[Send] CHECK_MOD request, ok=" << (ok ? "true" : "false") << "\n";

    // // 在启动后 500ms 发送一条最小消息：MSG_CheckMod { check_mod = 1 }
    // QTimer::singleShot(500, [&engine]{
    //     std::cout<<"send CHECK_MOD request"<<std::endl;
    //     MSG_CheckMod msg = MSG_CheckMod_init_zero;
    //     msg.check_mod = 1;
    //     bool ok = engine.sendMessage(ProtoID_MSG_CHECK_MOD, FunCode_FUN_REQUEST, &msg);
    //     std::cout << "[Send] CHECK_MOD request, ok=" << (ok ? "true" : "false") << "\n";
    // });
    //
    // // 5 秒后发送结束命令 check_mod=3，然后退出
    // QTimer::singleShot(5000, [&engine]{
    //     std::cout<<"send CHECK_MOD stop"<<std::endl;
    //     MSG_CheckMod msg = MSG_CheckMod_init_zero;
    //     msg.check_mod = 3;
    //     bool ok = engine.sendMessage(ProtoID_MSG_CHECK_MOD, FunCode_FUN_REQUEST, &msg);
    //     std::cout << "[Send] CHECK_MOD stop, ok=" << (ok ? "true" : "false") << "\n";
    // });

    return app.exec();
}


