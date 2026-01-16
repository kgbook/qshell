#include "CommandHistoryDialog.h"
#include "ui/terminal/BaseTerminal.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QLabel>
#include <QDialogButtonBox>

CommandHistoryDialog::CommandHistoryDialog(const QStringList &history, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(tr("Command History"));
    setMinimumSize(500, 400);

    auto *layout = new QVBoxLayout(this);

    // 历史记录列表
    listWidget_ = new QListWidget(this);
    listWidget_->setSelectionMode(QAbstractItemView::SingleSelection);

    // 按时间倒序显示（最新的在最上面）
    for (auto i = history.size() - 1; i >= 0; --i) {
        listWidget_->addItem(history[i]);
    }

    layout->addWidget(new QLabel(tr("Double-click to select a command:")));
    layout->addWidget(listWidget_);

    // 按钮
    auto *buttonLayout = new QHBoxLayout();

    auto *clearButton = new QPushButton(tr("Clear History"), this);
    clearButton->setIcon(QIcon::fromTheme("edit-clear"));
    connect(clearButton, &QPushButton::clicked, this, &CommandHistoryDialog::onClearHistory);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(buttonBox);

    layout->addLayout(buttonLayout);

    connect(listWidget_, &QListWidget::itemDoubleClicked,
            this, &CommandHistoryDialog::onItemDoubleClicked);
}

void CommandHistoryDialog::onItemDoubleClicked(QListWidgetItem *item) {
    if (item) {
        emit commandSelected(item->text());
        accept();
    }
}

void CommandHistoryDialog::onClearHistory() {
    auto result = QMessageBox::question(
        this,
        tr("Clear History"),
        tr("Are you sure you want to clear all command history?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (result == QMessageBox::Yes) {
        emit clearHistoryRequested();
        listWidget_->clear();
    }
}
