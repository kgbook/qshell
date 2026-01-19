#ifndef QSHELL_BASE_TERMINAL_H
#define QSHELL_BASE_TERMINAL_H

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

signals:
    void onSessionError(BaseTerminal *terminal);

protected:
    void onDisplayOutput(const char *data, int len) const;
    void onCopyAvailable(bool copyAvailable);

protected:
    QFont *font_ = nullptr;
    bool connect_ = false;
    IPtyProcess *localShell_ = nullptr;
};


#endif //QSHELL_BASE_TERMINAL_H
