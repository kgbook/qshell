#include "LocalTerminal.h"

#include "iptyprocess.h"

LocalTerminal::LocalTerminal(QWidget *parent) : BaseTerminal(parent) {
}

LocalTerminal::~LocalTerminal() = default;

void LocalTerminal::connect() {
    startLocalShell();
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
