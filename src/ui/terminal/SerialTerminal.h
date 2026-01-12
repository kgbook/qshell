#ifndef KKSHELL_SERIAL_TERMINAL_H
#define KKSHELL_SERIAL_TERMINAL_H

#include <QtSerialPort/QSerialPort>
#include "BaseTerminal.h"
#include "core/SessionData.h"

class SerialTerminal : public BaseTerminal {
public:
    explicit SerialTerminal(const SessionData &session, QWidget *parent);
    ~SerialTerminal() override;
    void connect() override;
    void disconnect() override;

private:
    void handleError(QSerialPort::SerialPortError error);
    QSerialPort *serial_ = nullptr;
    SessionData sessionData_;
};


#endif //KKSHELL_SERIAL_TERMINAL_H
