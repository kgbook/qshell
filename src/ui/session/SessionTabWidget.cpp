#include "SessionTabWidget.h"

#include <QInputDialog>
#include <QMenu>
#include <QTabBar>

SessionTabWidget::SessionTabWidget(QWidget *parent)
    : QTabWidget(parent) {

    // 设置 tab 可关闭
    setTabsClosable(true);

    // 为 tab bar 启用右键菜单
    tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);

    // 连接信号
    connect(tabBar(), &QTabBar::customContextMenuRequested,
            this, &SessionTabWidget::showTabContextMenu);

    // 连接关闭按钮信号
    connect(this, &QTabWidget::tabCloseRequested,
            this, [this](int index) {
                QWidget *widget = this->widget(index);
                removeTab(index);
                if (widget) {
                    widget->deleteLater();
                }
            });
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

    menu.addSeparator();

    QAction *closeAction = menu.addAction(tr("Close Tab"));
    connect(closeAction, &QAction::triggered, this, &SessionTabWidget::closeTab);

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

void SessionTabWidget::closeTab() {
    if (m_contextMenuTabIndex < 0 || m_contextMenuTabIndex >= count()) {
        return;
    }

    // 获取并移除 tab
    QWidget *widget = this->widget(m_contextMenuTabIndex);
    removeTab(m_contextMenuTabIndex);

    // 删除对应的 widget
    if (widget) {
        widget->deleteLater();
    }
}
