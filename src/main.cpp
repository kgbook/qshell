#include <QApplication>
#include <QStyleFactory>
#include "ui/MainWindow.h"
#include "core/ConfigManager.h"

#ifdef _WIN32
#include <windows.h>
#include <cstdio>
#endif

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <sol/sol.hpp>

static void showConsole(bool show)
{
#ifdef _WIN32
    if (show)
    {
        AllocConsole();

        // 把 stdout / stderr 重定向到控制台（这样 printf / std::cout / qDebug 都能看到）
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        SetConsoleTitleW(L"Debug Console");
    }
    else
    {
        FreeConsole();
    }
#endif
}

static void luatest() {
    // 测试 Lua 集成
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    lua.script(R"(
        print("Hello from Lua " .. _VERSION)
    )");

    // 测试 Qt 与 Lua 交互
    lua.set_function("qtVersion", []() {
        return qVersion();
    });

    lua.script(R"(
        print("Qt version: " .. qtVersion())
    )");

    qDebug() << "Lua integration successful!";
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(qtermwidget);
    QApplication a(argc, argv);
    showConsole(ConfigManager::instance()->globalSettings().debug);
    qDebug() << "style list:" << QStyleFactory::keys();
    luatest();
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    MainWindow w;
    w.show();
    return QApplication::exec();
}