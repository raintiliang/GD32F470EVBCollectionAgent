#!/bin/bash
# Temperature Scenes技能安装脚本

set -e

SKILL_NAME="temperature-scenes"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SKILL_DIR="$SCRIPT_DIR"
TARGET_DIR=""

echo "安装Temperature Scenes技能"
echo "=========================="

# 检测安装类型
if [ "$1" = "--global" ]; then
    # 全局安装（需要管理员权限）
    if [ "$EUID" -ne 0 ]; then
        echo "全局安装需要root权限"
        echo "请使用sudo运行: sudo $0 --global"
        exit 1
    fi
    
    OPENCLAW_DIR="/usr/local/lib/node_modules/openclaw"
    if [ ! -d "$OPENCLAW_DIR" ]; then
        OPENCLAW_DIR="/usr/lib/node_modules/openclaw"
    fi
    if [ ! -d "$OPENCLAW_DIR" ]; then
        OPENCLAW_DIR="$(dirname $(which openclaw))/../lib/node_modules/openclaw"
    fi
    
    if [ ! -d "$OPENCLAW_DIR" ]; then
        echo "错误: 找不到OpenClaw安装目录"
        exit 1
    fi
    
    TARGET_DIR="$OPENCLAW_DIR/skills/$SKILL_NAME"
    echo "安装到: $TARGET_DIR (全局)"
    
else
    # 本地安装到用户目录
    USER_SKILLS_DIR="$HOME/.openclaw/skills"
    mkdir -p "$USER_SKILLS_DIR"
    TARGET_DIR="$USER_SKILLS_DIR/$SKILL_NAME"
    echo "安装到: $TARGET_DIR (用户本地)"
fi

# 复制文件
echo "复制技能文件..."
if [ -d "$TARGET_DIR" ]; then
    echo "目标目录已存在，备份为 $TARGET_DIR.backup"
    mv "$TARGET_DIR" "$TARGET_DIR.backup"
fi

cp -r "$SKILL_DIR" "$TARGET_DIR"

# 设置文件权限
find "$TARGET_DIR" -type f -name "*.py" -exec chmod +x {} \;
find "$TARGET_DIR" -type f -name "*.sh" -exec chmod +x {} \;

echo "安装完成!"
echo ""
echo "使用方法:"
echo "1. 技能目录: $TARGET_DIR"
echo "2. Python模块: import temperature_config"
echo "3. 命令行工具: python3 $TARGET_DIR/scripts/temperature_cli.py"
echo ""
echo "集成到OpenClaw:"
echo "1. 确保OpenClaw配置中包含技能目录"
echo "2. 或手动在OpenClaw中引用技能"
echo ""
echo "测试安装:"
echo "  python3 $TARGET_DIR/scripts/temperature_cli.py --text '写一个Python函数'"