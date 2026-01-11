#include "SessionTreeWidget.h"
#include "SessionEditDialog.h"
#include "GroupEditDialog.h"
#include "core/ConfigManager.h"
#include "core/SessionData.h"

#include <QVBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QHeaderView>

// 自定义数据角色
enum {
    ItemTypeRole = Qt::UserRole + 1,
    ItemIdRole = Qt::UserRole + 2
};

enum ItemType {
    GroupItem = 1,
    SessionItem = 2
};

SessionTreeWidget::SessionTreeWidget(QWidget* parent)
    : QTreeWidget(parent) {
    setupUI();
    loadData();

    // 连接配置管理器信号
    auto config = ConfigManager::instance();
    connect(config, &ConfigManager::sessionAdded, this, &SessionTreeWidget::onSessionAdded);
    connect(config, &ConfigManager::sessionUpdated, this, &SessionTreeWidget::onSessionUpdated);
    connect(config, &ConfigManager::sessionRemoved, this, &SessionTreeWidget::onSessionRemoved);
    connect(config, &ConfigManager::groupAdded, this, &SessionTreeWidget::onGroupAdded);
    connect(config, &ConfigManager::groupUpdated, this, &SessionTreeWidget::onGroupUpdated);
    connect(config, &ConfigManager::groupRemoved, this, &SessionTreeWidget::onGroupRemoved);
}

void SessionTreeWidget::setupUI() {
    setHeaderLabel(tr("Sessions"));
    setContextMenuPolicy(Qt::CustomContextMenu);
    setRootIsDecorated(true);
    setAnimated(true);
    header()->setVisible(false);
    
    connect(this, &QTreeWidget::itemDoubleClicked,
            this, &SessionTreeWidget::onItemDoubleClicked);
    connect(this, &QTreeWidget::customContextMenuRequested,
            this, &SessionTreeWidget::onCustomContextMenu);
}

void SessionTreeWidget::loadData() {
    refreshTree();
}

void SessionTreeWidget::refreshTree() {
    qDebug() << "SessionTreeWidget::refreshTree";
    this->clear();
    
    auto config = ConfigManager::instance();
    
    // 添加分组
    QMap<QString, QTreeWidgetItem*> groupItems;
    for (const auto& group : config->groups()) {
        qDebug() << "add group:" << group.name;
        auto* item = new QTreeWidgetItem(this);
        item->setText(0, group.name);
        item->setData(0, ItemTypeRole, GroupItem);
        item->setData(0, ItemIdRole, group.id);
        item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        item->setExpanded(true);
        groupItems[group.id] = item;
    }
    
    // 添加会话
    for (const auto& session : config->sessions()) {
        qDebug() << "add session:" << session.name;
        QTreeWidgetItem* parent = nullptr;
        if (!session.groupId.isEmpty() && groupItems.contains(session.groupId)) {
            parent = groupItems[session.groupId];
        }
        
        QTreeWidgetItem* item;
        if (parent) {
            item = new QTreeWidgetItem(parent);
        } else {
            item = new QTreeWidgetItem(this);
        }
        
        item->setText(0, session.name);
        item->setData(0, ItemTypeRole, SessionItem);
        item->setData(0, ItemIdRole, session.id);
        
        QStyle::StandardPixmap icon = QStyle::SP_ComputerIcon;
        item->setIcon(0, style()->standardIcon(icon));
    }
}

QTreeWidgetItem* SessionTreeWidget::findGroupItem(const QString& groupId) const {
    for (int i = 0; i < this->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = this->topLevelItem(i);
        if (item->data(0, ItemTypeRole).toInt() == GroupItem &&
            item->data(0, ItemIdRole).toString() == groupId) {
            return item;
        }
    }
    return nullptr;
}

QTreeWidgetItem* SessionTreeWidget::findSessionItem(const QString& sessionId) const {
    std::function<QTreeWidgetItem*(QTreeWidgetItem*)> findInItem = [&](QTreeWidgetItem* parent) -> QTreeWidgetItem* {
        for (int i = 0; i < (parent ? parent->childCount() : this->topLevelItemCount()); ++i) {
            QTreeWidgetItem* item = parent ? parent->child(i) : this->topLevelItem(i);
            if (item->data(0, ItemTypeRole).toInt() == SessionItem &&
                item->data(0, ItemIdRole).toString() == sessionId) {
                return item;
            }
            if (item->childCount() > 0) {
                if (auto found = findInItem(item)) {
                    return found;
                }
            }
        }
        return nullptr;
    };
    return findInItem(nullptr);
}

void SessionTreeWidget::onItemDoubleClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column)
    
    if (item->data(0, ItemTypeRole).toInt() == SessionItem) {
        QString sessionId = item->data(0, ItemIdRole).toString();
        emit openSession(sessionId);
    }
}

void SessionTreeWidget::onCustomContextMenu(const QPoint& pos) {
    QTreeWidgetItem* item = this->itemAt(pos);
    QMenu menu(this);
    
    if (!item) {
        // 空白区域右键
        menu.addAction(tr("New Group..."), this, &SessionTreeWidget::createNewGroup);
        menu.addAction(tr("New Session..."), this, &SessionTreeWidget::createNewSession);
    } else if (item->data(0, ItemTypeRole).toInt() == GroupItem) {
        // 分组右键
        QString groupId = item->data(0, ItemIdRole).toString();
        
        menu.addAction(tr("New Session in Group..."), this, [this, groupId]() {
            SessionEditDialog dialog(this);
            dialog.setGroupId(groupId);
            if (dialog.exec() == QDialog::Accepted) {
                ConfigManager::instance()->addSession(dialog.sessionData());
            }
        });
        
        menu.addSeparator();
        
        menu.addAction(tr("Edit Group..."), this, [this, groupId]() {
            editGroup(groupId);
        });
        
        menu.addAction(tr("Delete Group"), this, [this, groupId]() {
            deleteGroup(groupId);
        });
    } else {
        // 会话右键
        QString sessionId = item->data(0, ItemIdRole).toString();
        
        menu.addAction(tr("Connect"), this, [this, sessionId]() {
            emit openSession(sessionId);
        });
        
        menu.addSeparator();
        
        menu.addAction(tr("Edit Session..."), this, [this, sessionId]() {
            editSession(sessionId);
        });
        
        menu.addAction(tr("Delete Session"), this, [this, sessionId]() {
            deleteSession(sessionId);
        });
    }
    
    menu.exec(this->viewport()->mapToGlobal(pos));
}

void SessionTreeWidget::createNewSession() {
    SessionEditDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        ConfigManager::instance()->addSession(dialog.sessionData());
    }
}

void SessionTreeWidget::createNewGroup() {
    GroupEditDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        ConfigManager::instance()->addGroup(dialog.groupData());
    }
}

void SessionTreeWidget::editSession(const QString& sessionId) {
    SessionData session = ConfigManager::instance()->session(sessionId);
    if (session.id.isEmpty()) return;
    
    SessionEditDialog dialog(this);
    dialog.setSessionData(session);
    if (dialog.exec() == QDialog::Accepted) {
        ConfigManager::instance()->updateSession(dialog.sessionData());
    }
}

void SessionTreeWidget::deleteSession(const QString& sessionId) {
    SessionData session = ConfigManager::instance()->session(sessionId);
    if (session.id.isEmpty()) return;
    
    QMessageBox::StandardButton result = QMessageBox::question(
        this,
        tr("Delete Session"),
        tr("Are you sure you want to delete session '%1'?").arg(session.name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (result == QMessageBox::Yes) {
        ConfigManager::instance()->removeSession(sessionId);
    }
}

void SessionTreeWidget::editGroup(const QString& groupId) {
    GroupData group = ConfigManager::instance()->group(groupId);
    if (group.id.isEmpty()) return;
    
    GroupEditDialog dialog(this);
    dialog.setGroupData(group);
    if (dialog.exec() == QDialog::Accepted) {
        ConfigManager::instance()->updateGroup(dialog.groupData());
    }
}

void SessionTreeWidget::deleteGroup(const QString& groupId) {
    if (!ConfigManager::instance()->isGroupEmpty(groupId)) {
        QMessageBox::warning(
            this,
            tr("Cannot Delete Group"),
            tr("The group is not empty. Please delete all sessions in the group first.")
        );
        return;
    }
    
    GroupData group = ConfigManager::instance()->group(groupId);
    if (group.id.isEmpty()) return;
    
    QMessageBox::StandardButton result = QMessageBox::question(
        this,
        tr("Delete Group"),
        tr("Are you sure you want to delete group '%1'?").arg(group.name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (result == QMessageBox::Yes) {
        ConfigManager::instance()->removeGroup(groupId);
    }
}

void SessionTreeWidget::onSessionAdded(const SessionData& session) {
    Q_UNUSED(session)
    refreshTree();
}

void SessionTreeWidget::onSessionUpdated(const SessionData& session) {
    Q_UNUSED(session)
    refreshTree();
}

void SessionTreeWidget::onSessionRemoved(const QString& id) {
    Q_UNUSED(id)
    refreshTree();
}

void SessionTreeWidget::onGroupAdded(const GroupData& group) {
    Q_UNUSED(group)
    refreshTree();
}

void SessionTreeWidget::onGroupUpdated(const GroupData& group) {
    Q_UNUSED(group)
    refreshTree();
}

void SessionTreeWidget::onGroupRemoved(const QString& id) {
    Q_UNUSED(id)
    refreshTree();
}