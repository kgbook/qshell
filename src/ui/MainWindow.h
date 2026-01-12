#ifndef QSHELL_MAINWINDOW_H
#define QSHELL_MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>

class SessionTabWidget;
class SessionTreeWidget;
class CommandButtonBar;
class BaseTerminal;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private:
    void onOpenSession(const QString& sessionId);
    void onSessionError(BaseTerminal *terminal) const;
    void onDisconnectAction() const;
    void onTabChanged(int index);
    void onTabCloseRequested(int index) const;

    void initIcons();
    void initSessionTree();
    void initTableWidget();
    void initCommandWindow();
    void initButtonBar();

    QIcon *connectIcon_ = nullptr;
    QIcon *disconnectIcon_ = nullptr;
    QIcon *connectStateIcon_ = nullptr;
    QIcon *disconnectStateIcon_ = nullptr;

    // session table
    BaseTerminal *currentTab_ = nullptr;
    SessionTabWidget *tabWidget_ = nullptr;
    SessionTreeWidget *treeWidget_ = nullptr;

    // session manager
    QDockWidget *sessionDock_ = nullptr;
    SessionTreeWidget *sessionTree_ = nullptr;

    //command window
    QDockWidget *commandWindowDock_ = nullptr;
    QTextEdit *commandEditor_ = nullptr;

    // command button bar
    CommandButtonBar *commandButtonBar_ = nullptr;
};


#endif//QSHELL_MAINWINDOW_H
