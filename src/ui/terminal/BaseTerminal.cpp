#include "BaseTerminal.h"

#include "core/ConfigManager.h"
#include "ptyqt.h"
#include <QDebug>
#include <QDir>
#include <qprocess.h>

BaseTerminal::BaseTerminal(QWidget *parent) : QTermWidget(parent, parent) {
    connect_ = false;

    auto globalSettings = ConfigManager::instance()->globalSettings();

    font_ = new QFont();
    font_->setFamily(globalSettings.fontFamily);
    font_->setPointSize(globalSettings.fontSize);
    setTerminalFont(*font_);
    setHistorySize(128000);
    setTerminalSizeHint(false);
    setColorScheme(globalSettings.colorScheme);
    setScrollBarPosition(ScrollBarRight);
    QObject::connect(this, &QTermWidget::copyAvailable, this, &BaseTerminal::onCopyAvailable);

    // QObject::connect(this, &QTermWidget::onNewLine, this, &BaseTerminal::onNewLine);
    // QObject::connect(this, &QTermWidget::receivedData, this, &BaseTerminal::onHexData);
}

BaseTerminal::~BaseTerminal() {
    disableLogSession();
}
void BaseTerminal::startLocalShell() {
    QString shellPath;
    QStringList args;
    QStringList envs = QProcessEnvironment::systemEnvironment().toStringList();
    envs.append("TERM=xterm-256color");

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    shellPath = qEnvironmentVariable("SHELL");
#elif defined(Q_OS_WIN)
    shellPath = "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe";
#endif

    // 明确指定 ConPty
    localShell_ = PtyQt::createPtyProcess();
    if (!localShell_) {
        qWarning() << "Failed to create ConPty process!";
        return;
    }

    // 连接发送数据
    QObject::connect(this, &QTermWidget::sendData, this, [this](const char *data, int size) {
        if (localShell_) {
            QByteArray senddata(data, size);
            localShell_->write(senddata);
        }
    });

    // 连接大小变化
    QObject::connect(this, &QTermWidget::termSizeChange, this, [this](int lines, int columns) {
        if (localShell_) {
            localShell_->resize(static_cast<qint16>(columns), static_cast<qint16>(lines));
        }
    });

    // 启动进程
    bool ret = localShell_->startProcess(
        shellPath,
        args,
        QDir::homePath(),
        envs,
        static_cast<qint16>(screenColumnsCount()),
        static_cast<qint16>(screenLinesCount())
    );

    if (!ret) {
        qWarning() << "startProcess failed:" << localShell_->lastError();
        return;
    }

    // 连接 notifier（如果可用）
    if (QIODevice* notifier = localShell_->notifier()) {
        QObject::connect(notifier, &QIODevice::readyRead, this, [this]() {
            QByteArray data = localShell_->readAll();
            if (!data.isEmpty()) {
                this->recvData(data.data(), static_cast<int>(data.size()));
            }
        });
    } else {
        qDebug() << "localShell_->notifier() is nullptr";
    }

    connect_ = true;
}


bool BaseTerminal::isConnect() const {
    return connect_;
}

void BaseTerminal::logSession(const std::string &logPath) {
    logPath_ = logPath;
    logFp_ = fopen(logPath.c_str(), "wb+");
    logging_ = true;
}

void BaseTerminal::disableLogSession() {
    logging_ = false;
    if (logFp_) {
        fflush(logFp_);
        fclose(logFp_);
        logFp_ = nullptr;
    }
}

void BaseTerminal::onNewLine(const QString &line) const {
    if (logging_) {
        fwrite(line.toStdString().c_str(), line.toStdString().length(), 1, logFp_);
        fwrite("\n", 1, 1, logFp_);
    }
}

bool BaseTerminal::isLoggingSession() const {
    return logging_;
}


void BaseTerminal::logHexSession(const std::string &logPath) {
    logHexPath_ = logPath;
    logHexFp_ = fopen(logPath.c_str(), "wb+");
    loggingHex_ = true;
}

void BaseTerminal::disableLogHexSession() {
    loggingHex_ = false;
    if (logHexFp_) {
        fflush(logHexFp_);
        fclose(logHexFp_);
        logHexFp_ = nullptr;
    }
}

void BaseTerminal::onHexData(const char *data, int len) const {
    if (loggingHex_) {
        fwrite(data, len, 1, logHexFp_);
    }
}

void BaseTerminal::onCopyAvailable(bool copyAvailable) {
    if (copyAvailable) {
        copyClipboard();
    }

}

bool BaseTerminal::isLoggingHexSession() const {
    return loggingHex_;
}
