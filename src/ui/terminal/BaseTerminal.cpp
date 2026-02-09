#include "BaseTerminal.h"

#include "core/ConfigManager.h"
#include "ptyqt.h"
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QContextMenuEvent>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>

BaseTerminal::BaseTerminal(QWidget *parent) : QTermWidget(parent, parent) {
    connect_ = false;
    logging_ = false;
    logFile_ = nullptr;

    auto globalSettings = ConfigManager::instance()->globalSettings();

    font_ = new QFont();
    font_->setFamily(globalSettings.fontFamily);
    font_->setPointSize(globalSettings.fontSize);
    setTerminalFont(*font_);
    setHistorySize(128000);
    setTerminalSizeHint(false);
    setUrlFilterEnabled(false);
    setColorScheme(globalSettings.colorScheme);
    setScrollBarPosition(ScrollBarRight);

    if (globalSettings.copyOnSelect) {
        QObject::connect(this, &QTermWidget::copyAvailable, this, &BaseTerminal::onCopyAvailable);
    }

    QObject::connect(this, &QTermWidget::onNewLine, this, &BaseTerminal::onDisplayOutput);

    // 启用右键菜单
    setContextMenuPolicy(Qt::DefaultContextMenu);
}

BaseTerminal::~BaseTerminal() {
    // 确保关闭日志文件
    stopLogging();

    delete font_;
    font_ = nullptr;
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

bool BaseTerminal::isLogging() const {
    return logging_;
}

QString BaseTerminal::logFilePath() const {
    return logFilePath_;
}

QString BaseTerminal::getSessionName() const {
    return sessionData_.name;
}

void BaseTerminal::onDisplayOutput(const QString &line) {
    // 如果正在记录日志，写入数据
    if (logging_ && logFile_ && logFile_->isOpen()) {
        writeToLog(line);
    }
}

void BaseTerminal::onCopyAvailable(bool copyAvailable) {
    if (copyAvailable) {
        copyClipboard();
    }
}

void BaseTerminal::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);

    // 复制操作
    QAction *copyAction = menu.addAction(tr("复制"));
    copyAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
    copyAction->setEnabled(selectedText().length() > 0);
    QObject::connect(copyAction, &QAction::triggered, this, [this]() {
        copyClipboard();
    });

    // 粘贴操作
    QAction *pasteAction = menu.addAction(tr("粘贴"));
    pasteAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V));
    QObject::connect(pasteAction, &QAction::triggered, this, [this]() {
        pasteClipboard();
    });

    menu.addSeparator();

    // 日志保存操作
    QAction *logAction = nullptr;
    if (logging_) {
        logAction = menu.addAction(tr("停止保存日志"));
        logAction->setIcon(QIcon::fromTheme("media-playback-stop"));
    } else {
        logAction = menu.addAction(tr("保存日志..."));
        logAction->setIcon(QIcon::fromTheme("document-save"));
    }
    QObject::connect(logAction, &QAction::triggered, this, &BaseTerminal::onToggleLogging);

    menu.addSeparator();

    // 清屏操作
    QAction *clearAction = menu.addAction(tr("清屏"));
    clearAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_X));
    QObject::connect(clearAction, &QAction::triggered, this, [this]() {
        clear();
    });

    menu.exec(event->globalPos());
}

void BaseTerminal::onToggleLogging() {
    if (logging_) {
        // 停止日志记录
        stopLogging();
        QMessageBox::information(this, tr("日志保存"),
            tr("日志保存已停止。\n文件: %1").arg(logFilePath_));
    } else {
        // 开始日志记录 - 打开文件选择对话框
        QString defaultFileName = QString("terminal_log_%1.log")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));

        QString filePath = QFileDialog::getSaveFileName(
            this,
            tr("保存终端日志"),
            QDir::homePath() + "/" + defaultFileName,
            tr("日志文件 (*.log *.txt);;所有文件 (*)"),
            nullptr,
            QFileDialog::DontConfirmOverwrite  // 允许选择现有文件
        );

        if (!filePath.isEmpty()) {
            // 如果文件已存在，询问是追加还是覆盖
            if (QFile::exists(filePath)) {
                QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    tr("文件已存在"),
                    tr("文件 \"%1\" 已存在。\n\n"
                       "点击\"是\"追加到现有文件\n"
                       "点击\"否\"覆盖现有文件\n"
                       "点击\"取消\"放弃操作").arg(QFileInfo(filePath).fileName()),
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                    QMessageBox::Yes
                );

                if (reply == QMessageBox::Cancel) {
                    return;
                }

                if (reply == QMessageBox::No) {
                    // 覆盖文件 - 先删除
                    QFile::remove(filePath);
                }
            }

            startLogging(filePath);

            if (logging_) {
                QMessageBox::information(this, tr("日志保存"),
                    tr("开始保存日志到:\n%1").arg(filePath));
            }
        }
    }
}

void BaseTerminal::startLogging(const QString &filePath) {
    if (logging_) {
        stopLogging();
    }

    logFile_ = new QFile(filePath);

    // 以追加模式打开（如果用户选择覆盖，文件已被删除）
    if (!logFile_->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QMessageBox::critical(this, tr("错误"),
            tr("无法打开文件进行写入:\n%1\n\n错误: %2")
                .arg(filePath)
                .arg(logFile_->errorString()));
        delete logFile_;
        logFile_ = nullptr;
        return;
    }

    logFilePath_ = filePath;
    logging_ = true;

    // 写入日志头
    QString header = QString("\n========== 日志开始: %1 ==========\n")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    logFile_->write(header.toUtf8());
    logFile_->flush();

    emit loggingStateChanged(true);

    qDebug() << "Started logging to:" << filePath;
}

void BaseTerminal::stopLogging() {
    if (!logging_ || !logFile_) {
        return;
    }

    // 写入日志尾
    QString footer = QString("\n========== 日志结束: %1 ==========\n")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    logFile_->write(footer.toUtf8());
    logFile_->flush();

    logFile_->close();
    delete logFile_;
    logFile_ = nullptr;
    logging_ = false;

    emit loggingStateChanged(false);

    qDebug() << "Stopped logging to:" << logFilePath_;
}

void BaseTerminal::writeToLog(const QString &line) {
    if (!logFile_ || !logFile_->isOpen()) {
        return;
    }

    // 直接写入原始数据
    logFile_->write(line.toUtf8());
    logFile_->write("\n");

    // 定期刷新确保数据写入磁盘
    static int writeCount = 0;
    if (++writeCount >= 10) {  // 每10次写入刷新一次
        logFile_->flush();
        writeCount = 0;
    }
}
