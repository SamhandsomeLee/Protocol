#ifndef ERNC_PROTOCOL_V2_QSERIALTRANSPORT_H
#define ERNC_PROTOCOL_V2_QSERIALTRANSPORT_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <functional>
#include "ProtocolEngine.h"

namespace ernc::v2 {

class QSerialTransport : public QObject, public ITransport {
    Q_OBJECT
public:
    explicit QSerialTransport(const QString& portName,
                              int baudRate,
                              QObject* parent = nullptr);

    bool open();
    void close();

    // ITransport
    bool send(const uint8_t* data, size_t len) override;

    void setReceiveHandler(std::function<void(const uint8_t*, size_t)> handler);

private slots:
    void onReadyRead();

private:
    QSerialPort serial_;
    QByteArray rxBuffer_;
    std::function<void(const uint8_t*, size_t)> receiveHandler_;
};

} // namespace ernc::v2

#endif // ERNC_PROTOCOL_V2_QSERIALTRANSPORT_H


