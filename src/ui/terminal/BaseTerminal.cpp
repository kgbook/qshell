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
    if (globalSettings.copyOnSelect) {
        QObject::connect(this, &QTermWidget::copyAvailable, this, &BaseTerminal::onCopyAvailable);
    }

    QObject::connect(this, &QTermWidget::dupDisplayOutput, this, &BaseTerminal::onDisplayOutput);
}

BaseTerminal::~BaseTerminal() = default;

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

void BaseTerminal::onDisplayOutput(const char *data, int len) const {

}

void BaseTerminal::onCopyAvailable(bool copyAvailable) {
    if (copyAvailable) {
        copyClipboard();
    }
}
