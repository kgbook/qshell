#ifndef QSHELL_MAINWINDOW_H
#define QSHELL_MAINWINDOW_H

#include <QMainWindow>

class SessionTabWidget;
class SessionTreeWidget;
class CommandButtonBar;
class CommandWindow;
class BaseTerminal;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onOpenSession(const QString& sessionId);
    void onSessionError(BaseTerminal *terminal) const;
    void onDisconnectAction() const;
    void onTabChanged(int index);
    void onTabCloseRequested(int index) const;
    void onCommandSend(const QString &command);

private:
    void initIcons();
    void initSessionTree();
    void initTableWidget();
    void initCommandWindow();
    void initButtonBar();
    void initMenuBar();

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

    // command window
    QDockWidget *commandWindowDock_ = nullptr;
    CommandWindow *commandWindow_ = nullptr;

    // command button bar
    CommandButtonBar *commandButtonBar_ = nullptr;
};

#endif // QSHELL_MAINWINDOW_H
