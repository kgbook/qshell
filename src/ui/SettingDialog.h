#ifndef QSHELL_SETTINGDIALOG_H
#define QSHELL_SETTINGDIALOG_H

#include <QFormLayout>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QFontComboBox>
#include <QVBoxLayout>
#include <QCheckBox>


class SettingDialog : public QDialog  {
    Q_OBJECT
    public:
    explicit SettingDialog(QWidget *parent);
    ~SettingDialog() override;


private:
    void initWidgets();
    void fillColorScheme();
    void onOK();
    void onCancel();

    QVBoxLayout *mainLayout_ = nullptr;
    QFormLayout *formLayout_ = nullptr;

    QFontComboBox *fontFamilyEdit_ = nullptr;
    QLineEdit *fontSizeEdit_ = nullptr;
    QComboBox *colorSchemeEdit_ = nullptr;
    QCheckBox *copyOnSelectCheckBox_ = nullptr;

    QHBoxLayout *buttonLayout_ = nullptr;
    QPushButton *okButton_ = nullptr;
    QPushButton *cancelButton_ = nullptr;
};


#endif //QSHELL_SETTINGDIALOG_H
