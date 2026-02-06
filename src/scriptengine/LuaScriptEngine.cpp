#include "LuaScriptEngine.h"
#include <QThread>
#include <QEventLoop>
#include <QTimer>
#include <QRegularExpression>
#include "ui/MainWindow.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <thread>

// 全局停止标志（线程安全）
std::atomic<bool> gShouldStop{false};

// 钩子函数：每执行一定数量的指令就检查是否需要停止
void interruptHook(lua_State* L, lua_Debug* ar) {
    if (gShouldStop.load()) {
        // 抛出 Lua 错误来中断执行
        luaL_error(L, "Script execution interrupted by user");
    }
}

LuaScriptEngine::LuaScriptEngine(MainWindow* mainWindow)
    : QObject(mainWindow), mainWindow_(mainWindow)
{
    lua_.open_libraries(sol::lib::base, sol::lib::string,
                         sol::lib::table, sol::lib::math,
                         sol::lib::os, sol::lib::io);
    registerAPIs();
    // 设置钩子：每执行 1000 条指令检查一次
    // LUA_MASKCOUNT - 按指令计数触发
    // LUA_MASKLINE  - 每行触发（更精确但更慢）
    lua_sethook(lua_.lua_state(), interruptHook, LUA_MASKCOUNT, 1000);
}

void LuaScriptEngine::registerAPIs()
{
    // 创建顶层命名空间 qshell
    sol::table qshell = lua_.create_named_table("qshell");

    // 注册各模块
    registerAppModule(qshell);
    registerScreenModule(qshell);
}

// ========== qshell 模块 ==========
void LuaScriptEngine::registerAppModule(sol::table& qshell) const {
    // qshell.showMessage(msg)
    qshell.set_function("showMessage", [this](const std::string& msg) {
        QString qmsg = QString::fromStdString(msg);

        // 在主线程中显示消息框
        QMetaObject::invokeMethod(mainWindow_, [qmsg]() {
            QMessageBox::information(nullptr, "Script Message", qmsg);
        }, Qt::BlockingQueuedConnection);
    });


    // qshell.log(msg)
    qshell.set_function("log", [](const std::string& msg) {
        qDebug() << QString::fromStdString(msg);
    });

    // qshell.sleep(seconds)
    qshell.set_function("sleep", [](int seconds) {
        auto endTime = std::chrono::steady_clock::now()
                          + std::chrono::milliseconds(static_cast<int>(seconds * 1000));

            // 分段 sleep，每 100ms 检查一次停止标志
            while (std::chrono::steady_clock::now() < endTime) {
                if (gShouldStop.load()) {
                    throw std::runtime_error("interrupted during sleep");
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
    });

    // qshell.getVersionStr()
    qshell.set_function("getVersionStr", []() {
        return QCoreApplication::applicationVersion().toStdString();
    });

}

// ========== qshell.screen 模块 ==========
void LuaScriptEngine::registerScreenModule(sol::table& qshell)
{
    sol::table screen = qshell.create_named("screen");

    // qshell.screen.sendText(command) 发送指令
    screen.set_function("sendText", [this](const std::string& command) {
        auto qstr = QString::fromStdString(command);
        QMetaObject::invokeMethod(mainWindow_, "onCommandSend",
            Qt::QueuedConnection,
            Q_ARG(QString, qstr));
    });

    // qshell.screen.sendKey(keyName) 发送特殊按键
    // 支持: "Enter", "Tab", "Escape", "Backspace", "Delete",
    //       "Up", "Down", "Left", "Right", "Home", "End",
    //       "PageUp", "PageDown", "F1"-"F12", "Ctrl+C" 等
    screen.set_function("sendKey", [this](const std::string& keyName) {
        QString qkey = QString::fromStdString(keyName);
        QMetaObject::invokeMethod(mainWindow_, "onSendKey",
            Qt::QueuedConnection,
            Q_ARG(QString, qkey));
    });

    // qshell.screen.getScreenText() 获取当前屏幕文本
    screen.set_function("getScreenText", [this]() -> std::string {
        QString text;
        QMetaObject::invokeMethod(mainWindow_, "getScreenText",
            Qt::BlockingQueuedConnection,
            Q_RETURN_ARG(QString, text));
        return text.toStdString();
    });

    // qshell.screen.clear() 清屏
    screen.set_function("clear", [this]() {
        QMetaObject::invokeMethod(mainWindow_, "onClearScreenAction",
            Qt::QueuedConnection);
    });
}

bool LuaScriptEngine::executeScript(const QString& scriptPath)
{
    running_ = true;
    try {
        auto result = lua_.script_file(scriptPath.toStdString());
        running_ = false;
        emit scriptFinished();
        return result.valid();
    } catch (const sol::error& e) {
        running_ = false;
        emit scriptError(QString::fromStdString(e.what()));
        return false;
    }
}

bool LuaScriptEngine::executeCode(const QString& code)
{
    running_ = true;
    try {
        auto result = lua_.script(code.toStdString());
        running_ = false;
        emit scriptFinished();
        return result.valid();
    } catch (const sol::error& e) {
        running_ = false;
        emit scriptError(QString::fromStdString(e.what()));
        return false;
    }
}
bool LuaScriptEngine::isRunning() {
    return  running_;
}

void LuaScriptEngine::stopScript()
{
    qDebug() << "stopScript";
    gShouldStop = true;
}
