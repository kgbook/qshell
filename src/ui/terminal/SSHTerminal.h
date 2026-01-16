#ifndef QSHELL_SSH_TERMINAL_H
#define QSHELL_SSH_TERMINAL_H

#include <QDir>
#include <QFile>
#include <QSocketNotifier>
#include "BaseTerminal.h"
#include "core/SessionData.h"

#if defined(Q_CC_MSVC)
#include <winsock2.h>
#include <libssh2.h>
#endif

class SSHTerminal : public BaseTerminal {
    Q_OBJECT
public:
    explicit SSHTerminal(const SessionData &session, QWidget *parent = nullptr);
    ~SSHTerminal() override;
    void connect() override;
    void disconnect() override;

private:
    SessionData sessionData_;

#if defined(Q_CC_MSVC)
private:
    bool createSocket();
    bool connectToHost();
    bool initSession();
    bool verifyHostKey();
    bool authenticate();
    bool openChannel();

    QString getKnownHostsPath();
    void cleanup();
    void sendData(const QByteArray &data);
    void resizePty(int cols, int rows);
    void readAvailableData();

private slots:
    void onSocketReadyRead();

private:
    SOCKET sock_ = INVALID_SOCKET;
    LIBSSH2_SESSION *session_ = nullptr;
    LIBSSH2_CHANNEL *channel_ = nullptr;
    bool running_ = false;
    QSocketNotifier *readNotifier_ = nullptr;
#endif
};

#endif //QSHELL_SSH_TERMINAL_H
