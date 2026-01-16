#ifndef QSHELL_COMMANDWINDOW_H
#define QSHELL_COMMANDWINDOW_H

#include <QDialog>
#include <QListWidget>
#include <QStringList>
#include <QTextEdit>
#include <QWidget>

class CommandWindow : public QWidget {
    Q_OBJECT

public:
    explicit CommandWindow(QWidget *parent = nullptr);
    ~CommandWindow() override;

    QWidget *widget() const;

    signals:
        void commandSend(const QString &command);

public slots:
    void showHistoryDialog();
    void clearHistory();

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    void loadHistory();
    void saveHistory();
    void addToHistory(const QString &command);
    void navigateHistory(int direction);
    void setupContextMenu();

    QTextEdit *commandEditor_ = nullptr;

    // 历史记录
    QStringList history_;
    qsizetype historyIndex_ = -1;
    QString currentInput_;// 保存用户当前输入（未发送）

    static constexpr int MAX_HISTORY_SIZE = 500;
    QString historyFilePath_;
};

#endif// QSHELL_COMMANDWINDOW_H
