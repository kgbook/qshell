# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

qshell is a cross-platform terminal management tool (Xshell/SecureCRT/MobaXterm class) written in **C++17 with Qt6**. It supports three protocols — SSH, Serial, and local shell — with session management, multi-tab terminals, logging, a Lua automation engine, and a local-only MCP control endpoint. Targets Linux, Windows (MSVC), and macOS.

## Build / run

```bash
cmake -B build -S .            # add -DAPP_VERSION=x.y.z to set version (default 1.0.0)
cmake --build build            # on Windows this also runs windeployqt as a POST_BUILD step
sudo cmake --install build     # Linux/macOS
```

Platform packaging scripts (resolve version from git tag, then build+package):
- `./scripts/linux-build.sh [version]` → DEB via CPack
- `./scripts/macos-build.sh` / `cd build && cpack -G DragNDrop` → DMG
- `.\scripts\windows-build.ps1 -BuildOnly` or `-Version 1.2.3` (PowerShell)

Run a Lua script at startup: `./qshell --script path/to.lua -- arg1 arg2` (args after `--` go to the script).

**clang-tidy is auto-enabled on Linux builds** (`QSLOG_CLANG_TIDY_ENABLE=ON` in root CMakeLists.txt). Code that fails the `.clang-tidy` checks will fail the Linux build — do not disable the option to make errors go away; fix the finding.

## Formatting & commits

```bash
./scripts/format.sh    # dos2unix → clang-format → cmake-format on all non-vendored files
```

To wire this into git: `ln -s ../../scripts/pre-commit .git/hooks/pre-commit` (the hook just sources `scripts/format.sh`).

Commit messages follow the existing history: short, focused, imperative, commonly in Chinese (e.g. `修复会话切换崩溃`). One logical change per commit.

**Naming** (enforced by `.clang-tidy` `readability-identifier-naming`): classes `CamelCase`; functions/variables `camelBack`; private/protected members end with `_`. `.clang-format` is LLVM-based, 4-space indent, no tabs, **`ColumnLimit: 0`** (no line wrapping).

## Testing

There is **no automated test target** (no `ctest`/`add_test`). Validate by clean build + manual UI checks per protocol. For script-engine and MCP changes, exercise representative Lua scripts in `scripts/lua/` (e.g. `timer_test.lua`, `startup_args_test.lua`, `reboot_test.lua`) and the MCP curl checks in `docs/MCP.md`.

## Architecture

### Terminal class hierarchy (src/ui/terminal)
`BaseTerminal : QTermWidget` is the abstract base (PTY lifecycle, logging, context menu, highlight menu). Concrete terminals — `SSHTerminal`, `SerialTerminal`, `LocalTerminal` — implement `connect()`/`disconnect()`. **The SSH terminal is platform-split and selected by CMake** (see `src/CMakeLists.txt`):
- Linux/macOS: `SSHTerminalLinux.cpp`
- Windows: `SSHTerminalWindows.cpp` (libssh2, includes X11 forwarding to a local VcXsrv)

Adding/changing SSH behavior usually means touching both files.

### Config: singleton + serialized structs (src/core)
- `ConfigManager` is a process-wide singleton (`ConfigManager::instance()`) backed by `config.json` in `QStandardPaths::AppConfigLocation`. It owns sessions, groups, button groups, quick buttons, global settings, and window layout, and emits `sessionTreeUpdated` / `buttonGroupsChanged` / `quickButtonsChanged` / `globalSettingsChanged` signals that the UI subscribes to.
- `core/datatype.h` defines every persisted struct (`SessionData`, `SSHConfig`, `SerialConfig`, `QuickButton`, `GlobalSettings`, `WindowLayout`, …), each with hand-written `toJson()`/`fromJson()`. **When you add a field to one of these structs, update both `toJson` and `fromJson` plus the ConfigManager load/save logic, or it won't round-trip.**
- `CryptoHelper` encrypts SSH passwords/passphrases before they land in `config.json`; the MCP session list deliberately strips these secret fields.

### Lua script engine runs on a worker thread (src/scriptengine)
This is the most important constraint to understand before touching scriptengine or MainWindow. Scripts execute on a dedicated `QThread` (`ScriptRunner`), **not** the Qt UI thread. The engine reaches the UI exclusively via blocking marshaling:
```cpp
QMetaObject::invokeMethod(mainWindow_, "slotOrLambda",
    Qt::BlockingQueuedConnection, ...);
```
`LuaScriptEngine` registers modules under the `qshell.*` namespace (`app`, `screen`, `session`, `timer`, `http`). The `screen` module's `waitForString`/`waitForRegex` and the `timer` module are interruptible (`interruptibleSleep`) so a script can be stopped. Any new UI-touching API **must** go through `BlockingQueuedConnection` — never touch widgets directly from the worker thread. Full API reference: `docs/LuaScriptEngine.md`.

### MainWindow is the automation bridge (src/ui/MainWindow)
`MainWindow` exposes the bridge surface for both Lua and MCP as `Q_INVOKABLE` methods (`openSessionById`, `sendTextToCurrent`, `getScreenText`, `switchToTab`, `connectCurrentSession`, …). When adding a capability that should be reachable from scripts or MCP, add a `Q_INVOKABLE` method here first, then bind it from `LuaScriptEngine` and/or `McpToolRegistry`.

### MCP control endpoint (src/mcp)
Local-only Streamable HTTP MCP server (MCP 2025-06-18 JSON-RPC), binds `127.0.0.1:<port>/mcp`, bearer-token auth, off by default (toggled in Settings). Two-layer design:
- `McpHttpServer` — raw `QTcpServer` transport, request framing, auth/origin checks, JSON-RPC dispatch. Returns HTTP 405 for GET (no SSE).
- `McpToolRegistry` — the `tools/list` + `tools/call` implementation. Tool callbacks run on the **UI thread** via `runUiTool` (correct `Qt::ConnectionType` based on caller thread); async waits (`wait_for_string`/`wait_for_regex`) complete through `ToolCallback`.

Changing settings restarts the endpoint (`MainWindow::syncMcpServer`). See `docs/MCP.md` for the tool list and manual curl checks.

## Conventions & gotchas
- `third_party/` is vendored (qtermwidget, libssh2, ptyqt, utf8proc, Lua, sol2). Don't edit it unless intentionally updating a dependency.
- All UI strings and commit history are bilingual (Chinese primary) — match surrounding style.
- Release version is the `APP_VERSION` CMake var; the release process bumps it and builds per-platform packages.
