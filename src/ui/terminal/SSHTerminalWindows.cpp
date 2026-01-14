#include "SSHTerminal.h"
#include "iptyprocess.h"

#include <QDebug>

SSHTerminal::SSHTerminal(const SessionData &session, QWidget *parent) : BaseTerminal(parent) {
    sessionData_ = session;
}

SSHTerminal::~SSHTerminal() {
}

void SSHTerminal::connect() {

}

void SSHTerminal::disconnect() {
}

bool SSHTerminal::isInRemoteServer() const {
    return localShell_->hasChildProcess();
}
