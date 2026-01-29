#include "SessionTabWidget.h"

#include <QInputDialog>
#include <QMenu>
#include <QTabBar>

SessionTabWidget::SessionTabWidget(QWidget *parent)
    : QTabWidget(parent) {

    // 为 tab bar 启用右键菜单
    tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);

    // 连接信号
    connect(tabBar(), &QTabBar::customContextMenuRequested,
            this, &SessionTabWidget::showTabContextMenu);
}

void SessionTabWidget::showTabContextMenu(const QPoint &pos) {
    // 获取点击位置对应的 tab 索引
    m_contextMenuTabIndex = tabBar()->tabAt(pos);

    // 如果没有点击到 tab，则不显示菜单
    if (m_contextMenuTabIndex < 0) {
        return;
    }

    // 创建右键菜单
    QMenu menu(this);

    QAction *renameAction = menu.addAction(tr("Rename Tab"));
    connect(renameAction, &QAction::triggered, this, &SessionTabWidget::renameTab);

    // 在鼠标位置显示菜单
    menu.exec(tabBar()->mapToGlobal(pos));
}

void SessionTabWidget::renameTab() {
    if (m_contextMenuTabIndex < 0 || m_contextMenuTabIndex >= count()) {
        return;
    }

    // 获取当前 tab 名称
    QString currentName = tabText(m_contextMenuTabIndex);

    // 弹出输入对话框
    bool ok;
    QString newName = QInputDialog::getText(this,
                                            tr("Rename Tab"),
                                            tr("New name:"),
                                            QLineEdit::Normal,
                                            currentName,
                                            &ok);

    // 如果用户确认且输入不为空，则重命名
    if (ok && !newName.isEmpty()) {
        setTabText(m_contextMenuTabIndex, newName);
    }
}
