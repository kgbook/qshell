// LuaScriptEngine.h
#pragma once
#include <QObject>
#include <QString>
#include <sol/sol.hpp>

class MainWindow;

class LuaScriptEngine : public QObject {
    Q_OBJECT
public:
    explicit LuaScriptEngine(MainWindow *window);

    bool executeScript(const QString &scriptPath);
    bool executeCode(const QString &code);
    void stopScript();

signals:
    void scriptOutput(const QString &message);
    void scriptError(const QString &error);
    void scriptFinished();

private:
    void registerAPIs();
    void registerAppModule(sol::table &qshell) const;
    void registerSessionModule(sol::table &qshell);
    sol::table createSessionObject();

    sol::state lua_;
    MainWindow *mainWindow_ = nullptr;
    std::atomic<bool> running_{false};
};
