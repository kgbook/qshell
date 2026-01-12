#include "BaseTerminal.h"

#include <QDebug>
#include "core/ConfigManager.h"

BaseTerminal::BaseTerminal(QWidget *parent) : QTermWidget(parent, parent) {
    connect_ = false;

    auto globalSettings = ConfigManager::instance()->globalSettings();

    //font
    font_ = new QFont();
    font_->setFamily(globalSettings.fontFamily);
    font_->setPointSize(globalSettings.fontSize);
    setTerminalFont(*font_);
    setHistorySize(128000);
    setTerminalSizeHint(false);
    QStringList env;
    env.append("TERM=xterm-256color");
    // setEnvironment(env);

    //set color scheme
    setColorScheme("Tango");

    //set scroll bar
    setScrollBarPosition(ScrollBarRight);

    // QObject::connect(this, &QTermWidget::onNewLine, this, &BaseTerminal::onNewLine);
    // QObject::connect(this, &QTermWidget::receivedData, this, &BaseTerminal::onHexData);
}

BaseTerminal::~BaseTerminal() {
    disableLogSession();
}

bool BaseTerminal::isConnect() const {
    return connect_;
}

void BaseTerminal::logSession(const std::string &logPath) {
    logPath_ = logPath;
    logFp_ = fopen(logPath.c_str(), "wb+");
    logging_ = true;
}

void BaseTerminal::disableLogSession() {
    logging_ = false;
    if (logFp_) {
        fflush(logFp_);
        fclose(logFp_);
        logFp_ = nullptr;
    }
}

void BaseTerminal::onNewLine(const QString &line) const {
    if (logging_) {
        fwrite(line.toStdString().c_str(), line.toStdString().length(), 1, logFp_);
        fwrite("\n", 1, 1, logFp_);
    }
}

bool BaseTerminal::isLoggingSession() const {
    return logging_;
}


void BaseTerminal::logHexSession(const std::string &logPath) {
    logHexPath_ = logPath;
    logHexFp_ = fopen(logPath.c_str(), "wb+");
    loggingHex_ = true;
}

void BaseTerminal::disableLogHexSession() {
    loggingHex_ = false;
    if (logHexFp_) {
        fflush(logHexFp_);
        fclose(logHexFp_);
        logHexFp_ = nullptr;
    }
}

void BaseTerminal::onHexData(const char *data, int len) const {
    if (loggingHex_) {
        fwrite(data, len, 1, logHexFp_);
    }
}

bool BaseTerminal::isLoggingHexSession() const {
    return loggingHex_;
}
