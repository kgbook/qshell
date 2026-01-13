#include "SSHTerminal.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTime>
#include <QtCore/QEventLoop>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <unistd.h>

SSHTerminal::SSHTerminal(const SessionData &session, QWidget *parent) : BaseTerminal(parent) {
    sessionData_ = session;
}

SSHTerminal::~SSHTerminal() {
}

void SSHTerminal::connect() {

}

void SSHTerminal::disconnect() {
}

QString SSHTerminal::createShellFile() {

}

bool SSHTerminal::isInRemoteServer() {

}