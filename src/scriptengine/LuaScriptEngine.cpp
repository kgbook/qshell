#include "LuaScriptEngine.h"
#include <QThread>
#include <QEventLoop>
#include <QTimer>
#include <QRegularExpression>
#include "ui/MainWindow.h"
#include "ui/terminal/BaseTerminal.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QInputDialog>
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
    registerSessionModule(qshell);
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

    // qshell.input(prompt, [defaultValue], [title])
    // 弹出输入对话框，返回用户输入的字符串
    // 如果用户点击取消，返回空字符串
    qshell.set_function("input", [this](const std::string& prompt,
                                         sol::optional<std::string> defaultValue,
                                         sol::optional<std::string> title) -> std::string {
        QString qprompt = QString::fromStdString(prompt);
        QString qdefault = defaultValue ? QString::fromStdString(defaultValue.value()) : QString();
        QString qtitle = title ? QString::fromStdString(title.value()) : "Script Input";
        QString result;
        bool ok = false;

        // 在主线程中显示输入对话框
        QMetaObject::invokeMethod(mainWindow_, [qtitle, qprompt, qdefault, &result, &ok]() {
            result = QInputDialog::getText(nullptr, qtitle, qprompt,
                                           QLineEdit::Normal, qdefault, &ok);
        }, Qt::BlockingQueuedConnection);

        // 用户点击取消时返回空字符串
        if (!ok) {
            return "";
        }
        return result.toStdString();
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
            Qt::BlockingQueuedConnection,
            Q_ARG(QString, qstr));
    });

    // qshell.screen.sendKey(keyName) 发送特殊按键
    // 支持: "Enter", "Tab", "Escape", "Backspace", "Delete",
    //       "Up", "Down", "Left", "Right", "Home", "End",
    //       "PageUp", "PageDown", "F1"-"F12", "Ctrl+C" 等
    screen.set_function("sendKey", [this](const std::string& keyName) {
        QString qkey = QString::fromStdString(keyName);
        QMetaObject::invokeMethod(mainWindow_, "onSendKey",
            Qt::BlockingQueuedConnection,
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
            Qt::BlockingQueuedConnection);
    });

    // qshell.screen.waitForString(str, timeoutSeconds)
    // 返回: true=找到, false=超时
    screen.set_function("waitForString", [this](const std::string& str, int timeoutSeconds) -> bool {
        isWaitForString_ = true;
        waitForString_ = QString::fromStdString(str);
        findWaitForString_ = false;
        auto currentSession = mainWindow_->getCurrentSession();
        QObject::connect(currentSession, &QTermWidget::onNewLine, this, &LuaScriptEngine::onDisplayOutput);
        auto endTime = std::chrono::steady_clock::now()
                          + std::chrono::milliseconds(static_cast<int>(timeoutSeconds * 1000));

            // 分段 sleep，每 100ms 检查一次停止标志
            while (std::chrono::steady_clock::now() < endTime) {
                if (gShouldStop.load()) {
                    throw std::runtime_error("interrupted during sleep");
                }

                if (findWaitForString_) {
                    isWaitForString_ = false;
                    QObject::disconnect(currentSession, &QTermWidget::onNewLine, this, &LuaScriptEngine::onDisplayOutput);
                    return true;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

        QObject::disconnect(currentSession, &QTermWidget::onNewLine, this, &LuaScriptEngine::onDisplayOutput);
        return false; // 超时
    });

    // qshell.screen.waitForRegexp(pattern, timeoutSeconds)
    // 返回: true=匹配成功, false=超时
    // 示例: qshell.screen.waitForRegexp("\\$\\s*$", 10)  -- 等待 shell 提示符
    screen.set_function("waitForRegexp", [this](const std::string& pattern, int timeoutSeconds) -> bool {
        isWaitForRegexp_ = true;
        waitForRegexp_ = QRegularExpression(QString::fromStdString(pattern));
        findWaitForRegexp_ = false;
        lastRegexpMatch_.clear();

        // 检查正则表达式是否有效
        if (!waitForRegexp_.isValid()) {
            qWarning() << "Invalid regexp pattern:" << waitForRegexp_.errorString();
            isWaitForRegexp_ = false;
            return false;
        }

        auto currentSession = mainWindow_->getCurrentSession();
        QObject::connect(currentSession, &QTermWidget::onNewLine,
                         this, &LuaScriptEngine::onDisplayOutput);

        auto endTime = std::chrono::steady_clock::now()
                          + std::chrono::milliseconds(static_cast<int>(timeoutSeconds * 1000));

        while (std::chrono::steady_clock::now() < endTime) {
            if (gShouldStop.load()) {
                isWaitForRegexp_ = false;
                QObject::disconnect(currentSession, &QTermWidget::onNewLine,
                                   this, &LuaScriptEngine::onDisplayOutput);
                throw std::runtime_error("interrupted during waitForRegexp");
            }

            if (findWaitForRegexp_) {
                isWaitForRegexp_ = false;
                QObject::disconnect(currentSession, &QTermWidget::onNewLine,
                                   this, &LuaScriptEngine::onDisplayOutput);
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        isWaitForRegexp_ = false;
        QObject::disconnect(currentSession, &QTermWidget::onNewLine,
                           this, &LuaScriptEngine::onDisplayOutput);
        return false; // 超时
    });

    // 可选：获取最后一次正则匹配的内容
    screen.set_function("getLastMatch", [this]() -> std::string {
        return lastRegexpMatch_.toStdString();
    });


}

void LuaScriptEngine::registerSessionModule(sol::table &qshell) {
    sol::table session = qshell.create_named("session");

    // 打开会话
    session.set_function("open", [this](const std::string& sessionName) -> bool {
        bool ok = false;
        QMetaObject::invokeMethod(mainWindow_, [this, sessionName, &ok]() {
            ok = mainWindow_->openSessionByName(sessionName.data());
        }, Qt::BlockingQueuedConnection);
        return ok;
    });

    // 获取当前会话名
    session.set_function("tabName", [this]() -> std::string {
        auto currentSession = mainWindow_->getCurrentSession();
        if (currentSession != nullptr) {
            return currentSession->getSessionName().toStdString();
        }
        return "";
    });

    //切换到下一个 tab
    session.set_function("nextTab", [this]() {
        QMetaObject::invokeMethod(mainWindow_, [this]() {
            mainWindow_->nextTab();
        }, Qt::BlockingQueuedConnection);
    });

    //切换到 tabName 的 tab
    session.set_function("switchToTab", [this](const std::string& tabName) -> bool {
        bool ok = false;
        QMetaObject::invokeMethod(mainWindow_, [this, tabName, &ok]() {
            ok = mainWindow_->switchToTab(tabName.data());
        }, Qt::BlockingQueuedConnection);
        return ok;
    });

    // qshell.session.connect() 连接
    session.set_function("connect", [this]() {
        QMetaObject::invokeMethod(mainWindow_, "onConnectAction",
            Qt::BlockingQueuedConnection);
    });

    // qshell.session.disconnect() 连接
    session.set_function("disconnect", [this]() {
        QMetaObject::invokeMethod(mainWindow_, "onDisconnectAction",
            Qt::BlockingQueuedConnection);
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

void LuaScriptEngine::onDisplayOutput(const QString &line) {
    // qDebug() << "onDisplayOutput:" << line;
    if (isWaitForString_) {
        if (line.contains(waitForString_)) {
            findWaitForString_ = true;
        }
    }

    // 处理正则表达式匹配
    if (isWaitForRegexp_) {
        QRegularExpressionMatch match = waitForRegexp_.match(line);
        if (match.hasMatch()) {
            findWaitForRegexp_ = true;
            lastRegexpMatch_ = match.captured(0);  // 保存匹配内容
        }
    }
}