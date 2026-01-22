#include "SSHTerminal.h"
#include "qtermwidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QCoreApplication>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

// ==================== SSHTerminal ====================

SSHTerminal::SSHTerminal(const SessionData &session, QWidget *parent)
    : BaseTerminal(parent), sessionData_(session) {

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    libssh2_init(0);

    // 终端输入 -> SSH发送
    QObject::connect(this, &QTermWidget::sendData, [this](const char *data, int size) {
        sendData(QByteArray(data, size));
    });

    // 终端大小改变时通知 SSH
    QObject::connect(this, &QTermWidget::termSizeChange, [this](int lines, int columns) {
        resizePty(columns, lines);
    });
}

SSHTerminal::~SSHTerminal() {
    disconnect();
    libssh2_exit();
    WSACleanup();
}

void SSHTerminal::connect() {
    qDebug() << "Connecting to" << sessionData_.sshConfig.host;

    if (!createSocket()) { cleanup(); return; }
    if (!connectToHost()) { cleanup(); return; }
    if (!initSession()) { cleanup(); return; }
    if (!verifyHostKey()) { cleanup(); return; }
    if (!authenticate()) { cleanup(); return; }
    if (!openChannel()) { cleanup(); return; }

    // 设置非阻塞模式
    libssh2_session_set_blocking(session_, 0);

    running_ = true;
    connect_ = true;

    // 使用 QSocketNotifier 监听 socket 可读事件
    readNotifier_ = new QSocketNotifier((qintptr)sock_, QSocketNotifier::Read, this);
    QObject::connect(readNotifier_, &QSocketNotifier::activated,
                     this, &SSHTerminal::onSocketReadyRead);
    readNotifier_->setEnabled(true);

    qDebug() << "SSH connection established to" << sessionData_.name;
}

void SSHTerminal::disconnect() {
    running_ = false;
    connect_ = false;

    if (readNotifier_) {
        readNotifier_->setEnabled(false);
        readNotifier_->deleteLater();
        readNotifier_ = nullptr;
    }

    cleanup();
    qDebug() << "SSH disconnected from" << sessionData_.name;
}


void SSHTerminal::onSocketReadyRead() {
    if (!running_ || !channel_) return;

    // 临时禁用通知，避免重入
    if (readNotifier_) {
        readNotifier_->setEnabled(false);
    }

    readAvailableData();  // 原有 shell 数据
    pumpRemoteX11();      // 新增：X11 子通道数据泵

    // 重新启用通知
    if (readNotifier_ && running_) {
        readNotifier_->setEnabled(true);
    }
}

void SSHTerminal::readAvailableData() {
    if (!channel_ || !running_) return;

    char buffer[65536];  // 增大缓冲区

    // 循环读取所有可用数据
    while (running_) {
        // 读取标准输出
        ssize_t bytesRead = libssh2_channel_read(channel_, buffer, sizeof(buffer));

        if (bytesRead > 0) {
            this->recvData(buffer, bytesRead);
            continue;  // 继续尝试读取更多数据
        } else if (bytesRead == LIBSSH2_ERROR_EAGAIN) {
            // 没有更多数据可读
            break;
        } else if (bytesRead < 0) {
            if (!libssh2_channel_eof(channel_)) {
                QMessageBox::critical(this, tr("SSH Error"), tr("Read error"));
                emit onSessionError(this);
                disconnect();
                return;
            }
            break;
        }

        // bytesRead == 0
        break;
    }

    // 读取标准错误
    while (running_) {
        ssize_t bytesRead = libssh2_channel_read_stderr(channel_, buffer, sizeof(buffer));

        if (bytesRead > 0) {
            this->recvData(buffer, bytesRead);
            continue;
        } else if (bytesRead == LIBSSH2_ERROR_EAGAIN || bytesRead <= 0) {
            break;
        }
    }

    // 检查连接是否关闭
    if (channel_ && libssh2_channel_eof(channel_)) {
        QMessageBox::information(this, tr("SSH"), tr("Connection closed by remote host"));
        emit onSessionError(this);
        disconnect();
    }
}

QString SSHTerminal::getKnownHostsPath() {
    QString homePath = QDir::homePath();
    QString sshDir = homePath + "/.ssh";

    QDir dir(sshDir);
    if (!dir.exists()) {
        dir.mkpath(sshDir);
    }

    return sshDir + "/known_hosts";
}

bool SSHTerminal::createSocket() {
    sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_ == INVALID_SOCKET) {
        QMessageBox::critical(this, tr("SSH Error"),
            tr("Failed to create socket: %1").arg(WSAGetLastError()));
        emit onSessionError(this);
        return false;
    }

    // 设置 socket 为非阻塞模式（连接完成后）
    DWORD timeout = 30000;
    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(sock_, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

    // 禁用 Nagle 算法，减少延迟
    int flag = 1;
    setsockopt(sock_, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag));

    return true;
}

bool SSHTerminal::connectToHost() {
    struct addrinfo hints = {0};
    struct addrinfo *result = nullptr;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    QString portStr = QString::number(sessionData_.sshConfig.port);

    int rc = getaddrinfo(
        sessionData_.sshConfig.host.toUtf8().constData(),
        portStr.toUtf8().constData(),
        &hints,
        &result
    );

    if (rc != 0) {
        QMessageBox::critical(this, tr("SSH Error"),
            tr("Failed to resolve hostname '%1'").arg(sessionData_.sshConfig.host));
        emit onSessionError(this);
        return false;
    }

    bool connected = false;
    for (struct addrinfo *ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        if (::connect(sock_, ptr->ai_addr, (int)ptr->ai_addrlen) == 0) {
            connected = true;
            break;
        }
    }

    freeaddrinfo(result);

    if (!connected) {
        QMessageBox::critical(this, tr("SSH Error"),
            tr("Failed to connect to %1:%2")
                .arg(sessionData_.sshConfig.host)
                .arg(sessionData_.sshConfig.port));
        emit onSessionError(this);
        return false;
    }

    qDebug() << "TCP connected to" << sessionData_.sshConfig.host;
    return true;
}

bool SSHTerminal::initSession() {
    session_ = libssh2_session_init();
    if (!session_) {
        QMessageBox::critical(this, tr("SSH Error"), tr("Failed to create SSH session"));
        emit onSessionError(this);
        return false;
    }

    libssh2_session_set_blocking(session_, 1);
    libssh2_session_set_timeout(session_, 30000);

    int rc = libssh2_session_handshake(session_, sock_);
    if (rc) {
        char *errmsg = nullptr;
        libssh2_session_last_error(session_, &errmsg, nullptr, 0);
        QMessageBox::critical(this, tr("SSH Error"),
            tr("SSH handshake failed: %1").arg(errmsg ? errmsg : "Unknown"));
        emit onSessionError(this);
        return false;
    }

    // —— 注册 X11 回调
    void **abs = libssh2_session_abstract(session_);
    *abs = this;
    libssh2_session_callback_set(session_, LIBSSH2_CALLBACK_X11, (void*)x11Callback);

    qDebug() << "SSH handshake completed";
    return true;
}

bool SSHTerminal::verifyHostKey() {
    LIBSSH2_KNOWNHOSTS *knownHosts = libssh2_knownhost_init(session_);
    if (!knownHosts) {
        qWarning() << "Failed to init known_hosts, continuing anyway";
        return true;
    }

    QString knownHostsPath = getKnownHostsPath();

    libssh2_knownhost_readfile(knownHosts, knownHostsPath.toUtf8().constData(),
                                LIBSSH2_KNOWNHOST_FILE_OPENSSH);

    size_t keyLen = 0;
    int keyType = 0;
    const char *key = libssh2_session_hostkey(session_, &keyLen, &keyType);

    if (!key) {
        libssh2_knownhost_free(knownHosts);
        QMessageBox::critical(this, tr("SSH Error"), tr("Failed to get host key"));
        emit onSessionError(this);
        return false;
    }

    const char *fingerprint = libssh2_hostkey_hash(session_, LIBSSH2_HOSTKEY_HASH_SHA256);
    QString fingerprintStr;
    if (fingerprint) {
        QByteArray fp(fingerprint, 32);
        fingerprintStr = fp.toBase64();
        qDebug() << "Host fingerprint (SHA256):" << fingerprintStr;
    }

    struct libssh2_knownhost *host = nullptr;
    int checkResult = libssh2_knownhost_checkp(
        knownHosts,
        sessionData_.sshConfig.host.toUtf8().constData(),
        sessionData_.sshConfig.port,
        key, keyLen,
        LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW,
        &host
    );

    bool result = true;

    switch (checkResult) {
        case LIBSSH2_KNOWNHOST_CHECK_MATCH:
            qDebug() << "Host key verified";
            break;

        case LIBSSH2_KNOWNHOST_CHECK_NOTFOUND: {
            qDebug() << "New host, adding to known_hosts";

            int typeFlag = LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW;

            switch (keyType) {
                case LIBSSH2_HOSTKEY_TYPE_RSA:
                    typeFlag |= LIBSSH2_KNOWNHOST_KEY_SSHRSA;
                    break;
                case LIBSSH2_HOSTKEY_TYPE_DSS:
                    typeFlag |= LIBSSH2_KNOWNHOST_KEY_SSHDSS;
                    break;
#ifdef LIBSSH2_HOSTKEY_TYPE_ECDSA_256
                case LIBSSH2_HOSTKEY_TYPE_ECDSA_256:
                    typeFlag |= LIBSSH2_KNOWNHOST_KEY_ECDSA_256;
                    break;
                case LIBSSH2_HOSTKEY_TYPE_ECDSA_384:
                    typeFlag |= LIBSSH2_KNOWNHOST_KEY_ECDSA_384;
                    break;
                case LIBSSH2_HOSTKEY_TYPE_ECDSA_521:
                    typeFlag |= LIBSSH2_KNOWNHOST_KEY_ECDSA_521;
                    break;
#endif
#ifdef LIBSSH2_HOSTKEY_TYPE_ED25519
                case LIBSSH2_HOSTKEY_TYPE_ED25519:
                    typeFlag |= LIBSSH2_KNOWNHOST_KEY_ED25519;
                    break;
#endif
                default:
                    typeFlag |= LIBSSH2_KNOWNHOST_KEY_UNKNOWN;
                    break;
            }

            int addResult = libssh2_knownhost_addc(
                knownHosts,
                sessionData_.sshConfig.host.toUtf8().constData(),
                nullptr, key, keyLen,
                nullptr, 0, typeFlag, nullptr
            );

            if (addResult == 0) {
                libssh2_knownhost_writefile(knownHosts, knownHostsPath.toUtf8().constData(),
                                            LIBSSH2_KNOWNHOST_FILE_OPENSSH);
                qDebug() << "Host key saved to" << knownHostsPath;
            }
            break;
        }

        case LIBSSH2_KNOWNHOST_CHECK_MISMATCH:
            QMessageBox::critical(this, tr("SSH Error"),
                tr("WARNING: HOST KEY MISMATCH!\n\n"
                   "The host key for '%1' has changed.\n"
                   "This could indicate a man-in-the-middle attack.\n\n"
                   "Fingerprint: %2\n\n"
                   "Remove the old entry from:\n%3")
                    .arg(sessionData_.sshConfig.host)
                    .arg(fingerprintStr)
                    .arg(knownHostsPath));
            emit onSessionError(this);
            result = false;
            break;

        default:
            qWarning() << "Known host check returned:" << checkResult;
            break;
    }

    libssh2_knownhost_free(knownHosts);
    return result;
}

bool SSHTerminal::authenticate() {
    int rc = -1;

    if (!sessionData_.sshConfig.privateKeyPath.isEmpty()) {
        QFile keyFile(sessionData_.sshConfig.privateKeyPath);
        if (keyFile.exists()) {
            qDebug() << "Trying public key authentication";

            QString pubKeyPath = sessionData_.sshConfig.privateKeyPath + ".pub";

            rc = libssh2_userauth_publickey_fromfile(
                session_,
                sessionData_.sshConfig.username.toUtf8().constData(),
                QFile::exists(pubKeyPath) ? pubKeyPath.toUtf8().constData() : nullptr,
                sessionData_.sshConfig.privateKeyPath.toUtf8().constData(),
                sessionData_.sshConfig.passphrase.isEmpty() ? nullptr :
                    sessionData_.sshConfig.passphrase.toUtf8().constData()
            );

            if (rc == 0) {
                qDebug() << "Public key authentication successful";
                return true;
            }
            qDebug() << "Public key authentication failed";
        }
    }

    if (!sessionData_.sshConfig.password.isEmpty()) {
        qDebug() << "Trying password authentication";

        rc = libssh2_userauth_password(
            session_,
            sessionData_.sshConfig.username.toUtf8().constData(),
            sessionData_.sshConfig.password.toUtf8().constData()
        );

        if (rc == 0) {
            qDebug() << "Password authentication successful";
            return true;
        }
        qDebug() << "Password authentication failed";
    }

    QMessageBox::critical(this, tr("SSH Error"),
        tr("Authentication failed for user '%1'").arg(sessionData_.sshConfig.username));
    emit onSessionError(this);
    return false;
}

bool SSHTerminal::openChannel() {
    channel_ = libssh2_channel_open_session(session_);
    if (!channel_) {
        char *errmsg = nullptr;
        libssh2_session_last_error(session_, &errmsg, nullptr, 0);
        QMessageBox::critical(this, tr("SSH Error"),
            tr("Failed to open channel: %1").arg(errmsg ? errmsg : "Unknown"));
        emit onSessionError(this);
        return false;
    }

    int rc = libssh2_channel_request_pty(channel_, "xterm-256color");
    if (rc) {
        QMessageBox::critical(this, tr("SSH Error"), tr("Failed to request PTY"));
        emit onSessionError(this);
        return false;
    }

    // —— 新增：请求 X11 转发（0 = 允许多连接）——
    rc = libssh2_channel_x11_req(channel_, 0);
    if (rc) {
        QMessageBox::warning(this, tr("SSH Error"), tr("Failed to request X11 forwarding"));
        emit onSessionError(this);
        return false;
    }

    libssh2_channel_request_pty_size(channel_, 80, 24);

    rc = libssh2_channel_shell(channel_);
    if (rc) {
        QMessageBox::critical(this, tr("SSH Error"), tr("Failed to start shell"));
        emit onSessionError(this);
        return false;
    }

    qDebug() << "SSH channel opened";
    return true;
}

void SSHTerminal::sendData(const QByteArray &data) {
    if (!channel_ || !running_) return;

    const char *ptr = data.constData();
    int remaining = data.size();

    while (remaining > 0) {
        ssize_t written = libssh2_channel_write(channel_, ptr, remaining);

        if (written == LIBSSH2_ERROR_EAGAIN) {
            // 处理事件循环，避免死锁
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 1);
            continue;
        }

        if (written < 0) {
            QMessageBox::critical(this, tr("SSH Error"), tr("Failed to send data"));
            emit onSessionError(this);
            return;
        }

        ptr += written;
        remaining -= written;
    }
}

void SSHTerminal::resizePty(int cols, int rows) {
    if (!channel_ || !running_) return;

    libssh2_channel_request_pty_size(channel_, cols, rows);
}

// ====== X11 ======

void SSHTerminal::x11Callback(LIBSSH2_SESSION*,
                              LIBSSH2_CHANNEL* channel,
                              char*, int,
                              void **abstract)
{
    auto *self = static_cast<SSHTerminal*>(*abstract);
    if (self) self->handleNewX11Channel(channel);
}

void SSHTerminal::handleNewX11Channel(LIBSSH2_CHANNEL *chan)
{
    // 连接 127.0.0.1:6000（VcXsrv，DISPLAY :0）
    SOCKET xsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (xsock == INVALID_SOCKET) {
        libssh2_channel_free(chan);
        return;
    }

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(6000);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

    if (::connect(xsock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        closesocket(xsock);
        libssh2_channel_free(chan);
        qWarning() << "Failed to connect to local X server (VcXsrv)";
        return;
    }

    // 非阻塞
    u_long nb = 1;
    ioctlsocket(xsock, FIONBIO, &nb);

    auto *xf = new X11Forward;
    xf->chan  = chan;
    xf->xsock = xsock;
    xf->notifier = new QSocketNotifier((qintptr)xsock, QSocketNotifier::Read, this);

    // 本地 -> 远端
    QObject::connect(xf->notifier, &QSocketNotifier::activated,
                     this, [this, xf](qintptr){
        char buf[8192];
        int n = recv(xf->xsock, buf, sizeof(buf), 0);
        if (n > 0) {
            int sent = 0;
            while (sent < n) {
                int w = libssh2_channel_write(xf->chan, buf + sent, n - sent);
                if (w == LIBSSH2_ERROR_EAGAIN) {
                    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 1);
                    continue;
                }
                if (w < 0) break;
                sent += w;
            }
        } else { // 对端关闭
            if (xf->notifier) xf->notifier->deleteLater();
            libssh2_channel_send_eof(xf->chan);
        }
    });

    x11Chans_.push_back(xf);
    qDebug() << "X11 channel bridged to VcXsrv (:0)";
}

void SSHTerminal::pumpRemoteX11()
{
    char buf[8192];
    for (auto it = x11Chans_.begin(); it != x11Chans_.end(); ) {
        X11Forward *xf = *it;
        ssize_t r = libssh2_channel_read(xf->chan, buf, sizeof(buf));
        if (r > 0) {
            send(xf->xsock, buf, static_cast<int>(r), 0);
            ++it;
        } else if (r == LIBSSH2_ERROR_EAGAIN) {
            ++it;
        } else { // 关闭或错误
            if (xf->notifier) xf->notifier->deleteLater();
            if (xf->xsock != INVALID_SOCKET) closesocket(xf->xsock);
            if (xf->chan) libssh2_channel_free(xf->chan);
            delete xf;
            it = x11Chans_.erase(it);
        }
    }
}

void SSHTerminal::cleanup() {
    running_ = false;

    // 关闭 X11 子通道与本地 socket
    for (auto *xf : x11Chans_) {
        if (xf->notifier) xf->notifier->deleteLater();
        if (xf->chan)     libssh2_channel_free(xf->chan);
        if (xf->xsock != INVALID_SOCKET) closesocket(xf->xsock);
        delete xf;
    }
    x11Chans_.clear();

    if (channel_) {
        libssh2_channel_send_eof(channel_);
        libssh2_channel_close(channel_);
        libssh2_channel_free(channel_);
        channel_ = nullptr;
    }

    if (session_) {
        libssh2_session_disconnect(session_, "Normal shutdown");
        libssh2_session_free(session_);
        session_ = nullptr;
    }

    if (sock_ != INVALID_SOCKET) {
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
    }
}
