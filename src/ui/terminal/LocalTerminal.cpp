#include "LocalTerminal.h"

#include "iptyprocess.h"
#include "ptyqt.h"

#include <QDir>
#include <qprocess.h>

LocalTerminal::LocalTerminal(QWidget *parent) : BaseTerminal(parent) {
}

LocalTerminal::~LocalTerminal() = default;

void LocalTerminal::connect() {
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
    connect_ = true;
}

void LocalTerminal::disconnect() {
    if (localShell_) {
        localShell_->kill();
        delete localShell_;
        localShell_ = nullptr;
    }
    connect_ = false;
}
