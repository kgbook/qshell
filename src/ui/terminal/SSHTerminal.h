#ifndef QSHELL_SSH_TERMINAL_H
#define QSHELL_SSH_TERMINAL_H

#include "BaseTerminal.h"
#include "core/datatype.h"
#include <QDir>
#include <QFile>
#include <QSocketNotifier>

#if defined(Q_CC_MSVC)
#include <winsock2.h>
#include <libssh2.h>
#include <vector>
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
    // ====== X11 Forwarding ======
    struct X11Forward {
        LIBSSH2_CHANNEL *chan = nullptr;          // 远端 X11 子通道
        SOCKET           xsock = INVALID_SOCKET;   // 本地到 VcXsrv 的 socket
        QSocketNotifier *notifier = nullptr;       // 监听 xsock 的可读事件
    };

    static void x11Callback(LIBSSH2_SESSION *session,
                            LIBSSH2_CHANNEL *channel,
                            char *shost, int sport,
                            void **abstract);
    void handleNewX11Channel(LIBSSH2_CHANNEL *chan);
    void pumpRemoteX11();

private:
    SOCKET sock_ = INVALID_SOCKET;
    LIBSSH2_SESSION *session_ = nullptr;
    LIBSSH2_CHANNEL *channel_ = nullptr;
    bool running_ = false;
    QSocketNotifier *readNotifier_ = nullptr;

    // BaseTerminal 里已有 connect_；此处沿用其语义（原代码已使用）
    // bool connect_ = false; // 不重复声明

    std::vector<X11Forward*> x11Chans_;
#endif
};

#endif //QSHELL_SSH_TERMINAL_H
