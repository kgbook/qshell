#include "ScriptRunner.h"

void ScriptRunner::run() {
    engine_->executeScript(script_, scriptArgs_);
}
