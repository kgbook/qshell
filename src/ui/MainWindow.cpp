#include "MainWindow.h"
#include <QDockWidget>
#include "session/SessionTabWidget.h"
#include "session/SessionTreeWidget.h"
#include "button_toolbar/CommandButtonBar.h"

MainWindow::MainWindow(QWidget *parent) {
    setWindowState(Qt::WindowMaximized);
    setContextMenuPolicy(Qt::NoContextMenu);
    QIcon windowIcon(":/images/application.png");
    setWindowIcon(windowIcon);
    initTableWidget();
    initSessionTree();
    initCommandWindow();
    initButtonBar();
}

void MainWindow::initTableWidget() {
    tabWidget_ = new SessionTabWidget(this);
    tabWidget_->setTabsClosable(true);
    tabWidget_->setMovable(true);
    setCentralWidget(tabWidget_);
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
