#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
OpenClaw集成示例 - 演示如何将temperature场景配置集成到OpenClaw中
"""

import os
import sys
import json
import temperature_config

# 添加技能目录到路径，以便导入temperature_config
skill_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(skill_dir, "scripts"))

def integrate_with_openclaw_config():
    """
    方法1: 修改OpenClaw配置，为不同场景设置不同的模型配置
    
    这需要手动编辑 ~/.openclaw/config.json
    """
    config_path = os.path.expanduser("~/.openclaw/config.json")
    
    if not os.path.exists(config_path):
        print(f"警告: OpenClaw配置文件不存在: {config_path}")
        return
    
    print("方法1: 修改OpenClaw配置")
    print("-" * 50)
    print("您可以在OpenClaw配置文件中为不同场景创建不同的模型配置。")
    print("例如，在config.json中添加:")
    
    config_example = {
        "agents": {
            "defaults": {
                "model": "custom-api-deepseek-com/deepseek-reasoner",
                # 默认temperature
            },
            "profiles": {
                "code_generation": {
                    "model": "custom-api-deepseek-com/deepseek-reasoner",
                    "temperature": 0.0
                },
                "creative_writing": {
                    "model": "custom-api-deepseek-com/deepseek-reasoner", 
                    "temperature": 1.5
                },
                # ... 更多场景配置
            }
        }
    }
    
    print(json.dumps(config_example, indent=2, ensure_ascii=False))
    print()
    print("然后，您可以根据场景切换到不同的配置。")
    print()

def create_openclaw_skill_wrapper():
    """
    方法2: 创建OpenClaw技能包装器
    
    创建一个技能，在运行时动态设置temperature
    """
    print("方法2: 创建OpenClaw技能包装器")
    print("-" * 50)
    print("创建一个技能，在处理消息时自动检测场景并设置temperature。")
    print()
    print("技能代码示例 (伪代码):")
    
    skill_code = '''
# 在OpenClaw技能中
def on_message(context):
    user_message = context.message.text
    scene = detect_scene_from_text(user_message)
    temperature = get_temperature(scene)
    
    # 设置模型参数
    context.model_parameters["temperature"] = temperature
    
    # 继续正常处理
    return context.next()
'''
    
    print(skill_code)
    print()
    print("具体实现取决于OpenClaw的技能API。")
    print()

def create_preprocessor_script():
    """
    方法3: 创建预处理器脚本
    
    在处理用户输入前，先检测场景并设置环境变量
    """
    print("方法3: 创建预处理器脚本")
    print("-" * 50)
    
    script_content = '''#!/bin/bash
# OpenClaw预处理器脚本
# 在OpenClaw处理消息前运行，设置合适的temperature

# 获取用户输入（假设通过管道或环境变量）
USER_INPUT="$1"
if [ -z "$USER_INPUT" ]; then
    # 尝试从环境变量获取
    USER_INPUT="$OPENCLAW_USER_MESSAGE"
fi

# 使用Python检测场景
SCENE=$(python3 -c "
import sys
sys.path.append('/home/rainti/.openclaw/workspace/skills/temperature-scenes/scripts')
import temperature_config

text = '''$USER_INPUT'''
scene = temperature_config.detect_scene_from_text(text)
print(scene)
")

# 根据场景设置temperature
case $SCENE in
    code_generation|math_problem)
        TEMPERATURE=0.0
        ;;
    data_extraction|data_analysis)
        TEMPERATURE=1.0
        ;;
    translation|general_conversation)
        TEMPERATURE=1.3
        ;;
    creative_writing|poetry)
        TEMPERATURE=1.5
        ;;
    *)
        TEMPERATURE=1.3
        ;;
esac

# 导出环境变量供OpenClaw使用
export OPENCLAW_TEMPERATURE=$TEMPERATURE
export OPENCLAW_DETECTED_SCENE=$SCENE

# 继续原流程
exec "$@"
'''
    
    print("创建预处理器脚本，设置环境变量:")
    print(script_content)
    
    script_path = "/tmp/openclaw_temperature_preprocessor.sh"
    with open(script_path, "w") as f:
        f.write(script_content)
    
    print(f"\n示例脚本已保存到: {script_path}")
    print("使用方法:")
    print("  OPENCLAW_USER_MESSAGE='用户输入' /tmp/openclaw_temperature_preprocessor.sh openclaw_command")
    print()

def create_standalone_agent():
    """
    方法4: 创建独立的智能代理
    
    创建一个独立的Python脚本，使用合适的temperature调用DeepSeek API
    """
    print("方法4: 创建独立的智能代理")
    print("-" * 50)
    
    agent_code = '''#!/usr/bin/env python3
"""
智能代理 - 集成temperature场景检测
"""

import os
import sys
import temperature_config
from openai import OpenAI

class SmartTemperatureAgent:
    def __init__(self, api_key=None, base_url="https://api.deepseek.com"):
        self.api_key = api_key or os.getenv("DEEPSEEK_API_KEY")
        if not self.api_key:
            raise ValueError("需要DEEPSEEK_API_KEY环境变量或参数")
        
        self.client = OpenAI(
            api_key=self.api_key,
            base_url=base_url
        )
    
    def chat(self, message, scene=None):
        """智能聊天，自动设置temperature"""
        # 检测场景
        if scene is None:
            scene = temperature_config.detect_scene_from_text(message)
        
        temperature = temperature_config.get_temperature(scene)
        
        print(f"[智能代理] 场景: {scene}, temperature: {temperature}")
        
        # 调用API
        response = self.client.chat.completions.create(
            model="deepseek-chat",
            messages=[{"role": "user", "content": message}],
            temperature=temperature,
            max_tokens=2048
        )
        
        return response.choices[0].message.content
    
    def interactive_chat(self):
        """交互式聊天"""
        print("智能代理 (自动temperature设置)")
        print("输入 'quit' 退出, 'scene 场景名' 手动设置")
        print("-" * 50)
        
        while True:
            try:
                user_input = input("你: ").strip()
                if not user_input:
                    continue
                
                if user_input.lower() == 'quit':
                    break
                
                if user_input.lower().startswith('scene '):
                    scene = user_input[6:].strip()
                    print(f"手动设置场景: {scene}")
                    continue
                
                response = self.chat(user_input)
                print(f"AI: {response}")
                print()
                
            except KeyboardInterrupt:
                print("\n退出")
                break
            except Exception as e:
                print(f"错误: {e}")

if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="智能温度代理")
    parser.add_argument("--message", "-m", help="直接处理的消息")
    parser.add_argument("--scene", "-s", help="手动指定场景")
    parser.add_argument("--interactive", "-i", action="store_true", help="交互模式")
    
    args = parser.parse_args()
    
    agent = SmartTemperatureAgent()
    
    if args.message:
        response = agent.chat(args.message, args.scene)
        print(response)
    elif args.interactive:
        agent.interactive_chat()
    else:
        print("请提供消息或使用交互模式")
        print("示例:")
        print("  python3 smart_agent.py -m '写一个Python函数'")
        print("  python3 smart_agent.py -i")
'''
    
    print("创建独立的智能代理，直接调用DeepSeek API:")
    print(agent_code)
    
    agent_path = "/tmp/smart_temperature_agent.py"
    with open(agent_path, "w") as f:
        f.write(agent_code)
    
    print(f"\n示例代码已保存到: {agent_path}")
    print("依赖: pip install openai")
    print()

def main():
    """主函数"""
    print("OpenClaw Temperature场景配置集成指南")
    print("=" * 70)
    print()
    print("提供了多种集成方案，请根据需求选择:")
    print()
    
    integrate_with_openclaw_config()
    create_openclaw_skill_wrapper()
    create_preprocessor_script()
    create_standalone_agent()
    
    print("=" * 70)
    print("总结:")
    print("1. 方法1（配置修改）: 最直接，但需要了解OpenClaw配置结构")
    print("2. 方法2（技能包装器）: 最符合OpenClaw架构，但需要技能开发知识")  
    print("3. 方法3（预处理器）: 简单，通过环境变量传递参数")
    print("4. 方法4（独立代理）: 最灵活，不依赖OpenClaw，直接调用API")
    print()
    print("推荐方案:")
    print("- 快速集成: 使用方法3（预处理器）或方法4（独立代理）")
    print("- 生产环境: 使用方法2（技能包装器），如果OpenClaw支持")
    print("- 完全控制: 使用方法4（独立代理）")
    print()
    print("注意: 这些是示例方案，实际集成可能需要调整以适应您的OpenClaw版本。")

if __name__ == "__main__":
    main()