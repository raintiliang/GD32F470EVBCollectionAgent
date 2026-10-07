# Temperature Scenes Skill

根据对话场景自动设置AI模型的temperature参数，优化响应质量。

## 描述

该技能提供了根据场景动态调整temperature参数的功能，适用于需要不同创意程度和准确性的对话场景。基于预先定义的场景映射，自动选择最合适的temperature值。

## 场景映射

| 场景 | Temperature | 说明 |
|------|------------|------|
| 代码生成 / 数学解题 | 0.0 | 需要精确、确定性的输出 |
| 数据抽取 / 数据分析 | 1.0 | 需要一定的灵活性但保持准确性 |
| 通用对话 | 1.3 | 平衡创意和准确性的日常对话 |
| 翻译 | 1.3 | 保持原意的灵活翻译 |
| 创意写作 / 诗歌创作 | 1.5 | 最大化的创意和多样性 |

## 何时使用

- 当用户请求代码生成、数学解题时（使用temperature=0.0）
- 当用户请求数据抽取、数据分析时（使用temperature=1.0）
- 当用户进行翻译任务时（使用temperature=1.3）
- 当用户请求创意写作、诗歌创作时（使用temperature=1.5）
- 当进行通用对话时（使用temperature=1.3）
- 需要手动设置特定场景的temperature时

## 工具

该技能提供以下工具：

### Python模块
- `scripts/temperature_config.py` - 核心配置和场景检测
- `scripts/smart_chat.py` - 智能聊天演示（可选）

### 命令行工具
- `scripts/temperature_cli.py` - 命令行界面

## 用法

### 在OpenClaw对话中使用

当技能被激活时，OpenClaw会在处理用户消息时自动检测场景并设置合适的temperature。

**自动检测示例：**
```
用户: 帮我写一个Python函数
技能: 检测到"代码生成"场景，设置temperature=0.0
```

**手动设置场景：**
```
用户: /scene 创意写作
技能: 已设置场景为"创意写作"，后续对话使用temperature=1.5
```

### 作为独立模块使用

可以在其他Python脚本中导入：

```python
import sys
sys.path.append('/home/rainti/.openclaw/workspace/skills/temperature-scenes/scripts')
import temperature_config

# 获取场景的temperature
temp = temperature_config.get_temperature("代码生成")  # 0.0

# 自动检测文本场景
scene = temperature_config.detect_scene_from_text("写一首诗")
temp = temperature_config.get_temperature(scene)  # 1.5
```

### 命令行使用

```bash
cd /home/rainti/.openclaw/workspace/skills/temperature-scenes/scripts
python3 temperature_cli.py --text "帮我解这个方程"
# 输出: 场景: math_problem, temperature: 0.0
```

## 配置

### 自定义场景映射

编辑 `scripts/temperature_config.py` 中的 `SCENE_TEMPERATURES` 字典：

```python
SCENE_TEMPERATURES = {
    "code_generation": 0.0,
    "math_problem": 0.0,
    # ... 添加或修改场景
}
```

### 添加场景别名

编辑 `SCENE_ALIASES` 字典：

```python
SCENE_ALIASES = {
    "编程": "code_generation",
    "计算": "math_problem",
    # ... 添加别名
}
```

## 集成到OpenClaw

### 方法1: 作为技能使用
技能被触发时，会修改OpenClaw的模型调用参数，自动设置合适的temperature。

### 方法2: 作为预处理器
在OpenClaw处理用户消息前，先通过场景检测确定合适的temperature，然后将temperature参数传递给模型API。

### 方法3: 手动控制
用户可以使用命令手动设置场景，技能会记住设置并应用于后续对话。

## 文件结构

```
temperature-scenes/
├── SKILL.md (本文件)
├── scripts/
│   ├── temperature_config.py (核心配置)
│   ├── temperature_cli.py (命令行工具)
│   └── smart_chat.py (演示脚本)
└── references/
    └── README.md (详细文档)
```

## 依赖

- Python 3.6+
- 无额外Python包依赖（基础功能）
- 可选: `openai` 库（用于smart_chat.py演示）

## 开发说明

该技能使用简单的关键词匹配进行场景检测。对于更复杂的检测，可以考虑：
1. 使用机器学习模型进行分类
2. 增加更多关键词和语境分析
3. 支持用户自定义检测规则

## 作者

由OpenClaw助手为rainti创建于2026-03-19。

## 更新日志

- 2026-03-19: 初始版本发布
- 场景映射基于用户需求：
  - 代码生成/数学解题: 0.0
  - 数据抽取/分析: 1.0
  - 通用对话: 1.3
  - 翻译: 1.3
  - 创意类写作/诗歌创作: 1.5