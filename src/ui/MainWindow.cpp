#include "MainWindow.h"
#include "button_toolbar/CommandButtonBar.h"
#include "session/SessionTabWidget.h"
#include "session/SessionTreeWidget.h"
#include "core/ConfigManager.h"
#include "ui/terminal/BaseTerminal.h"
#include "ui/terminal/SerialTerminal.h"
#include <QDockWidget>

MainWindow::MainWindow(QWidget *parent) {
    setWindowState(Qt::WindowMaximized);
    setContextMenuPolicy(Qt::NoContextMenu);
    QIcon windowIcon(":/images/application.png");
    setWindowIcon(windowIcon);
    initIcons();
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
    commandEditor_ = new QTextEdit(this);
    commandWindowDock_->setWidget(commandEditor_);
    commandWindowDock_->setFeatures(QDockWidget::NoDockWidgetFeatures);
    commandEditor_->setMinimumHeight(50);
    addDockWidget(Qt::BottomDockWidgetArea, commandWindowDock_);
    resizeDocks({commandWindowDock_}, {50}, Qt::Vertical);
    commandEditor_->installEventFilter(this);

    //disable title bar
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

void MainWindow::onTabChanged(int index) {
    qDebug() << "onTabChanged, index = " << index;
    if (index < 0) {
        // _connectAction->setEnabled(false);
        // _disConnectAction->setEnabled(false);
        // _toggleSaveLogAction->setEnabled(false);
        // _toggleSaveHexLogAction->setEnabled(false);
        currentTab_ = nullptr;
        return;
    }

    currentTab_ = dynamic_cast<BaseTerminal *>(tabWidget_->widget(index));
    // if (_currentTab->isConnect()) {
    //     _connectAction->setEnabled(false);
    //     _disConnectAction->setEnabled(true);
    // } else {
    //     _connectAction->setEnabled(true);
    //     _disConnectAction->setEnabled(false);
    // }

    // _toggleSaveLogAction->setEnabled(true);
    // _toggleSaveHexLogAction->setEnabled(true);
    // if (_currentTab->isLoggingSession()) {
    //     _toggleSaveLogAction->setIcon(*_toggleOnIcon);
    // } else {
    //     _toggleSaveLogAction->setIcon(*_toggleOffIcon);
    // }

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
    if (!currentTab_->isConnect()) {
        // _connectAction->setEnabled(true);
        // _disConnectAction->setEnabled(false);
    }

    tabWidget_->setTabIcon(tabWidget_->currentIndex(), *disconnectStateIcon_);
}