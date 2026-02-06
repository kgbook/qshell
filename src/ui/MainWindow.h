#ifndef QSHELL_MAINWINDOW_H
#define QSHELL_MAINWINDOW_H

#include <QMainWindow>
#include <QShortcut>

class SessionTabWidget;
class SessionTreeWidget;
class CommandButtonBar;
class CommandWindow;
class BaseTerminal;
class CollapsibleDockWidget;
class LuaScriptEngine;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;
    void showEvent(QShowEvent *event) override;

private slots:
    void onOpenSession(const QString& sessionId);
    void onSessionError(BaseTerminal *terminal) const;
    void onDisconnectAction() const;
    void onTabChanged(int index);
    void onTabCloseRequested(int index) const;
    void onCommandSend(const QString &command);

    void onSettingsAction();
    void onConnectAction();
    void onExitAction();

    void onCopySelectedAction();
    void onCopyAllAction();
    void onPasteAction();
    void onFindAction();
    void onClearScreenAction();

    void onToggleToolbarAction();
    void onToggleSessionManagerAction();
    void onToggleCommandWindowAction();
    void onToggleCommandButtonAction();

    void onFullscreenAction();

    // Script menu slots
    void onRunLuaScriptAction();
    static void onStopScriptAction();
    void onRecentScriptTriggered();

    // Help menu slots
    static void onDocAction();
    void onAboutAction();

private:
    void initIcons();
    void initActions();
    void initMenu();
    void initToolbar();
    void initSessionTree();
    void initTableWidget();
    void initCommandWindow();
    void initButtonBar();
    void restoreLayoutState();
    void exitFullscreen();

    void runScript(const QString &scriptPath);
    void addRecentScript(const QString &scriptPath);
    void loadRecentScripts();
    void saveRecentScripts();
    void updateRecentScriptsMenu();

    bool eventFilter(QObject *watched, QEvent *event) override;

    QIcon *connectIcon_ = nullptr;
    QIcon *disconnectIcon_ = nullptr;
    QIcon *connectStateIcon_ = nullptr;
    QIcon *disconnectStateIcon_ = nullptr;

    QMenuBar *mainMenuBar_ = nullptr;
    QMenu *fileMenu_ = nullptr;
    QMenu *editMenu_ = nullptr;
    QMenu *viewMenu_ = nullptr;
    QMenu *scriptMenu_ = nullptr;
    QMenu *helpMenu_ = nullptr;

    QToolBar *toolBar_ = nullptr;
    QIcon *settingsIcon_ = nullptr;
    QIcon *exitIcon_ = nullptr;

    QIcon *copySelectedIcon_ = nullptr;
    QIcon *copyAllIcon_ = nullptr;
    QIcon *pasteIcon_ = nullptr;
    QIcon *findIcon_ = nullptr;
    QIcon *clearScreenIcon_ = nullptr;

    QIcon *toggleOnIcon_ = nullptr;
    QIcon *toggleOffIcon_ = nullptr;

    QIcon *fullscreenIcon_ = nullptr;

    QAction *settingsAction_ = nullptr;
    QAction *connectAction_ = nullptr;
    QAction *disConnectAction_ = nullptr;
    QAction *exitAction_ = nullptr;

    QAction *copySelectedAction_ = nullptr;
    QAction *copyAllAction_ = nullptr;
    QAction *pasteAction_ = nullptr;
    QAction *findAction_ = nullptr;
    QAction *clearScreenAction_ = nullptr;

    QAction *toggleToolbarAction_ = nullptr;
    QAction *toggleSessionManagerAction_ = nullptr;
    QAction *toggleCommandWindowAction_ = nullptr;
    QAction *toggleCommandButtonAction_ = nullptr;

    QAction *fullscreenAction_ = nullptr;

    // Script actions
    QAction *runLuaScriptAction_ = nullptr;
    QAction *stopScriptAction_ = nullptr;
    QMenu *recentScriptMenu_ = nullptr;
    QStringList recentScripts_;
    static constexpr int MaxRecentScripts = 10;

    // Help actions
    QAction *docAction_ = nullptr;
    QAction *aboutAction_ = nullptr;

    // session table
    BaseTerminal *currentTab_ = nullptr;
    SessionTabWidget *tabWidget_ = nullptr;
    SessionTreeWidget *treeWidget_ = nullptr;

    // session manager
    CollapsibleDockWidget *sessionDock_ = nullptr;
    SessionTreeWidget *sessionTree_ = nullptr;

    // command window
    QDockWidget *commandWindowDock_ = nullptr;
    CommandWindow *commandWindow_ = nullptr;

    // command button bar
    CommandButtonBar *commandButtonBar_ = nullptr;

    // full screen
    bool isFullscreen_ = false;
    QWidget *fullscreenWidget_ = nullptr;
    QShortcut *escShortcut_ = nullptr;

    LuaScriptEngine *luaEngine_ = nullptr;
};

#endif // QSHELL_MAINWINDOW_H
