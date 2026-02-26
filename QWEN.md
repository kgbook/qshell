# qshell - Cross-Platform Terminal Management Tool

## Project Overview

**qshell** is a cross-platform terminal management tool similar to Xshell, SecureCRT, and MobaXterm. It supports Linux, Windows, and macOS platforms with support for multiple protocols including serial, SSH, and local shell.

### Key Features
- Session management
- Multi-tab interface
- Log saving
- Button Bar
- Command Window
- Lua script engine for automation

### Technology Stack
- **Language**: C++17
- **Framework**: Qt6 (Core, Gui, Widgets, Network, SerialPort)
- **Build System**: CMake
- **Terminal Widget**: qtermwidget
- **SSH Library**: libssh2
- **PTY Library**: ptyqt
- **Scripting**: Lua + sol2 wrapper
- **Text Processing**: utf8proc

### Architecture
The project is organized into the following modules:

```
src/
├── core/              # Core utilities (config, crypto)
├── ui/                # User interface components
│   ├── session/       # Session management UI
│   ├── terminal/      # Terminal implementations (SSH, Serial, Local)
│   └── command/       # Command window and button bar
├── scriptengine/      # Lua script engine integration
└── resources/         # Application resources (icons, .qrc)
```

## Building and Running

### Linux (Ubuntu 22.04+)

**Install dependencies:**
```bash
sudo apt install git cmake build-essential qt6-base-dev libqt6serialport6-dev clang-tidy
```

**Build:**
```bash
cmake -B build -S .
cmake --build build
```

**Install:**
```bash
sudo cmake --install build
```

**Alternative:** Use the build script
```bash
./scripts/linux-build.sh
```

### Windows

**Prerequisites:**
- MSVC 2022
- Qt6 (installed via online installer)
- Set `Qt6_DIR` environment variable

**Build and package:**
```powershell
.\scripts\windows-build.ps1
```

### macOS

Build steps are similar to Linux, but dependencies may vary. Not fully tested.

### Packaging (Linux DEB)

```bash
cmake -B build -S .
cmake --build build
cd build
cpack -G DEB
```

## Development Conventions

### Code Style
- **Formatter**: clang-format (LLVM style with customizations)
- **Indentation**: 4 spaces (no tabs)
- **Naming conventions** (enforced by clang-tidy):
  - Classes: `CamelCase`
  - Functions: `camelBack`
  - Variables: `camelBack`
  - Private members: `camelBack_` (with trailing underscore)
  - Protected members: `camelBack_` (with trailing underscore)

### Code Quality
- **Static Analysis**: clang-tidy is enabled on Linux builds
- **Pre-commit hook**: Run `scripts/pre-commit` before committing to auto-format code
- **Line endings**: Unix-style (LF), converted automatically by format script

### Formatting Scripts

**Manual formatting:**
```bash
./scripts/format.sh
```

This script:
1. Converts line endings to Unix (dos2unix)
2. Applies clang-format to all C++ source files
3. Applies cmake-format to CMake files

**Pre-commit hook setup:**
```bash
ln -s ../../scripts/pre-commit .git/hooks/pre-commit
```

### Terminal Implementations

The project has platform-specific SSH terminal implementations:
- **Linux/macOS**: `SSHTerminalLinux.cpp`
- **Windows**: `SSHTerminalWindows.cpp`

### Lua Script Engine

The Lua script engine provides automation capabilities through the `qshell` namespace. Key modules:

- `qshell.*` - Core functions (message, input, log, sleep)
- `qshell.screen.*` - Screen operations (send text, wait for strings)
- `qshell.session.*` - Session management
- `qshell.timer.*` - Timer functions

See `docs/LuaScriptEngine.md` for complete API reference and examples.

## Testing

Currently, the project does not have automated unit tests. Manual testing is required:
1. Build the application
2. Test each protocol (SSH, Serial, Local shell)
3. Verify session management features
4. Test Lua script execution

## Third-Party Dependencies

Located in `third_party/`:
- qtermwidget
- libssh2
- ptyqt
- utf8proc
- Lua
- sol2 (Lua wrapper for C++)

## Project Structure

```
qshell/
├── src/                    # Source code
│   ├── core/              # Core utilities
│   ├── ui/                # UI components
│   ├── scriptengine/      # Lua integration
│   ├── resources/         # Resources (.qrc, icons)
│   ├── main.cpp           # Application entry point
│   └── CMakeLists.txt
├── third_party/           # External dependencies
├── scripts/               # Build and maintenance scripts
├── docs/                  # Documentation
├── .clang-format          # Code formatting rules
├── .clang-tidy            # Static analysis rules
├── CMakeLists.txt         # Root CMake configuration
├── README.md              # Project overview
└── BUILD.md               # Build instructions
```

## Release Process

1. Update `APP_VERSION` in CMakeLists.txt
2. Build for all target platforms
3. Create GitHub release with binaries
4. For Linux: Generate DEB package using CPack

## Known Limitations

- macOS support is experimental
- No automated testing framework
- Windows build requires specific Qt6/MSVC setup

## Contributing

1. Follow the code style conventions (use format script)
2. Run clang-tidy checks before submitting
3. Test on multiple platforms if possible
4. Update documentation for new features
