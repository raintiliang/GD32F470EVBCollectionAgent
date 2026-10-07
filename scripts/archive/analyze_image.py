#!/usr/bin/env python3
"""
简单的图片分析脚本
用于检测图片的基本特征
"""

import sys
from PIL import Image
import numpy as np

def analyze_image(image_path):
    try:
        img = Image.open(image_path)
        print(f"图片: {image_path}")
        print(f"尺寸: {img.size} (宽x高)")
        print(f"模式: {img.mode}")
        print(f"格式: {img.format}")
        
        # 转换为RGB数组
        if img.mode != 'RGB':
            img = img.convert('RGB')
        
        # 计算主要颜色
        np_img = np.array(img)
        # 简化分析：检查图像是否有很多文字（高对比度）
        # 计算亮度变化
        gray = np.mean(np_img, axis=2)
        contrast = np.std(gray)
        print(f"对比度（标准差）: {contrast:.2f}")
        
        # 检查是否有明显的边缘（可能表示文字或图表）
        if contrast > 30:
            print("提示: 图片可能有文字或高对比度内容")
        else:
            print("提示: 图片可能为普通照片")
            
        # 检查颜色分布
        unique_colors = len(np.unique(np_img.reshape(-1, 3), axis=0))
        print(f"大致颜色数量: {unique_colors}")
        
        return True
        
    except Exception as e:
        print(f"分析失败: {e}")
        return False

if __name__ == "__main__":
    if len(sys.argv) > 1:
        analyze_image(sys.argv[1])
    else:
        print("用法: python3 analyze_image.py <图片路径>")