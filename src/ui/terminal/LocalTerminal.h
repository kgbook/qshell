#ifndef KKSHELL_LOCAL_TERMINAL_H
#define KKSHELL_LOCAL_TERMINAL_H

#include "BaseTerminal.h"

class IPtyProcess;

class LocalTerminal : public BaseTerminal {
public:
    explicit LocalTerminal(QWidget *parent);
    ~LocalTerminal() override;
    void connect() override;
    void disconnect() override;

private:
    IPtyProcess *localShell_ = nullptr;
};


#endif //KKSHELL_LOCAL_TERMINAL_H
