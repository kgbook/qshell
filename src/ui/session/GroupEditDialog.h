#ifndef GROUPDIALOG_H
#define GROUPDIALOG_H

#include "core/datatype.h"
#include <QDialog>

class QLineEdit;

class GroupEditDialog : public QDialog {
    Q_OBJECT

public:
    explicit GroupEditDialog(QWidget *parent = nullptr);

    void setGroupData(const GroupData &data);

    GroupData groupData() const;

private:
    void setupUI();

    GroupData groupData_;
    QLineEdit *nameEdit_ = nullptr;
};

#endif // GROUPDIALOG_H
