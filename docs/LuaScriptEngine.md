# QShell Lua 脚本使用文档

## 概述

QShell 提供了内置的 Lua 脚本引擎，用于终端会话的自动化操作。所有 API 都通过 `qshell` 命名空间访问。

---

## API 参考

### 1. 核心模块 (`qshell`)

#### `qshell.showMessage(msg)`
弹出消息对话框。

| 参数 | 类型 | 说明 |
|------|------|------|
| `msg` | string | 要显示的消息内容 |

```lua
qshell.showMessage("操作完成！")
```

---

#### `qshell.input(prompt, [defaultValue], [title])`
弹出输入对话框，获取用户输入。

| 参数 | 类型 | 说明 |
|------|------|------|
| `prompt` | string | 提示信息 |
| `defaultValue` | string (可选) | 默认值 |
| `title` | string (可选) | 对话框标题，默认为 "Script Input" |

**返回值**: `string` - 用户输入的内容，取消时返回空字符串

```lua
local username = qshell.input("请输入用户名:", "admin", "登录")
if username ~= "" then
    qshell.log("用户名: " .. username)
end
```

---

#### `qshell.log(msg)`
输出调试日志（输出到 Qt 调试控制台）。

```lua
qshell.log("脚本开始执行...")
```

---

#### `qshell.sleep(seconds)`
暂停脚本执行指定秒数。

| 参数 | 类型 | 说明 |
|------|------|------|
| `seconds` | number | 等待的秒数 |

```lua
qshell.sleep(2)  -- 等待 2 秒
```

---

#### `qshell.getVersionStr()`
获取应用程序版本号。

**返回值**: `string` - 版本号字符串

```lua
local version = qshell.getVersionStr()
qshell.log("当前版本: " .. version)
```

---

### 2. 屏幕模块 (`qshell.screen`)

#### `qshell.screen.sendText(text)`
向当前终端发送文本（不自动添加换行符）。

```lua
qshell.screen.sendText("ls -la")
qshell.screen.sendKey("Enter")
```

---

#### `qshell.screen.sendKey(keyName)`
发送特殊按键。

**支持的按键**:
| 按键类型 | 值 |
|---------|-----|
| 功能键 | `Enter`, `Tab`, `Escape`, `Backspace`, `Delete` |
| 方向键 | `Up`, `Down`, `Left`, `Right` |
| 导航键 | `Home`, `End`, `PageUp`, `PageDown` |
| F键 | `F1` - `F12` |
| 组合键 | `Ctrl+C`, `Ctrl+D` 等 |

```lua
qshell.screen.sendKey("Enter")
qshell.screen.sendKey("Ctrl+C")
```

---

#### `qshell.screen.getScreenText()`
获取当前屏幕的全部文本内容。

**返回值**: `string` - 屏幕文本

```lua
local content = qshell.screen.getScreenText()
qshell.log(content)
```

---

#### `qshell.screen.clear()`
清除当前终端屏幕。

```lua
qshell.screen.clear()
```

---

#### `qshell.screen.waitForString(str, timeoutSeconds)`
等待屏幕上出现指定字符串。

| 参数 | 类型 | 说明 |
|------|------|------|
| `str` | string | 要等待的字符串 |
| `timeoutSeconds` | number | 超时时间（秒） |

**返回值**: `boolean` - `true` 表示找到，`false` 表示超时

```lua
qshell.screen.sendText("ssh user@server")
qshell.screen.sendKey("Enter")

if qshell.screen.waitForString("password:", 30) then
    qshell.screen.sendText("mypassword")
    qshell.screen.sendKey("Enter")
else
    qshell.showMessage("连接超时！")
end
```

---

#### `qshell.screen.waitForRegexp(pattern, timeoutSeconds)`
等待屏幕上出现匹配正则表达式的内容。

| 参数 | 类型 | 说明 |
|------|------|------|
| `pattern` | string | 正则表达式模式 |
| `timeoutSeconds` | number | 超时时间（秒） |

**返回值**: `boolean` - `true` 表示匹配成功，`false` 表示超时

```lua
-- 等待 shell 提示符 ($ 或 #)
if qshell.screen.waitForRegexp("[\\$#]\\s*$", 10) then
    qshell.log("已就绪")
end
```

---

#### `qshell.screen.getLastMatch()`
获取最后一次正则匹配的内容。

**返回值**: `string` - 匹配的字符串

```lua
if qshell.screen.waitForRegexp("IP: ([0-9.]+)", 5) then
    local match = qshell.screen.getLastMatch()
    qshell.log("匹配到: " .. match)
end
```

---

### 3. 会话模块 (`qshell.session`)

#### `qshell.session.open(sessionName)`
通过名称打开已保存的会话。

| 参数 | 类型 | 说明 |
|------|------|------|
| `sessionName` | string | 会话名称 |

**返回值**: `boolean` - 是否成功打开

```lua
if qshell.session.open("MyServer") then
    qshell.log("会话已打开")
end
```

---

#### `qshell.session.tabName()`
获取当前标签页的会话名称。

**返回值**: `string` - 会话名称

```lua
local name = qshell.session.tabName()
qshell.log("当前会话: " .. name)
```

---

#### `qshell.session.nextTab()`
切换到下一个标签页。

```lua
qshell.session.nextTab()
```

---

#### `qshell.session.switchToTab(tabName)`
切换到指定名称的标签页。

| 参数 | 类型 | 说明 |
|------|------|------|
| `tabName` | string | 标签页名称 |

**返回值**: `boolean` - 是否成功切换

```lua
if qshell.session.switchToTab("Server1") then
    qshell.log("已切换到 Server1")
end
```

---

#### `qshell.session.connect()`
连接当前会话。

```lua
qshell.session.connect()
```

---

#### `qshell.session.disconnect()`
断开当前会话连接。

```lua
qshell.session.disconnect()
```

---

### 4. 定时器模块 (`qshell.timer`)

#### `qshell.timer.setTimeout(callback, delayMs)`
创建单次定时器，在指定延迟后执行回调函数。

| 参数 | 类型 | 说明 |
|------|------|------|
| `callback` | function | 定时器触发时执行的回调函数 |
| `delayMs` | number | 延迟时间（毫秒） |

**返回值**: `number` - 定时器 ID，可用于取消定时器

```lua
local id = qshell.timer.setTimeout(function()
    qshell.log("3秒后执行！")
end, 3000)
```

---

#### `qshell.timer.setInterval(callback, intervalMs)`
创建重复定时器，按指定间隔周期性执行回调函数。

| 参数 | 类型 | 说明 |
|------|------|------|
| `callback` | function | 定时器触发时执行的回调函数 |
| `intervalMs` | number | 间隔时间（毫秒） |

**返回值**: `number` - 定时器 ID，可用于取消定时器

```lua
local count = 0
local id = qshell.timer.setInterval(function()
    count = count + 1
    qshell.log("心跳检测 #" .. count)
end, 5000)

-- 稍后取消
-- qshell.timer.clear(id)
```

---

#### `qshell.timer.clear(timerId)`
取消指定的定时器。

| 参数 | 类型 | 说明 |
|------|------|------|
| `timerId` | number | 要取消的定时器 ID |

**返回值**: `boolean` - `true` 表示成功取消，`false` 表示未找到该定时器

```lua
local id = qshell.timer.setTimeout(function()
    qshell.log("这条消息不会显示")
end, 5000)

-- 在定时器触发前取消
qshell.timer.clear(id)
```

---

#### `qshell.timer.clearAll()`
取消所有活动的定时器。

```lua
-- 清理所有定时器
qshell.timer.clearAll()
qshell.log("所有定时器已清除")
```

---

#### `qshell.timer.process()`
手动处理所有到期的定时器回调。在长时间运行的循环中应定期调用此函数，以确保定时器能够正常触发。

```lua
-- 在自定义循环中处理定时器
while running do
    -- 执行其他操作...
    
    -- 处理到期的定时器
    qshell.timer.process()
    
    qshell.sleep(0.1)
end
```

---

#### `qshell.timer.count()`
获取当前活动定时器的数量。

**返回值**: `number` - 活动定时器数量

```lua
local n = qshell.timer.count()
qshell.log("当前有 " .. n .. " 个活动定时器")
```


## 完整示例

### 示例 1： reboot 压测

```lua
versionInfo = qshell.getVersionStr()
qshell.log("脚本开始执行..." .. versionInfo)

ret = qshell.session.open("ttyUSB1")
if (ret == false) then
    qshell.showMessage("open session ttyUSB1 failed")
    return
end
qshell.sleep(1)
qshell.screen.sendText("su\r")
qshell.screen.sendText("setprop persist.auto.logd.enable 1\r")

ret = qshell.session.open("bash")
if (ret == false) then
    qshell.showMessage("open session bash failed")
    return
end

qshell.sleep(1)
qshell.screen.sendText("mkdir -p ~/reboot_log\r")
qshell.screen.sendText("cd ~/reboot_log\r")

for i = 1, 10 do
    ret = qshell.session.switchToTab("ttyUSB1");
    if (ret == false) then
        qshell.showMessage("switchToTab ttyUSB1 failed")
        return
    end
    qshell.sleep(1)

    qshell.screen.sendText("reboot\r")
    qshell.sleep(3)
    ret = qshell.screen.waitForString("Starting kernel ...", 30)
    if (ret == false) then
        qshell.showMessage("wait kernel start failed")
        return
    end

    qshell.sleep(7)
    qshell.screen.sendText("\r")
    qshell.screen.sendText("\r")
    ret = qshell.screen.waitForString("console:/ $", 30)
    if (ret == false) then
        qshell.showMessage("wait console start failed")
        return
    end

    qshell.sleep(1)
    qshell.screen.sendText("\r")
    qshell.screen.sendText("su\r")
    qshell.sleep(1)
    qshell.screen.sendText("echo 0 > /proc/sys/kernel/printk\r")
    qshell.sleep(1)
    qshell.screen.sendText("start adbd\r")
    qshell.sleep(1)
    qshell.screen.sendText("logcat > /data/reboot" .. i .. ".log&\r")
    qshell.sleep(1)
    qshell.screen.sendText("dmesg -w > /data/dmesg" .. i .. ".log&\r")
    qshell.sleep(30)
    qshell.screen.sendText("fg\r")
    qshell.sleep(1)
    qshell.screen.sendKey("Ctrl+C")
    qshell.sleep(1)
    qshell.screen.sendText("fg\r")
    qshell.sleep(1)
    qshell.screen.sendKey("Ctrl+C")
    qshell.sleep(1)

    qshell.screen.sendText("ifconfig\r")
    qshell.screen.sendText("\r")
    qshell.sleep(1)
    local str = qshell.screen.getScreenText()
    local ip = str:match("eth0.-inet addr:([%d%.]+)")
    if (ip == false or ip == nil) then
        qshell.showMessage("get ip failed")
        return
    end

    ret = qshell.session.switchToTab("bash")
    if (ret == false) then
        qshell.showMessage("switchToTab bash failed")
        return
    end
    qshell.sleep(1)

    qshell.screen.sendText("\r")
    qshell.sleep(1)
    qshell.screen.sendText("adb connect " .. ip .. "\r")
    qshell.sleep(2)
    qshell.screen.sendText("adb root\r")
    qshell.sleep(1)
    qshell.screen.sendText("adb pull /data/reboot" .. i .. ".log ~/reboot_log/\r")
    qshell.sleep(1)
    qshell.screen.sendText("adb pull /data/dmesg" .. i .. ".log ~/reboot_log/\r")
    qshell.sleep(1)
    qshell.screen.sendText("adb disconnect\r")
    qshell.sleep(2)
end

qshell.showMessage("exec script finish")

```


