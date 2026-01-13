#ifndef KKSHELL_BASE_TERMINAL_H
#define KKSHELL_BASE_TERMINAL_H

#include "qtermwidget.h"
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

    bool isLoggingSession() const;
    void logSession(const std::string &logPath);
    void disableLogSession();

    bool isLoggingHexSession() const;
    void logHexSession(const std::string &logPath);
    void disableLogHexSession();

signals:
    void onSessionError(BaseTerminal *terminal);

protected:
    void onNewLine(const QString &line) const;
    void onHexData(const char *data, int len) const;

protected:
    QFont *font_ = nullptr;
    bool connect_ = false;
    bool logging_ = false;
    std::string logPath_;
    FILE *logFp_ = nullptr;
    bool loggingHex_ = false;
    std::string logHexPath_;
    FILE *logHexFp_ = nullptr;
    IPtyProcess *localShell_ = nullptr;
};


#endif //KKSHELL_BASE_TERMINAL_H
