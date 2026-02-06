#include "LuaScriptEngine.h"
#include <QThread>
#include <QEventLoop>
#include <QTimer>
#include <QRegularExpression>
#include "ui/MainWindow.h"

#include <QMessageBox>

LuaScriptEngine::LuaScriptEngine(MainWindow* mainWindow)
    : QObject(mainWindow), mainWindow_(mainWindow)
{
    lua_.open_libraries(sol::lib::base, sol::lib::string,
                         sol::lib::table, sol::lib::math,
                         sol::lib::os, sol::lib::io);
    registerAPIs();
}

void LuaScriptEngine::registerAPIs()
{
    // 创建顶层命名空间 qshell
    sol::table qshell = lua_.create_named_table("qshell");

    // 注册各模块
    registerAppModule(qshell);
    registerSessionModule(qshell);
}

// ========== qshell.app 模块 ==========
void LuaScriptEngine::registerAppModule(sol::table& qshell) const {
    sol::table app = qshell.create_named("app");

    // qshell.app.MessageBox(msg)
    app.set_function("MessageBox", [this](const std::string& msg) {
        QString qmsg = QString::fromStdString(msg);

        // 在主线程中显示消息框
        QMetaObject::invokeMethod(mainWindow_, [qmsg]() {
            QMessageBox::information(nullptr, "Script Message", qmsg);
        }, Qt::BlockingQueuedConnection);
    });


    // qshell.app.Log(msg)
    app.set_function("Log", [this](const std::string& msg) {
        qDebug() << QString::fromStdString(msg);
    });

    // qshell.app.Sleep(ms)
    app.set_function("Sleep", [this](int ms) {
        if (!running_) return;
        QThread::msleep(ms);
    });

}

// ========== qshell.session 模块 ==========
void LuaScriptEngine::registerSessionModule(sol::table& qshell)
{
    sol::table session = qshell.create_named("session");

    // qshell.session.getCurrentSession() -> Session object
    session.set_function("getCurrentSession", [this]() {
        return createSessionObject();
    });
}

// ========== Session 对象创建 ==========
sol::table LuaScriptEngine::createSessionObject()
{
    sol::table sessionObj = lua_.create_table();
    return sessionObj;
}

bool LuaScriptEngine::executeScript(const QString& scriptPath)
{
    running_ = true;
    try {
        auto result = lua_.script_file(scriptPath.toStdString());
        emit scriptFinished();
        return result.valid();
    } catch (const sol::error& e) {
        emit scriptError(QString::fromStdString(e.what()));
        return false;
    }
    running_ = false;
}

bool LuaScriptEngine::executeCode(const QString& code)
{
    running_ = true;
    try {
        auto result = lua_.script(code.toStdString());
        emit scriptFinished();
        return result.valid();
    } catch (const sol::error& e) {
        emit scriptError(QString::fromStdString(e.what()));
        return false;
    }
    running_ = false;
}

void LuaScriptEngine::stopScript()
{
    running_ = false;
}
