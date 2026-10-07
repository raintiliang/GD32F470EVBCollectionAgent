# Temperature Scenes 参考文档

详细的技术文档和使用指南。

## 架构概述

该技能采用模块化设计，核心功能集中在 `temperature_config.py` 中：

1. **场景配置** (`SCENE_TEMPERATURES`): 场景与temperature的映射
2. **别名系统** (`SCENE_ALIASES`): 支持多语言和简写
3. **场景检测** (`detect_scene_from_text`): 基于关键词的自动检测
4. **API接口** (`get_temperature`, `get_temperature_for_text`): 主要对外接口

## 核心模块详解

### temperature_config.py

#### SCENE_TEMPERATURES 字典

```python
SCENE_TEMPERATURES = {
    "code_generation": 0.0,          # 代码生成
    "math_problem": 0.0,             # 数学解题
    "data_extraction": 1.0,          # 数据抽取
    "data_analysis": 1.0,            # 数据分析
    "general_conversation": 1.3,     # 通用对话
    "translation": 1.3,              # 翻译
    "creative_writing": 1.5,         # 创意写作
    "poetry": 1.5,                   # 诗歌创作
}
```

#### 场景检测算法

`detect_scene_from_text()` 函数使用关键词匹配算法：

1. 将输入文本转换为小写
2. 遍历 `keyword_to_scene` 字典中的关键词
3. 统计每个场景的匹配次数
4. 返回匹配次数最多的场景
5. 平局时返回第一个匹配的场景

**关键词字典结构:**
```python
keyword_to_scene = {
    "代码": "code_generation",
    "编程": "code_generation",
    "python": "code_generation",
    # ... 更多关键词
}
```

#### 性能特点

- **时间复杂度**: O(n*m)，n为文本长度，m为关键词数量
- **空间复杂度**: O(1)
- **准确性**: 依赖于关键词覆盖度，适合明确场景的任务
- **可扩展性**: 容易添加新关键词和场景

## 集成指南

### 与OpenClaw集成

#### 方法1: 技能触发器

在OpenClaw技能系统中，可以通过以下方式触发：

```python
# 伪代码，实际取决于OpenClaw技能API
def on_message_received(message):
    scene = detect_scene_from_text(message.text)
    temperature = get_temperature(scene)
    
    # 设置OpenClaw的模型参数
    openclaw.set_model_parameter("temperature", temperature)
    
    # 继续正常处理
    return process_message(message)
```

#### 方法2: 预处理器中间件

作为对话流水线的一部分：

```
用户输入 → 场景检测 → temperature设置 → 模型调用 → 响应生成
```

#### 方法3: 用户命令

支持用户手动控制：

```
用户: /temperature scene 创意写作
系统: 已设置场景为"创意写作"，temperature=1.5
```

### 与其他AI系统集成

#### 与OpenAI API集成

```python
import temperature_config
import openai

def chat_with_scene(messages, scene=None):
    if scene is None:
        # 自动检测
        last_msg = next((m["content"] for m in reversed(messages) 
                        if m["role"] == "user"), "")
        scene = temperature_config.detect_scene_from_text(last_msg)
    
    temperature = temperature_config.get_temperature(scene)
    
    response = openai.ChatCompletion.create(
        model="gpt-4",
        messages=messages,
        temperature=temperature,
        max_tokens=2048
    )
    
    return response.choices[0].message.content
```

#### 与LangChain集成

```python
from langchain.llms import OpenAI
import temperature_config

class SmartTemperatureLLM(OpenAI):
    def _call(self, prompt, **kwargs):
        # 检测场景并设置temperature
        scene = temperature_config.detect_scene_from_text(prompt)
        temperature = temperature_config.get_temperature(scene)
        
        # 调用父类方法，使用合适的temperature
        return super()._call(prompt, temperature=temperature, **kwargs)
```

## 配置自定义

### 添加新场景

1. 在 `SCENE_TEMPERATURES` 中添加新条目
2. 在 `SCENE_ALIASES` 中添加别名（可选）
3. 在 `detect_scene_from_text` 函数的 `keyword_to_scene` 中添加关键词

```python
# 示例：添加"摘要生成"场景
SCENE_TEMPERATURES["summarization"] = 0.7

SCENE_ALIASES.update({
    "摘要": "summarization",
    "总结": "summarization",
    "summarize": "summarization",
})

# 在detect_scene_from_text函数中添加：
keyword_to_scene = {
    # ... 现有关键词
    "摘要": "summarization",
    "总结": "summarization",
    "summarize": "summarization",
}
```

### 调整temperature值

直接修改 `SCENE_TEMPERATURES` 中的值：

```python
# 提高创意写作的temperature
SCENE_TEMPERATURES["creative_writing"] = 1.8
SCENE_TEMPERATURES["poetry"] = 1.8

# 降低代码生成的temperature
SCENE_TEMPERATURES["code_generation"] = 0.1
SCENE_TEMPERATURES["math_problem"] = 0.1
```

## 测试策略

### 单元测试

建议的测试用例：

1. **场景映射测试**: 验证每个场景返回正确的temperature
2. **别名测试**: 验证别名正确映射到场景
3. **场景检测测试**: 
   - 明确关键词应正确匹配
   - 模糊文本应返回默认场景
   - 冲突关键词应选择匹配最多的场景
4. **边界测试**:
   - 空字符串输入
   - 非常长的文本
   - 特殊字符和Unicode

### 集成测试

1. **API集成测试**: 验证与实际AI API的集成
2. **性能测试**: 验证响应时间在可接受范围内
3. **准确性评估**: 使用标注数据集评估场景检测准确率

## 性能优化建议

### 当前实现

- 关键词匹配使用线性扫描，适合中小型关键词库
- 内存占用低，无需外部依赖

### 优化方案（如需）

1. **关键词索引**: 使用Trie树或Aho-Corasick算法加速匹配
2. **机器学习**: 使用小型分类模型提高检测准确性
3. **缓存**: 缓存常见文本的检测结果
4. **并发**: 支持批量处理多个文本

### 内存与CPU

- **典型使用**: <10MB内存，<10ms处理时间（1000字符文本）
- **极限情况**: 10,000个关键词，100KB文本 ≈ 50ms处理时间

## 故障排除

### 常见问题

1. **场景检测不准确**
   - 解决方案：添加更多关键词或调整关键词权重
   
2. **temperature设置无效**
   - 检查AI API是否接受temperature参数
   - 验证temperature值在有效范围内（通常0.0-2.0）

3. **性能问题**
   - 减少关键词数量
   - 实现关键词索引

4. **新场景不被识别**
   - 确保在`SCENE_TEMPERATURES`、`SCENE_ALIASES`和`keyword_to_scene`中都添加了配置

### 调试技巧

启用调试输出：

```python
import temperature_config

# 临时修改函数添加调试输出
original_detect = temperature_config.detect_scene_from_text

def debug_detect(text):
    scene = original_detect(text)
    print(f"调试: 文本='{text[:50]}...', 场景='{scene}'")
    return scene

temperature_config.detect_scene_from_text = debug_detect
```

## 扩展方向

### 短期改进

1. **上下文感知**: 考虑对话历史而不仅仅是最后一条消息
2. **用户偏好**: 允许用户自定义场景映射
3. **动态调整**: 根据响应质量动态调整temperature

### 长期愿景

1. **多模态支持**: 支持图像、音频等输入的场景检测
2. **个性化模型**: 基于用户历史学习最佳的temperature设置
3. **实时优化**: 根据模型输出质量实时调整temperature

## 版本兼容性

### Python版本
- 支持Python 3.6+
- 使用标准库，无外部依赖

### API兼容性
- 与OpenAI API兼容的temperature参数范围
- 支持DeepSeek、GPT等使用temperature参数的模型

### 技能系统兼容性
- 符合OpenClaw技能规范
- 可与其他技能组合使用

## 贡献指南

欢迎贡献改进：

1. **添加新场景**: 提交包含完整配置（映射、别名、关键词）的PR
2. **优化检测**: 改进算法或添加新功能
3. **文档完善**: 补充使用示例或技术细节
4. **测试覆盖**: 增加测试用例提高稳定性

## 许可证

本技能代码采用MIT许可证，可自由使用、修改和分发。