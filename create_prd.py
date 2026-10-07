import os
from docx import Document
from docx.shared import Pt
from docx.enum.text import WD_ALIGN_PARAGRAPH

def create_prd_docx():
    doc = Document()
    
    # Title
    title = doc.add_heading('智能材料测试系统 - 软件需求说明书 (PRD)', 0)
    title.alignment = WD_ALIGN_PARAGRAPH.CENTER
    
    # 1. Project Info
    doc.add_heading('1. 文档信息', level=1)
    p = doc.add_paragraph()
    p.add_run('项目名称：').bold = True
    p.add_run('智能材料测试系统 (Material Tester System)\n')
    p.add_run('核心主控：').bold = True
    p.add_run('GD32F470 系列\n')
    p.add_run('版本：').bold = True
    p.add_run('V1.0\n')
    p.add_run('日期：').bold = True
    p.add_run('2026-09-15')

    # 2. Functional Requirements
    doc.add_heading('2. 系统功能需求', level=1)
    
    doc.add_heading('2.1 设备端固件功能 (Embedded Firmware)', level=2)
    doc.add_paragraph("• 实时控制：", style='List Bullet').runs[0].bold = True
    doc.add_paragraph("实现 CO2 浓度和湿度的 PID 闭环控制。通过电磁阀、加湿机和除湿机模组维持测试箱内的特定环境。", style='List Bullet')
    
    doc.add_paragraph("• 数据采集与显示：", style='List Bullet').runs[0].bold = True
    doc.add_paragraph("每秒采集 T/H/CO2 数据。使用 LVGL 在 5 寸屏上实时刷新数值、反应时长及当前状态。", style='List Bullet')
    
    doc.add_paragraph("• 本地存储管理：", style='List Bullet').runs[0].bold = True
    doc.add_paragraph("定时将环境数据以 CSV 格式存入 TF 卡。支持按日期命名文件，确保数据可离线追溯。", style='List Bullet')
    
    doc.add_paragraph("• 远程通讯：", style='List Bullet').runs[0].bold = True
    doc.add_paragraph("通过 4G 模块实现 MQTT 联网。支持断线重连及心跳检测。支持远程指令（开始/暂停/修改参数）。", style='List Bullet')

    doc.add_heading('2.2 网页客户端功能 (Web/Cloud)', level=2)
    doc.add_paragraph("• 登录鉴权：支持账号密码登录，区分用户权限。", style='List Bullet')
    doc.add_paragraph("• 实时监控面板：显示当前设备在线状态及实时采集曲线。", style='List Bullet')
    doc.add_paragraph("• 历史报表：支持查询过去任意时段的温湿度和浓度曲线，支持数据导出为 Excel。", style='List Bullet')
    doc.add_paragraph("• 远程配置：可在线修改设备运行参数（目标值、报警阈值）。", style='List Bullet')

    # 3. Pinout Plan
    doc.add_heading('3. GD32F470 引脚分配方案', level=1)
    table = doc.add_table(rows=1, cols=3)
    table.style = 'Table Grid'
    hdr_cells = table.rows[0].cells
    hdr_cells[0].text = '功能模块'
    hdr_cells[1].text = '接口/引脚'
    hdr_cells[2].text = '备注'
    
    pins = [
        ['5寸 RGB 屏', 'LTDC 控制器引脚', 'Port A, B, C, G (具体视屏幕手册)'],
        ['外部显存', 'EXMC (SDRAM)', '用于 800x480 分辨率缓冲'],
        ['CO2 传感器', 'UART1 (PA9/PA10)', '走 UART 串口'],
        ['4G 模块', 'UART2 (PD5/PD6)', '走 AT 指令'],
        ['温湿度传感器', 'I2C1 (PB6/PB7)', '走 I2C 通讯'],
        ['除湿机模组', 'UART3 + DE 引脚', '走 RS485 总线'],
        ['TF 存储卡', 'SDIO (PC8-PC12/PD2)', '高倍速存储'],
        ['电磁阀/加湿器', 'GPIO (PE2/PE3)', '继电器驱动'],
        ['交互按键', 'GPIO', '支持外部中断']
    ]
    
    for mod, pin, note in pins:
        row = table.add_row().cells
        row[0].text = mod
        row[1].text = pin
        row[2].text = note

    # 4. Non-functional Requirements
    doc.add_heading('4. 非功能需求', level=1)
    doc.add_paragraph("• 稳定性：系统需支持 7x24 小时连续不间断运行，具备软件看门狗监控。")
    doc.add_paragraph("• 安全性：传感器故障或浓度超限需触发本地和远程双向告警，并自动进入安全停机状态。")
    doc.add_paragraph("• 易用性：本地 5 寸屏界面需简洁直观，网页端需适配手机浏览器查看。")

    file_path = '智能材料测试系统_软件需求说明书(PRD).docx'
    doc.save(file_path)
    print(file_path)

if __name__ == "__main__":
    create_prd_docx()
