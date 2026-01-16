#include "MainWindow.h"
#include "SettingDialog.h"
#include "command/CommandButtonBar.h"
#include "core/ConfigManager.h"
#include "session/SessionTabWidget.h"
#include "session/SessionTreeWidget.h"
#include "ui/command/CommandWindow.h"
#include "ui/terminal/BaseTerminal.h"
#include "ui/terminal/LocalTerminal.h"
#include "ui/terminal/SSHTerminal.h"
#include "ui/terminal/SerialTerminal.h"

#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowState(Qt::WindowMaximized);
    setContextMenuPolicy(Qt::NoContextMenu);
    QIcon windowIcon(":/images/application.png");
    setWindowIcon(windowIcon);

    initIcons();
    initActions();
    initMenu();
    initToolbar();
    initTableWidget();
    initSessionTree();
    initCommandWindow();
    initButtonBar();
}

void MainWindow::initIcons() {
    connectIcon_ = new QIcon(":/images/connect.png");
    disconnectIcon_ = new QIcon(":/images/disconnect.png");
    connectStateIcon_ = new QIcon(":/images/connect_state.png");
    disconnectStateIcon_ = new QIcon(":/images/disconnect_state.png");

    settingsIcon_ = new QIcon(":/images/setting.png");
    exitIcon_ = new QIcon(":/images/exit.png");

    copySelectedIcon_ = new QIcon(":/images/copy.png");
    copyAllIcon_ = new QIcon(":/images/copy_all.png");
    pasteIcon_ = new QIcon(":/images/paste.png");
    findIcon_ = new QIcon(":/images/find.png");
    clearScreenIcon_ = new QIcon(":/images/clear.png");

    toggleOnIcon_ = new QIcon(":/images/toggle_on.png");
    toggleOffIcon_ = new QIcon(":/images/toggle_off.png");
}

void MainWindow::initTableWidget() {
    tabWidget_ = new SessionTabWidget(this);
    tabWidget_->setTabsClosable(true);
    tabWidget_->setMovable(true);
    setCentralWidget(tabWidget_);

    connect(tabWidget_, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(tabWidget_, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);
}

void MainWindow::onOpenSession(const QString &sessionId) {
    auto session = ConfigManager::instance()->session(sessionId);
    BaseTerminal *terminal = nullptr;

    if (session.protocolType == ProtocolType::Serial) {
        terminal = new SerialTerminal(session, this);
    } else if (session.protocolType == ProtocolType::LocalShell) {
        terminal = new LocalTerminal(this);
    } else if (session.protocolType == ProtocolType::SSH) {
        terminal = new SSHTerminal(session, this);
    } else {
        qDebug() << "unknown session type!!";
        return;
    }

    terminal->connect();
    connect(terminal, &BaseTerminal::onSessionError, this, &MainWindow::onSessionError);
    tabWidget_->addTab(terminal, *connectStateIcon_, session.name);
    tabWidget_->setCurrentWidget(terminal);
    terminal->setFocus();
    qDebug() << "onOpenSession" << session.name;
}

void MainWindow::onSessionError(BaseTerminal *terminal) const {
    if (terminal == currentTab_) {
        onDisconnectAction();
    } else {
        int index = tabWidget_->indexOf(terminal);
        terminal->disconnect();
        tabWidget_->setTabIcon(index, *disconnectStateIcon_);
    }
}

void MainWindow::initSessionTree() {
    sessionDock_ = new QDockWidget(tr("Session Manager"), this);
    sessionTree_ = new SessionTreeWidget(this);
    sessionDock_->setWidget(sessionTree_);
    sessionDock_->setFeatures(QDockWidget::DockWidgetMovable);
    sessionDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    sessionDock_->setMinimumWidth(220);
    sessionDock_->setMaximumWidth(480);
    addDockWidget(Qt::LeftDockWidgetArea, sessionDock_);

    connect(sessionTree_, &SessionTreeWidget::openSession,
            this, &MainWindow::onOpenSession);
}

void MainWindow::initCommandWindow() {
    commandWindowDock_ = new QDockWidget(tr("Command Window"), this);
    commandWindow_ = new CommandWindow(this);
    commandWindowDock_->setWidget(commandWindow_);
    commandWindowDock_->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::BottomDockWidgetArea, commandWindowDock_);
    resizeDocks({commandWindowDock_}, {80}, Qt::Vertical);

    // 隐藏标题栏
    QWidget *titleBar = commandWindowDock_->titleBarWidget();
    auto *emptyWidget = new QWidget();
    commandWindowDock_->setTitleBarWidget(emptyWidget);
    delete titleBar;

    // 连接命令发送信号
    connect(commandWindow_, &CommandWindow::commandSend,
            this, &MainWindow::onCommandSend);
}

void MainWindow::initButtonBar() {
    commandButtonBar_ = new CommandButtonBar(this);
    commandButtonBar_->setMovable(false);
    addToolBar(Qt::BottomToolBarArea, commandButtonBar_);

    // 连接命令触发信号
    connect(commandButtonBar_, &CommandButtonBar::commandTriggered,
            this, &MainWindow::onCommandSend);
}

void MainWindow::onTabChanged(int index) {
    qDebug() << "onTabChanged, index = " << index;

    if (index < 0) {
        currentTab_ = nullptr;
        return;
    }

    currentTab_ = dynamic_cast<BaseTerminal *>(tabWidget_->widget(index));
}

void MainWindow::onTabCloseRequested(int index) const {
    auto *tab = dynamic_cast<BaseTerminal *>(tabWidget_->widget(index));
    tab->disconnect();
    tabWidget_->removeTab(index);
    delete tab;
}

void MainWindow::onDisconnectAction() const {
    if (currentTab_ == nullptr) {
        return;
    }

    currentTab_->disconnect();
    tabWidget_->setTabIcon(tabWidget_->currentIndex(), *disconnectStateIcon_);
}

void MainWindow::onCommandSend(const QString &command) {
    if (currentTab_ != nullptr) {
        QString str = command;
        str.replace(QString("\\r"), QString("\r"));
        currentTab_->sendText(str);
    }
}

void MainWindow::initActions() {
    settingsAction_ = new QAction(*settingsIcon_, tr("Setting"), this);
    connect(settingsAction_, &QAction::triggered, this, &MainWindow::onSettingsAction);

    toggleSaveLogAction_ = new QAction(*toggleOffIcon_, tr("Save Session Log"), this);
    toggleSaveLogAction_->setEnabled(false);
    connect(toggleSaveLogAction_, &QAction::triggered, this, &MainWindow::onToggleSaveLogAction);

    toggleSaveHexLogAction_ = new QAction(*toggleOffIcon_, tr("Save Session Hex Log"), this);
    toggleSaveHexLogAction_->setEnabled(false);
    connect(toggleSaveHexLogAction_, &QAction::triggered, this, &MainWindow::onToggleSaveHexLogAction);

    connectAction_ = new QAction(*connectIcon_, tr("Connect"), this);
    connectAction_->setEnabled(false);
    connect(connectAction_, &QAction::triggered, this, &MainWindow::onConnectAction);

    disConnectAction_ = new QAction(*disconnectIcon_, tr("Disconnect"), this);
    disConnectAction_->setEnabled(false);
    connect(disConnectAction_, &QAction::triggered, this, &MainWindow::onDisconnectAction);

    exitAction_ = new QAction(*exitIcon_, tr("Exit"), this);
    connect(exitAction_, &QAction::triggered, this, &MainWindow::onExitAction);


    copySelectedAction_ = new QAction(*copySelectedIcon_, tr("Copy Selected"), this);
    connect(copySelectedAction_, &QAction::triggered, this, &MainWindow::onCopySelectedAction);
    copySelectedAction_->setShortcut(QApplication::translate("MainWindow", "Ctrl+Shift+C", nullptr));

    copyAllAction_ = new QAction(*copyAllIcon_, tr("Copy All"), this);
    connect(copyAllAction_, &QAction::triggered, this, &MainWindow::onCopyAllAction);
    copyAllAction_->setShortcut(QApplication::translate("MainWindow", "Ctrl+Shift+A", nullptr));

    pasteAction_ = new QAction(*pasteIcon_, tr("Paste"), this);
    connect(pasteAction_, &QAction::triggered, this, &MainWindow::onPasteAction);
    pasteAction_->setShortcut(QApplication::translate("MainWindow", "Ctrl+Shift+V", nullptr));

    findAction_ = new QAction(*findIcon_, tr("Find"), this);
    connect(findAction_, &QAction::triggered, this, &MainWindow::onFindAction);
    findAction_->setShortcut(QApplication::translate("MainWindow", "Ctrl+F", nullptr));

    clearScreenAction_ = new QAction(*clearScreenIcon_, tr("Clear Screen"), this);
    connect(clearScreenAction_, &QAction::triggered, this, &MainWindow::onClearScreenAction);
    clearScreenAction_->setShortcut(QApplication::translate("MainWindow", "Ctrl+Shift+X", nullptr));

    toggleToolbarAction_ = new QAction(*toggleOnIcon_, tr("Toolbar"), this);
    connect(toggleToolbarAction_, &QAction::triggered, this, &MainWindow::onToggleToolbarAction);

    toggleSessionManagerAction_ = new QAction(*toggleOnIcon_, tr("Session Manager"), this);
    connect(toggleSessionManagerAction_, &QAction::triggered, this, &MainWindow::onToggleSessionManagerAction);

    toggleCommandWindowAction_ = new QAction(*toggleOnIcon_, tr("Command Window"), this);
    connect(toggleCommandWindowAction_, &QAction::triggered, this, &MainWindow::onToggleCommandWindowAction);

    toggleCommandButtonAction_ = new QAction(*toggleOnIcon_, tr("Command Button"), this);
    connect(toggleCommandButtonAction_, &QAction::triggered, this, &MainWindow::onToggleCommandButtonAction);
}

void MainWindow::initMenu() {
    mainMenuBar_ = new QMenuBar(this);
    setMenuBar(mainMenuBar_);

    fileMenu_ = new QMenu(tr("File"), mainMenuBar_);
    mainMenuBar_->addAction(fileMenu_->menuAction());
    fileMenu_->addAction(settingsAction_);
    fileMenu_->addAction(toggleSaveLogAction_);
    fileMenu_->addAction(toggleSaveHexLogAction_);
    fileMenu_->addAction(connectAction_);
    fileMenu_->addAction(disConnectAction_);
    fileMenu_->addAction(exitAction_);

    editMenu_ = new QMenu(tr("Edit"), mainMenuBar_);
    mainMenuBar_->addAction(editMenu_->menuAction());
    editMenu_->addAction(copySelectedAction_);
    editMenu_->addAction(copyAllAction_);
    editMenu_->addAction(pasteAction_);
    editMenu_->addAction(findAction_);
    editMenu_->addAction(clearScreenAction_);

    viewMenu_ = new QMenu(tr("View"), mainMenuBar_);
    mainMenuBar_->addAction(viewMenu_->menuAction());
    viewMenu_->addAction(toggleToolbarAction_);
    viewMenu_->addAction(toggleSessionManagerAction_);
    viewMenu_->addAction(toggleCommandWindowAction_);
    viewMenu_->addAction(toggleCommandButtonAction_);

    helpMenu_ = new QMenu(tr("Help"), mainMenuBar_);
    mainMenuBar_->addAction(helpMenu_->menuAction());
}

void MainWindow::initToolbar() {
    toolBar_ = new QToolBar(this);
    toolBar_->setMovable(false);
    addToolBar(Qt::TopToolBarArea, toolBar_);
    toolBar_->addAction(settingsAction_);
    toolBar_->addSeparator();
    toolBar_->addAction(connectAction_);
    toolBar_->addAction(disConnectAction_);
    toolBar_->addSeparator();
    toolBar_->addAction(copyAllAction_);
    toolBar_->addAction(copySelectedAction_);
    toolBar_->addAction(pasteAction_);
    toolBar_->addSeparator();
    toolBar_->addAction(findAction_);
    toolBar_->addAction(clearScreenAction_);
}

void MainWindow::onSettingsAction() {
    SettingDialog settingDialog(this);
    settingDialog.exec();
}

void MainWindow::onConnectAction() {
    if (currentTab_ == nullptr) {
        return;
    }

    currentTab_->connect();
    if (currentTab_->isConnect()) {
        connectAction_->setEnabled(false);
        disConnectAction_->setEnabled(true);
    }

    tabWidget_->setTabIcon(tabWidget_->currentIndex(), *connectStateIcon_);
}

void MainWindow::onExitAction() {
    BaseTerminal *terminal = nullptr;
    while (nullptr != (terminal = dynamic_cast<BaseTerminal *>(tabWidget_->currentWidget()))) {
        terminal->disconnect();
    }
    exit(0);
}

void MainWindow::onCopySelectedAction() {
    if (currentTab_ != nullptr) {
        currentTab_->copyClipboard();
    }
}

void MainWindow::onCopyAllAction() {
    currentTab_->setSelectionStart(0, 0);
    currentTab_->setSelectionEnd(currentTab_->screenLinesCount(), currentTab_->screenColumnsCount());
    currentTab_->copyClipboard();
}

void MainWindow::onPasteAction() {
    if (currentTab_ != nullptr) {
        currentTab_->pasteClipboard();
    }
}

void MainWindow::onFindAction() {
    if (currentTab_ != nullptr) {
        currentTab_->toggleShowSearchBar();
    }
}

void MainWindow::onClearScreenAction() {
    if (currentTab_ != nullptr) {
        currentTab_->clear();
    }
}

void MainWindow::onToggleToolbarAction() {
    if (toolBar_->isHidden()) {
        toolBar_->show();
        toggleToolbarAction_->setIcon(*toggleOnIcon_);
    } else {
        toolBar_->hide();
        toggleToolbarAction_->setIcon(*toggleOffIcon_);
    }
}

void MainWindow::onToggleSessionManagerAction() {
    if (sessionDock_->isHidden()) {
        sessionDock_->show();
        toggleSessionManagerAction_->setIcon(*toggleOnIcon_);
    } else {
        sessionDock_->hide();
        toggleSessionManagerAction_->setIcon(*toggleOffIcon_);
    }
}

void MainWindow::onToggleCommandWindowAction() {
    if (commandWindowDock_->isHidden()) {
        commandWindowDock_->show();
        toggleCommandWindowAction_->setIcon(*toggleOnIcon_);
    } else {
        commandWindowDock_->hide();
        toggleCommandWindowAction_->setIcon(*toggleOffIcon_);
    }
}

void MainWindow::onToggleCommandButtonAction() {
    if (commandButtonBar_->isHidden()) {
        commandButtonBar_->show();
        toggleCommandButtonAction_->setIcon(*toggleOnIcon_);
    } else {
        commandButtonBar_->hide();
        toggleCommandButtonAction_->setIcon(*toggleOffIcon_);
    }
}

void MainWindow::onToggleSaveLogAction() {
}

void MainWindow::onToggleSaveHexLogAction() {

}