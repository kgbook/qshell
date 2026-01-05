#ifndef QSHELL_COMMANDBUTTONBAR_H
#define QSHELL_COMMANDBUTTONBAR_H

#include <QToolBar>
class CommandButtonBar : public QToolBar {
    Q_OBJECT
public:
    explicit CommandButtonBar(QWidget *parent = nullptr);
    ~CommandButtonBar() override = default;

};


#endif//QSHELL_COMMANDBUTTONBAR_H
