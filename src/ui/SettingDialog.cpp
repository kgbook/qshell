#include "SettingDialog.h"
#include "qtermwidget.h"
#include "core/ConfigManager.h"

SettingDialog::SettingDialog(QWidget *parent) :QDialog(parent) {
    initWidgets();
    fillColorScheme();
    const auto config = ConfigManager::instance();
    fontFamilyEdit_->setCurrentText(config->globalSettings().fontFamily);
    fontSizeEdit_->setText(QString::number(config->globalSettings().fontSize));
    colorSchemeEdit_->setCurrentText(config->globalSettings().colorScheme);
}

SettingDialog::~SettingDialog() = default;

void SettingDialog::initWidgets() {
    mainLayout_ = new QVBoxLayout(this);
    setLayout(mainLayout_);

    formLayout_ = new QFormLayout(this);
    mainLayout_->addLayout(formLayout_);

    fontFamilyEdit_ = new QFontComboBox(this);
    formLayout_->addRow(tr("Font Family:"), fontFamilyEdit_);

    fontSizeEdit_ = new QLineEdit(this);
    formLayout_->addRow(tr("Font Size:"), fontSizeEdit_);

    colorSchemeEdit_ = new QComboBox(this);
    formLayout_->addRow(tr("Color Scheme:"), colorSchemeEdit_);

    buttonLayout_ = new QHBoxLayout(this);
    mainLayout_->addLayout(buttonLayout_);

    okButton_ = new QPushButton(tr("OK"));
    cancelButton_ = new QPushButton(tr("Cancel"));
    buttonLayout_->addWidget(cancelButton_);
    buttonLayout_->addWidget(okButton_);

    QObject::connect(okButton_, &QPushButton::clicked, this, &SettingDialog::onOK);
    QObject::connect(cancelButton_, &QPushButton::clicked, this, &SettingDialog::onCancel);
}

void SettingDialog::fillColorScheme() {
    QStringList schemes = QTermWidget::availableColorSchemes();
    for (const QString &scheme : schemes) {
        colorSchemeEdit_->addItem(scheme, scheme);
    }
}

void SettingDialog::onOK() {
    GlobalSettings settings;
    settings.fontFamily = fontFamilyEdit_->currentText();
    settings.fontSize = fontSizeEdit_->text().toInt();
    settings.colorScheme = colorSchemeEdit_->currentText();
    ConfigManager::instance()->setGlobalSettings(settings);
    close();
}

void SettingDialog::onCancel() {
    close();
}
