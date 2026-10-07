#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Temperature场景配置命令行工具
"""

import sys
import argparse
import temperature_config

def main():
    parser = argparse.ArgumentParser(description="Temperature场景配置工具")
    parser.add_argument("--text", "-t", help="分析文本并检测场景")
    parser.add_argument("--scene", "-s", help="手动指定场景名称")
    parser.add_argument("--list", "-l", action="store_true", help="列出所有支持的场景")
    parser.add_argument("--test", action="store_true", help="运行测试用例")
    
    args = parser.parse_args()
    
    if args.list:
        print("支持的场景及其temperature值:")
        print("-" * 50)
        for scene, temp in temperature_config.SCENE_TEMPERATURES.items():
            print(f"{scene:20} -> {temp:.1f}")
        
        print("\n场景别名:")
        for alias, scene in temperature_config.SCENE_ALIASES.items():
            print(f"{alias:20} -> {scene}")
        return
    
    if args.test:
        print("运行测试...")
        temperature_config.main()  # 调用模块的内置测试
        return
    
    if args.scene:
        temperature = temperature_config.get_temperature(args.scene)
        print(f"场景: {args.scene}")
        print(f"temperature: {temperature}")
        return
    
    if args.text:
        scene = temperature_config.detect_scene_from_text(args.text)
        temperature = temperature_config.get_temperature(scene)
        print(f"输入文本: {args.text}")
        print(f"检测场景: {scene}")
        print(f"建议temperature: {temperature}")
        
        # 显示检测到的关键词（如果可能）
        if hasattr(temperature_config, 'keyword_to_scene'):
            print("\n关键词匹配:")
            text_lower = args.text.lower()
            for keyword, keyword_scene in temperature_config.keyword_to_scene.items():
                if keyword in text_lower and keyword_scene == scene:
                    print(f"  - {keyword}")
        return
    
    # 如果没有参数，进入交互模式
    print("Temperature场景配置工具")
    print("输入文本或场景名称，Ctrl+C退出")
    print("-" * 50)
    
    try:
        while True:
            user_input = input("\n> ").strip()
            if not user_input:
                continue
            
            # 检查是否是场景名称
            temp = temperature_config.get_temperature(user_input)
            if temp != 1.3 or user_input in temperature_config.SCENE_ALIASES:
                print(f"场景: {user_input} -> temperature: {temp}")
                continue
            
            # 否则作为文本处理
            scene = temperature_config.detect_scene_from_text(user_input)
            temperature = temperature_config.get_temperature(scene)
            print(f"文本: {user_input}")
            print(f"检测场景: {scene}")
            print(f"建议temperature: {temperature}")
            
    except KeyboardInterrupt:
        print("\n\n退出")
    except EOFError:
        print("\n\n退出")

if __name__ == "__main__":
    main()