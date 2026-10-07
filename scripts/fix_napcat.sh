#!/bin/bash
# 1. 检查插件目录
echo "--- 检查插件 ---"
ls -ld ~/.openclaw/plugins/napcat-qq 2>/dev/null || echo "插件目录不存在"

# 2. 尝试重启网关以触发重新连接
echo "--- 重启网关 ---"
openclaw gateway restart

# 3. 等待并检查状态
echo "--- 等待连接 (15s) ---"
sleep 15
openclaw sessions list --label napcat-qq-bot
