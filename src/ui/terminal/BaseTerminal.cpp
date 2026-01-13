#include "BaseTerminal.h"

#include "core/ConfigManager.h"
#include "ptyqt.h"
#include <QDebug>
#include <QDir>
#include <qprocess.h>

BaseTerminal::BaseTerminal(QWidget *parent) : QTermWidget(parent, parent) {
    connect_ = false;

    auto globalSettings = ConfigManager::instance()->globalSettings();

    //font
    font_ = new QFont();
    font_->setFamily(globalSettings.fontFamily);
    font_->setPointSize(globalSettings.fontSize);
    setTerminalFont(*font_);
    setHistorySize(128000);
    setTerminalSizeHint(false);
    QStringList env;
    env.append("TERM=xterm-256color");
    // setEnvironment(env);

    //set color scheme
    setColorScheme("Tango");

    //set scroll bar
    setScrollBarPosition(ScrollBarRight);

    // QObject::connect(this, &QTermWidget::onNewLine, this, &BaseTerminal::onNewLine);
    // QObject::connect(this, &QTermWidget::receivedData, this, &BaseTerminal::onHexData);
}

BaseTerminal::~BaseTerminal() {
    disableLogSession();
}
void BaseTerminal::startLocalShell() {
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    QString shellPath = qEnvironmentVariable("SHELL");
#elif defined(Q_OS_WIN)
    QString shellPath = "C:/Windows/System32/WindowsPowerShell/v1.0/powershell.exe";
#endif
    localShell_ = PtyQt::createPtyProcess();
    QObject::connect(this, &QTermWidget::termSizeChange, [=](int lines, int columns) {
        localShell_->resize(static_cast<qint16>(columns), static_cast<qint16>(lines));
    });
    QStringList args;
    localShell_->startProcess(shellPath, args, QDir::homePath(), QProcessEnvironment::systemEnvironment().toStringList(),
                              static_cast<qint16>(screenColumnsCount()), static_cast<qint16>(screenLinesCount()));
    QObject::connect(localShell_->notifier(), &QIODevice::readyRead, [=]() {
        QByteArray data = localShell_->readAll();
        if (!data.isEmpty()) {
            this->recvData(data.data(), static_cast<int>(data.size()));
        }
    });
    QObject::connect(localShell_->notifier(), &QIODevice::aboutToClose, [=]() {
        if (localShell_) {
            QByteArray restOfOutput = localShell_->readAll();
            if (!restOfOutput.isEmpty()) {
                this->recvData(restOfOutput.data(), static_cast<int>(restOfOutput.size()));
                localShell_->notifier()->disconnect();
            }
        }
    });
    QObject::connect(this, &QTermWidget::sendData, [=](const char *data, int size) {
        localShell_->write(QByteArray(data, size));
    });
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

bool BaseTerminal::isLoggingHexSession() const {
    return loggingHex_;
}
