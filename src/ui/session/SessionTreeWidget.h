#ifndef QSHELL_SESSIONTREEWIDGET_H
#define QSHELL_SESSIONTREEWIDGET_H

#include <QItemDelegate>
#include <QTreeWidget>

class SessionTreeWidget : public QTreeWidget {
    Q_OBJECT

public:
    explicit SessionTreeWidget(QWidget *parent = nullptr);
    ~SessionTreeWidget() override = default;
};


#endif//QSHELL_SESSIONTREEWIDGET_H
