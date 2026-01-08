#ifndef IPROTOCOL_H
#define IPROTOCOL_H

#include <QObject>
#include <QSize>

class IProtocol : public QObject {
    Q_OBJECT

public:
    explicit IProtocol(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IProtocol() = default;

    // 连接/断开
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    // 发送数据到远端
    virtual void sendData(const QByteArray& data) = 0;

    // 调整终端大小
    virtual void resize(int cols, int rows) = 0;

    signals:
        // 从远端接收到数据
        void dataReceived(const QByteArray& data);

    // 连接状态改变
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
};

#endif // IPROTOCOL_H
