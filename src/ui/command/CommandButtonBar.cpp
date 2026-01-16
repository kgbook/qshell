#include "CommandButtonBar.h"
#include "core/ConfigManager.h"

#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QLabel>

CommandButtonBar::CommandButtonBar(QWidget *parent)
    : QToolBar(parent) {
    setupUI();

    // 连接配置管理器的信号
    connect(ConfigManager::instance(), &ConfigManager::buttonGroupsChanged,
            this, &CommandButtonBar::refreshGroups);
    connect(ConfigManager::instance(), &ConfigManager::quickButtonsChanged,
            this, &CommandButtonBar::refreshButtons);

    // 初始加载
    refreshGroups();
}

void CommandButtonBar::setupUI() {
    setMovable(false);
    setFloatable(false);

    // 分组选择下拉框
    groupComboBox_ = new QComboBox(this);
    groupComboBox_->setMinimumWidth(120);
    groupComboBox_->setMaximumWidth(200);
    connect(groupComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CommandButtonBar::onGroupChanged);
    addWidget(groupComboBox_);

    addSeparator();

    // 管理分组按钮
    auto *manageBtn = new QPushButton(tr("管理"), this);
    manageBtn->setMaximumWidth(60);
    connect(manageBtn, &QPushButton::clicked, this, &CommandButtonBar::onManageGroups);
    addWidget(manageBtn);

    // 添加按钮
    auto *addBtn = new QPushButton(tr("+"), this);
    addBtn->setMaximumWidth(30);
    addBtn->setToolTip(tr("添加命令按钮"));
    connect(addBtn, &QPushButton::clicked, this, &CommandButtonBar::onAddButton);
    addWidget(addBtn);

    addSeparator();

    // 按钮容器
    buttonContainer_ = new QWidget(this);
    buttonLayout_ = new QHBoxLayout(buttonContainer_);
    buttonLayout_->setContentsMargins(0, 0, 0, 0);
    buttonLayout_->setSpacing(4);
    buttonLayout_->addStretch();
    addWidget(buttonContainer_);
}

void CommandButtonBar::refreshGroups() {
    QString currentGroupId;
    if (groupComboBox_->currentIndex() >= 0) {
        currentGroupId = groupComboBox_->currentData().toString();
    }

    groupComboBox_->blockSignals(true);
    groupComboBox_->clear();

    auto groups = ConfigManager::instance()->buttonGroups();

    if (groups.isEmpty()) {
        groupComboBox_->addItem(tr("(无分组)"), QString());
    } else {
        for (const auto &group : groups) {
            groupComboBox_->addItem(group.name, group.id);
        }
    }

    // 恢复之前选中的分组
    int index = -1;
    if (!currentGroupId.isEmpty()) {
        for (int i = 0; i < groupComboBox_->count(); ++i) {
            if (groupComboBox_->itemData(i).toString() == currentGroupId) {
                index = i;
                break;
            }
        }
    }
    groupComboBox_->setCurrentIndex(index >= 0 ? index : 0);
    groupComboBox_->blockSignals(false);

    refreshButtons();
}

void CommandButtonBar::refreshButtons() {
    clearButtons();

    QString groupId = groupComboBox_->currentData().toString();
    if (groupId.isEmpty() && groupComboBox_->currentIndex() == 0
        && groupComboBox_->itemText(0) == tr("(无分组)")) {
        return;
    }

    auto buttons = ConfigManager::instance()->quickButtonsByGroup(groupId);

    for (const auto &btn : buttons) {
        auto *pushBtn = new QPushButton(btn.name, buttonContainer_);
        pushBtn->setProperty("buttonId", btn.id);
        pushBtn->setProperty("command", btn.command);
        pushBtn->setToolTip(btn.command);
        pushBtn->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(pushBtn, &QPushButton::clicked, this, &CommandButtonBar::onButtonClicked);
        connect(pushBtn, &QPushButton::customContextMenuRequested,
                this, &CommandButtonBar::onButtonContextMenu);

        // 插入到stretch之前
        buttonLayout_->insertWidget(buttonLayout_->count() - 1, pushBtn);
        buttons_[btn.id] = pushBtn;
    }
}

void CommandButtonBar::clearButtons() {
    for (auto *btn : buttons_) {
        buttonLayout_->removeWidget(btn);
        delete btn;
    }
    buttons_.clear();
}

void CommandButtonBar::onGroupChanged(int index) {
    Q_UNUSED(index)
    refreshButtons();
}

void CommandButtonBar::onButtonClicked() {
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (btn) {
        QString command = btn->property("command").toString();
        emit commandTriggered(command);
    }
}

void CommandButtonBar::onManageGroups() {
    QMenu menu(this);

    // 添加分组
    QAction *addGroupAction = menu.addAction(tr("添加分组"));
    connect(addGroupAction, &QAction::triggered, this, [this]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("添加分组"),
                                             tr("分组名称:"), QLineEdit::Normal,
                                             QString(), &ok);
        if (ok && !name.isEmpty()) {
            ButtonGroup group;
            group.name = name;
            ConfigManager::instance()->addButtonGroup(group);
        }
    });

    menu.addSeparator();

    // 当前分组操作
    QString currentGroupId = groupComboBox_->currentData().toString();
    if (!currentGroupId.isEmpty()) {
        // 重命名分组
        QAction *renameAction = menu.addAction(tr("重命名当前分组"));
        connect(renameAction, &QAction::triggered, this, [this, currentGroupId]() {
            auto group = ConfigManager::instance()->buttonGroup(currentGroupId);
            bool ok;
            QString name = QInputDialog::getText(this, tr("重命名分组"),
                                                 tr("分组名称:"), QLineEdit::Normal,
                                                 group.name, &ok);
            if (ok && !name.isEmpty()) {
                group.name = name;
                ConfigManager::instance()->updateButtonGroup(group);
            }
        });

        // 删除分组
        QAction *deleteAction = menu.addAction(tr("删除当前分组"));
        connect(deleteAction, &QAction::triggered, this, [this, currentGroupId]() {
            auto buttons = ConfigManager::instance()->quickButtonsByGroup(currentGroupId);
            QString msg = tr("确定要删除此分组吗？");
            if (!buttons.isEmpty()) {
                msg += tr("\n该分组下有 %1 个按钮，将一并删除。").arg(buttons.size());
            }

            if (QMessageBox::question(this, tr("确认删除"), msg) == QMessageBox::Yes) {
                ConfigManager::instance()->removeButtonGroup(currentGroupId);
            }
        });

        menu.addSeparator();

        // 上移/下移分组
        QAction *moveUpAction = menu.addAction(tr("上移分组"));
        connect(moveUpAction, &QAction::triggered, this, [currentGroupId]() {
            ConfigManager::instance()->moveButtonGroupUp(currentGroupId);
        });

        QAction *moveDownAction = menu.addAction(tr("下移分组"));
        connect(moveDownAction, &QAction::triggered, this, [currentGroupId]() {
            ConfigManager::instance()->moveButtonGroupDown(currentGroupId);
        });
    }

    menu.exec(QCursor::pos());
}

void CommandButtonBar::onAddButton() {
    QString groupId = groupComboBox_->currentData().toString();

    // 检查是否有分组
    if (groupId.isEmpty()) {
        auto groups = ConfigManager::instance()->buttonGroups();
        if (groups.isEmpty()) {
            QMessageBox::information(this, tr("提示"),
                                     tr("请先创建一个分组"));
            return;
        }
    }

    // 创建添加按钮对话框
    QDialog dialog(this);
    dialog.setWindowTitle(tr("添加命令按钮"));
    dialog.setMinimumWidth(400);

    auto *layout = new QFormLayout(&dialog);

    auto *nameEdit = new QLineEdit(&dialog);
    layout->addRow(tr("按钮名称:"), nameEdit);

    auto *commandEdit = new QTextEdit(&dialog);
    commandEdit->setMaximumHeight(100);
    layout->addRow(tr("命令:"), commandEdit);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addRow(buttonBox);

    if (dialog.exec() == QDialog::Accepted) {
        QString name = nameEdit->text().trimmed();
        QString command = commandEdit->toPlainText();

        if (name.isEmpty()) {
            QMessageBox::warning(this, tr("错误"), tr("按钮名称不能为空"));
            return;
        }

        QuickButton btn;
        btn.name = name;
        btn.command = command;
        btn.groupId = groupId;
        ConfigManager::instance()->addQuickButton(btn);
    }
}

void CommandButtonBar::onButtonContextMenu(const QPoint &pos) {
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QString buttonId = btn->property("buttonId").toString();

    QMenu menu(this);

    // 编辑
    QAction *editAction = menu.addAction(tr("编辑"));
    connect(editAction, &QAction::triggered, this, [this, buttonId]() {
        auto button = ConfigManager::instance()->quickButton(buttonId);

        QDialog dialog(this);
        dialog.setWindowTitle(tr("编辑命令按钮"));
        dialog.setMinimumWidth(400);

        auto *layout = new QFormLayout(&dialog);

        auto *nameEdit = new QLineEdit(button.name, &dialog);
        layout->addRow(tr("按钮名称:"), nameEdit);

        auto *commandEdit = new QTextEdit(&dialog);
        commandEdit->setPlainText(button.command);
        commandEdit->setMaximumHeight(100);
        layout->addRow(tr("命令:"), commandEdit);

        auto *buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout->addRow(buttonBox);

        if (dialog.exec() == QDialog::Accepted) {
            button.name = nameEdit->text().trimmed();
            button.command = commandEdit->toPlainText();

            if (!button.name.isEmpty()) {
                ConfigManager::instance()->updateQuickButton(button);
            }
        }
    });

    // 删除
    QAction *deleteAction = menu.addAction(tr("删除"));
    connect(deleteAction, &QAction::triggered, this, [this, buttonId, btn]() {
        if (QMessageBox::question(this, tr("确认删除"),
                                  tr("确定要删除按钮 \"%1\" 吗？").arg(btn->text()))
            == QMessageBox::Yes) {
            ConfigManager::instance()->removeQuickButton(buttonId);
        }
    });

    menu.addSeparator();

    // 上移/下移
    QAction *moveUpAction = menu.addAction(tr("左移"));
    connect(moveUpAction, &QAction::triggered, this, [buttonId]() {
        ConfigManager::instance()->moveQuickButtonUp(buttonId);
    });

    QAction *moveDownAction = menu.addAction(tr("右移"));
    connect(moveDownAction, &QAction::triggered, this, [buttonId]() {
        ConfigManager::instance()->moveQuickButtonDown(buttonId);
    });

    menu.exec(btn->mapToGlobal(pos));
}
