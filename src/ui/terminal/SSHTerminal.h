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
    // ===== X11 Forwarding（Windows） =====
    struct X11Forward {
        LIBSSH2_CHANNEL *chan = nullptr;          // 远端 X11 子通道
        SOCKET           xsock = INVALID_SOCKET;   // 本地到 VcXsrv 的 TCP
        QSocketNotifier *notifier = nullptr;       // xsock 可读
        QSocketNotifier *writable = nullptr;       // xsock 可写（背压用）
        QByteArray toLocal;                        // 远端→本地 待写缓冲
        QByteArray toRemote;                       // 本地→远端 待写缓冲
    };

    static void x11Callback(LIBSSH2_SESSION *session,
                            LIBSSH2_CHANNEL *channel,
                            char *shost, int sport,
                            void **abstract);
    void handleNewX11Channel(LIBSSH2_CHANNEL *chan);
    void pumpRemoteX11();
    void flushX11ToLocal(X11Forward *xf);
    void flushX11ToRemote(X11Forward *xf);

private:
    SOCKET sock_ = INVALID_SOCKET;
    LIBSSH2_SESSION *session_ = nullptr;
    LIBSSH2_CHANNEL *channel_ = nullptr;
    bool running_ = false;
    QSocketNotifier *readNotifier_ = nullptr;

    std::vector<X11Forward*> x11Chans_;
#endif
};

#endif //QSHELL_SSH_TERMINAL_H
