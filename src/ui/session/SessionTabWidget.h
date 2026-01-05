#ifndef QSHELL_SESSIONTABWIDGET_H
#define QSHELL_SESSIONTABWIDGET_H

#include <QTabWidget>

class SessionTabWidget : public QTabWidget {
    Q_OBJECT

public:
    explicit SessionTabWidget(QWidget *parent = nullptr);
    ~SessionTabWidget() override = default;
};


#endif//QSHELL_SESSIONTABWIDGET_H
