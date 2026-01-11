#ifndef QSHELL_SESSIONTREEWIDGET_H
#define QSHELL_SESSIONTREEWIDGET_H

#include <QItemDelegate>
#include <QTreeWidget>

class SessionTreeWidget : public QTreeWidget {
    Q_OBJECT

public:
    explicit SessionTreeWidget(QWidget *parent = nullptr);

    ~SessionTreeWidget() override = default;

public slots:
    void createNewSession();

    void createNewGroup();

signals:
    void openSession(const QString &sessionId);

private slots:
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

    void onCustomContextMenu(const QPoint &pos);

    void onSessionAdded(const class SessionData &session);

    void onSessionUpdated(const class SessionData &session);

    void onSessionRemoved(const QString &id);

    void onGroupAdded(const class GroupData &group);

    void onGroupUpdated(const class GroupData &group);

    void onGroupRemoved(const QString &id);

private:
    void setupUI();

    void loadData();

    void refreshTree();

    QTreeWidgetItem *findGroupItem(const QString &groupId) const;

    QTreeWidgetItem *findSessionItem(const QString &sessionId) const;

    void editSession(const QString &sessionId);

    void deleteSession(const QString &sessionId);

    void editGroup(const QString &groupId);

    void deleteGroup(const QString &groupId);
};


#endif//QSHELL_SESSIONTREEWIDGET_H
