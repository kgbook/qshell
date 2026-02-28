-- 启动参数示例：
--   qshell --script scripts/lua/startup_args_test.lua -- dev_session "echo hello" 2
--
-- 约定：
--   arg[0] = 脚本路径
--   arg[1..n] = 传入参数

local sessionName = arg[1]
local command = arg[2]
local repeatCount = tonumber(arg[3] or "1")

if sessionName == nil or command == nil then
    qshell.showMessage("usage: <sessionName> <command> [repeatCount]")
    return
end

if repeatCount == nil or repeatCount < 1 then
    qshell.showMessage("repeatCount 必须是 >= 1 的数字")
    return
end

qshell.log("script path: " .. tostring(arg[0]))
qshell.log("sessionName: " .. sessionName)
qshell.log("command: " .. command)
qshell.log("repeatCount: " .. tostring(repeatCount))

if not qshell.session.open(sessionName) then
    qshell.showMessage("open session failed: " .. sessionName)
    return
end

for i = 1, repeatCount do
    qshell.screen.sendText(command .. "\r")
    qshell.sleep(1)
end

qshell.showMessage("startup_args_test.lua finished")
qshell.exit()
