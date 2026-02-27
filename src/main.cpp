#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QStyleFactory>
#include <QTimer>
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

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(qtermwidget);
    QApplication a(argc, argv);
    showConsole(ConfigManager::instance()->globalSettings().debug);
    qDebug() << "style list:" << QStyleFactory::keys();
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // 设置应用信息
    QCoreApplication::setApplicationName("qshell");
    QCoreApplication::setApplicationVersion(APP_VERSION);
    QCoreApplication::setOrganizationName("qiushao");
    QCoreApplication::setOrganizationDomain("https://github.com/qiushao/qshell");

    // 读取版本信息
    QString version = QCoreApplication::applicationVersion();
    qDebug() << "Version:" << version;

    QCommandLineParser parser;
    parser.setApplicationDescription("QShell Terminal Emulator");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption scriptOption(
        QStringList() << "script",
        "Execute Lua script on startup.",
        "path"
    );
    parser.addOption(scriptOption);
    parser.process(a);

    const QString startupScriptPath = parser.value(scriptOption).trimmed();

    MainWindow w;
    w.show();

    if (!startupScriptPath.isEmpty()) {
        QTimer::singleShot(0, &w, [startupScriptPath, &w]() {
            w.runScriptAtStartup(startupScriptPath);
        });
    }

    return QApplication::exec();
}
