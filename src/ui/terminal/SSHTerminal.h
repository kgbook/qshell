#ifndef KKSHELL_SSH_TERMINAL_H
#define KKSHELL_SSH_TERMINAL_H

#include "BaseTerminal.h"
#include "core/SessionData.h"

class SSHTerminal : public BaseTerminal {
public:
    explicit SSHTerminal(const SessionData &session, QWidget *parent);
    ~SSHTerminal() override;
    void connect() override;
    void disconnect() override;

private:
    bool isInRemoteServer() const;
    SessionData sessionData_;
};


#endif //KKSHELL_SSH_TERMINAL_H
