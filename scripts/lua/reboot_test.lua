-- 使用 qshell.app 模块
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

