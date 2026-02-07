// LuaScriptEngine.h
#pragma once
#include <QObject>
#include <QString>
#include <QRegularExpression>
#include <sol/sol.hpp>

class MainWindow;

class LuaScriptEngine : public QObject {
    Q_OBJECT
public:
    explicit LuaScriptEngine(MainWindow *window);

    bool executeScript(const QString &scriptPath);
    bool executeCode(const QString &code);
    bool isRunning();
    static void stopScript();


signals:
    void scriptError(const QString &error);
    void scriptFinished();

private:
    void registerAPIs();
    void registerAppModule(sol::table &qshell) const;
    void registerScreenModule(sol::table &qshell);
    void onDisplayOutput(const QString &line);

    sol::state lua_;
    MainWindow *mainWindow_ = nullptr;
    std::atomic<bool> running_{false};
    bool isWaitForString_ = false;
    bool findWaitForString_ = false;
    QString waitForString_;

    // 新增 waitForRegexp 相关变量
    bool isWaitForRegexp_ = false;
    QRegularExpression waitForRegexp_;
    bool findWaitForRegexp_ = false;
    QString lastRegexpMatch_;  // 可选：保存最后匹配的内容
};
