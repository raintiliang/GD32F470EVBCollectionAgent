import os
import shutil
import hashlib
import json
import csv
import tempfile
import zipfile
import tarfile
from datetime import datetime
from PIL import Image
from PIL.ExifTags import TAGS

def get_file_hash(file_path):
    """计算文件的 MD5 哈希值，用于识别重复文件"""
    hasher = hashlib.md5()
    with open(file_path, 'rb') as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hasher.update(chunk)
    return hasher.hexdigest()

def get_exif_data(file_path):
    """读取图片的 EXIF 信息"""
    exif_data = {}
    try:
        with Image.open(file_path) as img:
            info = img._getexif()
            if info:
                for tag, value in info.items():
                    decoded = TAGS.get(tag, tag)
                    exif_data[decoded] = value
    except Exception:
        pass
    return exif_data

def get_photo_date(exif_data, file_path):
    """从 EXIF 或文件修改时间获取拍摄日期"""
    # 尝试从 EXIF 获取拍摄时间
    date_str = exif_data.get('DateTimeOriginal') or exif_data.get('DateTime')
    if date_str:
        try:
            # EXIF 格式通常是 'YYYY:MM:DD HH:MM:SS'
            return datetime.strptime(str(date_str)[:10], '%Y:%m:%d')
        except ValueError:
            pass
    
    # 如果没有 EXIF，使用文件的最后修改时间
    mtime = os.path.getmtime(file_path)
    return datetime.fromtimestamp(mtime)

def process_file(file_path, target_dir, action, seen_hashes, report):
    """处理单张图片的整理逻辑"""
    extensions = ('.jpg', '.jpeg', '.png', '.gif', '.bmp', '.tiff', '.webp')
    filename = os.path.basename(file_path)
    
    if not filename.lower().endswith(extensions):
        return

    report["total_processed"] += 1
    
    try:
        # 1. 计算哈希值查重
        file_hash = get_file_hash(file_path)
        if file_hash in seen_hashes:
            report["duplicates_skipped"] += 1
            report["details"].append({
                "file": filename,
                "status": "duplicate",
                "original": seen_hashes[file_hash]
            })
            return
        
        seen_hashes[file_hash] = file_path
        
        # 2. 获取日期和 EXIF
        exif = get_exif_data(file_path)
        date = get_photo_date(exif, file_path)
        
        # 3. 创建目标文件夹 (年/月/日)
        dest_folder = os.path.join(
            target_dir, 
            date.strftime('%Y'), 
            date.strftime('%m'), 
            date.strftime('%d')
        )
        if not os.path.exists(dest_folder):
            os.makedirs(dest_folder)
        
        dest_path = os.path.join(dest_folder, filename)
        
        # 处理同名但内容不同的情况
        counter = 1
        base, ext = os.path.splitext(filename)
        while os.path.exists(dest_path):
            dest_path = os.path.join(dest_folder, f"{base}_{counter}{ext}")
            counter += 1

        # 4. 执行移动或复制
        if action == 'move':
            shutil.move(file_path, dest_path)
        else:
            shutil.copy2(file_path, dest_path)
        
        report["moved_or_copied"] += 1
        report["details"].append({
            "file": filename,
            "status": "success",
            "destination": dest_path,
            "date": date.strftime('%Y-%m-%d')
        })

    except Exception as e:
        report["errors"] += 1
        report["details"].append({
            "file": filename,
            "status": "error",
            "message": str(e)
        })

def extract_and_process(archive_path, target_dir, action, seen_hashes, report):
    """解压并处理压缩包内容，采用流式解压/分批处理思想以节省空间"""
    print(f"正在处理压缩包: {archive_path}")
    
    with tempfile.TemporaryDirectory() as tmpdir:
        try:
            if archive_path.lower().endswith('.zip'):
                with zipfile.ZipFile(archive_path, 'r') as zip_ref:
                    zip_ref.extractall(tmpdir)
            elif archive_path.lower().endswith(('.tar.gz', '.tgz', '.tar')):
                with tarfile.open(archive_path, 'r:*') as tar_ref:
                    tar_ref.extractall(tmpdir)
            elif archive_path.lower().endswith('.rar'):
                try:
                    import rarfile
                    with rarfile.RarFile(archive_path) as rar_ref:
                        rar_ref.extractall(tmpdir)
                except ImportError:
                    print("警告: 处理 .rar 文件需要安装 rarfile 库 (pip install rarfile) 且系统中安装有 unrar。")
                    return
            
            # 递归处理解压后的目录
            for root, dirs, files in os.walk(tmpdir):
                for filename in files:
                    file_path = os.path.join(root, filename)
                    process_file(file_path, target_dir, action, seen_hashes, report)
        except Exception as e:
            print(f"解压或处理压缩包出错: {archive_path}, 错误: {e}")

def organize_photos(source_path, target_dir, action='copy'):
    """整理照片主逻辑，支持目录或单个压缩文件"""
    report = {
        "total_processed": 0,
        "moved_or_copied": 0,
        "duplicates_skipped": 0,
        "errors": 0,
        "details": []
    }
    
    seen_hashes = {} # hash -> original_path
    
    if not os.path.exists(target_dir):
        os.makedirs(target_dir)

    archive_exts = ('.zip', '.tar.gz', '.tgz', '.tar', '.rar')

    # 如果 source 是文件且是压缩包
    if os.path.isfile(source_path):
        if source_path.lower().endswith(archive_exts):
            extract_and_process(source_path, target_dir, action, seen_hashes, report)
        else:
            process_file(source_path, target_dir, action, seen_hashes, report)
    # 如果 source 是目录
    elif os.path.isdir(source_path):
        # 遍历目录，寻找图片和压缩包
        for root, dirs, files in os.walk(source_path):
            for filename in files:
                full_path = os.path.join(root, filename)
                if filename.lower().endswith(archive_exts):
                    # 流式处理思想：每个压缩包解压在一个独立的临时目录，处理完后自动删除
                    extract_and_process(full_path, target_dir, action, seen_hashes, report)
                else:
                    process_file(full_path, target_dir, action, seen_hashes, report)

    return report

def save_report(report, target_dir):
    """保存整理报告"""
    summary_path = os.path.join(target_dir, 'organize_report.json')
    with open(summary_path, 'w', encoding='utf-8') as f:
        json.dump(report, f, indent=4, ensure_ascii=False)
    
    csv_path = os.path.join(target_dir, 'organize_summary.csv')
    with open(csv_path, 'w', encoding='utf-8', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(["Metric", "Value"])
        writer.writerow(["Total Processed", report["total_processed"]])
        writer.writerow(["Moved/Copied", report["moved_or_copied"]])
        writer.writerow(["Duplicates Skipped", report["duplicates_skipped"]])
        writer.writerow(["Errors", report["errors"]])
    
    print(f"
整理完成！")
    print(f"处理总数: {report['total_processed']}")
    print(f"成功归档: {report['moved_or_copied']}")
    print(f"发现重复: {report['duplicates_skipped']}")
    print(f"报告已保存至: {target_dir}")

if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="照片自动整理工具 (支持压缩包)")
    parser.add_argument("source", help="源文件夹路径或压缩包路径")
    parser.add_argument("target", help="目标文件夹路径")
    parser.add_argument("--action", choices=['copy', 'move'], default='copy', help="操作类型: copy (默认) 或 move")
    
    args = parser.parse_args()
    
    res = organize_photos(args.source, args.target, args.action)
    save_report(res, args.target)
