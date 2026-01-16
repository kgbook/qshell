#ifndef QSHELL_COMMANDWINDOW_DIALOG_H
#define QSHELL_COMMANDWINDOW_DIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QStringList>
#include <QTextEdit>

class BaseTerminal;

class CommandHistoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit CommandHistoryDialog(const QStringList &history, QWidget *parent = nullptr);

signals:
    void commandSelected(const QString &command);
    void clearHistoryRequested();

private:
    void onItemDoubleClicked(QListWidgetItem *item);
    void onClearHistory();

    QListWidget *listWidget_ = nullptr;
};

#endif// QSHELL_COMMANDWINDOW_DIALOG_H
