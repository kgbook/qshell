#include "MainWindow.h"
#include "button_toolbar/CommandButtonBar.h"
#include "session/SessionTabWidget.h"
#include "session/SessionTreeWidget.h"
#include "core/ConfigManager.h"
#include "ui/terminal/BaseTerminal.h"
#include "ui/terminal/SerialTerminal.h"
#include "ui/terminal/LocalTerminal.h"
#include "ui/terminal/SSHTerminal.h"
#include "ui/command/CommandWindow.h"

#include <QDockWidget>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowState(Qt::WindowMaximized);
    setContextMenuPolicy(Qt::NoContextMenu);
    QIcon windowIcon(":/images/application.png");
    setWindowIcon(windowIcon);

    initIcons();
    initTableWidget();
    initSessionTree();
    initCommandWindow();
    initButtonBar();
    initMenuBar();
}

void MainWindow::initIcons() {
    connectIcon_ = new QIcon(":/images/connect.png");
    disconnectIcon_ = new QIcon(":/images/disconnect.png");
    connectStateIcon_ = new QIcon(":/images/connect_state.png");
    disconnectStateIcon_ = new QIcon(":/images/disconnect_state.png");
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
}

void MainWindow::initButtonBar() {
    commandButtonBar_ = new CommandButtonBar(this);
    commandButtonBar_->setMovable(false);
    addToolBar(Qt::BottomToolBarArea, commandButtonBar_);
}

void MainWindow::initMenuBar() {
    // 在菜单栏添加历史记录相关菜单
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    QAction *viewHistoryAction = editMenu->addAction(tr("View Command History..."));
    viewHistoryAction->setShortcut(QKeySequence("Ctrl+Shift+H"));
    connect(viewHistoryAction, &QAction::triggered,
            commandWindow_, &CommandWindow::showHistoryDialog);

    QAction *clearHistoryAction = editMenu->addAction(tr("Clear Command History"));
    connect(clearHistoryAction, &QAction::triggered,
            commandWindow_, &CommandWindow::clearHistory);
}

void MainWindow::onTabChanged(int index) {
    qDebug() << "onTabChanged, index = " << index;

    if (index < 0) {
        currentTab_ = nullptr;
        commandWindow_->setCurrentTerminal(nullptr);
        return;
    }

    currentTab_ = dynamic_cast<BaseTerminal *>(tabWidget_->widget(index));
    commandWindow_->setCurrentTerminal(currentTab_);
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
