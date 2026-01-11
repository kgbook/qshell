#include "GroupEditDialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>

GroupEditDialog::GroupEditDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(tr("Group Properties"));
    setupUI();
}

void GroupEditDialog::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);

    auto *formLayout = new QFormLayout();

    nameEdit_ = new QLineEdit(this);
    formLayout->addRow(tr("Name:"), nameEdit_);

    mainLayout->addLayout(formLayout);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (nameEdit_->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, tr("Validation Error"),
                                 tr("Group name cannot be empty."));
            return;
        }
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void GroupEditDialog::setGroupData(const GroupData &data) {
    groupData_ = data;
    nameEdit_->setText(data.name);
}

GroupData GroupEditDialog::groupData() const {
    GroupData data = groupData_;
    data.name = nameEdit_->text().trimmed();
    return data;
}
