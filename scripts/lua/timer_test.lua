-- 示例 1: 单次定时器
local timerId = qshell.timer.setTimeout(function()
    qshell.log("This runs after 2 seconds!")
end, 1000)

-- 需要调用 sleep 或 process 来触发定时器
qshell.timer.sleep(5000)

-- 示例 2: 重复定时器
local count = 0
local intervalId = qshell.timer.setInterval(function()
    count = count + 1
    qshell.log("Tick " .. count)
    if count >= 5 then
        qshell.timer.clear(intervalId)
        qshell.log("Timer stopped!")
    end
end, 1000)

-- 使用 timer.sleep 等待，期间定时器会触发
qshell.timer.sleep(6000)

-- 示例 3: 在自定义循环中使用
local done = false
qshell.timer.setTimeout(function()
    done = true
end, 5000)

while not done do
    qshell.timer.process()  -- 手动处理定时器
    -- 做其他事情...
    qshell.sleep(0.1)  -- sleep 也会处理定时器
end

-- 示例 4: 清除所有定时器
qshell.timer.clearAll()

-- 示例 5: 获取活动定时器数量
local activeTimers = qshell.timer.count()
qshell.log("Active timers: " .. activeTimers)
