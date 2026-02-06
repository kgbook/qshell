#ifndef QSHELL_SCRIPTRUNNER_H
#define QSHELL_SCRIPTRUNNER_H

#include <QThread>
#include "LuaScriptEngine.h"

// 在单独线程中运行脚本
class ScriptRunner : public QThread {
    Q_OBJECT
public:
    ScriptRunner(LuaScriptEngine* engine, QString  script)
        : engine_(engine), script_(std::move(script)) {}

    void run() override;

private:
    LuaScriptEngine* engine_;
    QString script_;
};
#endif//QSHELL_SCRIPTRUNNER_H
