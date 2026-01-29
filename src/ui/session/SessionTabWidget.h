#ifndef QSHELL_SESSIONTABWIDGET_H
#define QSHELL_SESSIONTABWIDGET_H

#include <QTabWidget>

class SessionTabWidget : public QTabWidget {
    Q_OBJECT

public:
    explicit SessionTabWidget(QWidget *parent = nullptr);
    ~SessionTabWidget() override = default;

private slots:
    void showTabContextMenu(const QPoint &pos);
    void renameTab();

private:
    int m_contextMenuTabIndex = -1;  // 记录右键点击的 tab 索引
};

#endif//QSHELL_SESSIONTABWIDGET_H
