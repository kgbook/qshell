#ifndef QSHELL_MAINWINDOW_H
#define QSHELL_MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>

class SessionTabWidget;
class SessionTreeWidget;
class CommandButtonBar;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private:
    void initSessionTree();
    void initTableWidget();
    void initCommandWindow();
    void initButtonBar();

    // session table
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
