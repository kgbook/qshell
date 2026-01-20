#ifndef QSHELL_SERIAL_TERMINAL_H
#define QSHELL_SERIAL_TERMINAL_H

#include "BaseTerminal.h"
#include "core/datatype.h"
#include <QtSerialPort/QSerialPort>

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


#endif //QSHELL_SERIAL_TERMINAL_H
