#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Temperature配置模块
根据对话场景设置合适的temperature参数
"""

SCENE_TEMPERATURES = {
    # 场景: temperature值
    "code_generation": 0.0,          # 代码生成
    "math_problem": 0.0,             # 数学解题
    "data_extraction": 1.0,          # 数据抽取
    "data_analysis": 1.0,            # 数据分析
    "general_conversation": 1.3,     # 通用对话
    "translation": 1.3,              # 翻译
    "creative_writing": 1.5,         # 创意写作
    "poetry": 1.5,                   # 诗歌创作
}

# 场景别名映射（方便用户使用不同的名称）
SCENE_ALIASES = {
    "代码生成": "code_generation",
    "数学解题": "math_problem",
    "数据抽取": "data_extraction",
    "数据分析": "data_analysis",
    "通用对话": "general_conversation",
    "翻译": "translation",
    "创意写作": "creative_writing",
    "创意类写作": "creative_writing",
    "诗歌创作": "poetry",
    "code": "code_generation",
    "math": "math_problem",
    "data": "data_analysis",
    "chat": "general_conversation",
    "translate": "translation",
    "creative": "creative_writing",
}

def get_temperature(scene):
    """
    根据场景获取temperature值
    
    Args:
        scene (str): 场景名称（支持中文别名和英文）
        
    Returns:
        float: temperature值，如果场景未找到则返回通用对话的默认值1.3
        
    Examples:
        >>> get_temperature("代码生成")
        0.0
        >>> get_temperature("creative_writing")
        1.5
    """
    # 先尝试直接匹配
    if scene in SCENE_TEMPERATURES:
        return SCENE_TEMPERATURES[scene]
    
    # 尝试通过别名匹配
    normalized_scene = SCENE_ALIASES.get(scene)
    if normalized_scene and normalized_scene in SCENE_TEMPERATURES:
        return SCENE_TEMPERATURES[normalized_scene]
    
    # 未找到场景，返回通用对话的默认值
    return SCENE_TEMPERATURES["general_conversation"]

def detect_scene_from_text(text, default="general_conversation"):
    """
    从文本内容自动检测场景（基于关键词匹配）
    
    Args:
        text (str): 输入文本
        default (str): 默认场景
        
    Returns:
        str: 检测到的场景名称
        
    Note:
        这是一个简单的基于关键词的检测，对于复杂场景可能需要更高级的NLP模型
    """
    text_lower = text.lower()
    
    # 关键词映射到场景
    keyword_to_scene = {
        # 代码生成相关
        "代码": "code_generation",
        "编程": "code_generation",
        "function": "code_generation",
        "def ": "code_generation",
        "class ": "code_generation",
        "import ": "code_generation",
        "python": "code_generation",
        "javascript": "code_generation",
        "java": "code_generation",
        "c++": "code_generation",
        "html": "code_generation",
        "css": "code_generation",
        
        # 数学解题相关
        "数学": "math_problem",
        "计算": "math_problem",
        "方程": "math_problem",
        "公式": "math_problem",
        "solve": "math_problem",
        "calculate": "math_problem",
        "proof": "math_problem",
        
        # 数据抽取/分析相关
        "数据": "data_analysis",
        "分析": "data_analysis",
        "统计": "data_analysis",
        "图表": "data_analysis",
        "excel": "data_analysis",
        "csv": "data_analysis",
        "json": "data_analysis",
        "提取": "data_extraction",
        "抽取": "data_extraction",
        
        # 翻译相关
        "翻译": "translation",
        "translate": "translation",
        "英文": "translation",
        "中文": "translation",
        "日语": "translation",
        
        # 创意写作相关
        "诗歌": "poetry",
        "诗": "poetry",
        "创作": "creative_writing",
        "故事": "creative_writing",
        "小说": "creative_writing",
        "散文": "creative_writing",
        "创意": "creative_writing",
        "想象": "creative_writing",
    }
    
    # 计算关键词匹配次数
    scene_scores = {}
    for keyword, scene in keyword_to_scene.items():
        if keyword in text_lower:
            scene_scores[scene] = scene_scores.get(scene, 0) + 1
    
    if scene_scores:
        # 返回匹配次数最多的场景
        return max(scene_scores.items(), key=lambda x: x[1])[0]
    
    return default

def get_temperature_for_text(text, use_detection=True):
    """
    根据文本内容获取合适的temperature值
    
    Args:
        text (str): 输入文本
        use_detection (bool): 是否自动检测场景
        
    Returns:
        float: 合适的temperature值
    """
    if use_detection:
        scene = detect_scene_from_text(text)
    else:
        scene = "general_conversation"
    
    return get_temperature(scene)

if __name__ == "__main__":
    # 测试代码
    test_cases = [
        ("代码生成", 0.0),
        ("数学解题", 0.0),
        ("数据抽取", 1.0),
        ("数据分析", 1.0),
        ("通用对话", 1.3),
        ("翻译", 1.3),
        ("创意写作", 1.5),
        ("诗歌创作", 1.5),
        ("unknown_scene", 1.3),  # 默认值
    ]
    
    print("场景温度配置测试:")
    print("-" * 40)
    for scene, expected in test_cases:
        result = get_temperature(scene)
        status = "✓" if result == expected else "✗"
        print(f"{status} {scene:10} -> {result:.1f} (期望: {expected:.1f})")
    
    print("\n文本场景检测测试:")
    print("-" * 40)
    test_texts = [
        ("帮我写一个Python函数", "code_generation"),
        ("解这个方程: x^2 + 2x + 1 = 0", "math_problem"),
        ("从这段文本中提取数据", "data_extraction"),
        ("把这句话翻译成英文", "translation"),
        ("写一首关于春天的诗", "poetry"),
        ("今天天气怎么样", "general_conversation"),
    ]
    
    for text, expected_scene in test_texts:
        detected = detect_scene_from_text(text)
        temp = get_temperature(detected)
        print(f"文本: {text[:20]}...")
        print(f"  检测场景: {detected} (期望: {expected_scene})")
        print(f"  temperature: {temp:.1f}")
        print()