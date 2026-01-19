#ifndef QSHELL_BASE_TERMINAL_H
#define QSHELL_BASE_TERMINAL_H

#include "qtermwidget.h"
#include <QFile>
#include <QMenu>

class IPtyProcess;

class BaseTerminal : public QTermWidget {

    Q_OBJECT

public:
    explicit BaseTerminal(QWidget *parent);
    ~BaseTerminal() override;

    void startLocalShell();
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    bool isConnect() const;

    // 日志相关方法
    bool isLogging() const;
    QString logFilePath() const;

signals:
    void onSessionError(BaseTerminal *terminal);
    void loggingStateChanged(bool isLogging);

protected:
    void onDisplayOutput(const char *data, int len);
    void onCopyAvailable(bool copyAvailable);

    // 右键菜单事件
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void onToggleLogging();

private:
    void startLogging(const QString &filePath);
    void stopLogging();
    void writeToLog(const char *data, int len);

protected:
    QFont *font_ = nullptr;
    bool connect_ = false;
    IPtyProcess *localShell_ = nullptr;

private:
    // 日志相关成员
    bool logging_ = false;
    QFile *logFile_ = nullptr;
    QString logFilePath_;
};

#endif//QSHELL_BASE_TERMINAL_H
