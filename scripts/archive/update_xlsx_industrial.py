
import pandas as pd

file_path = 'Domestic_OS_RISCV_Survey_CN.xlsx'

# Data from Xinda Securities Industrial Control Report (2023-04-20)
industrial_control_data = {
    'Sector/Layer': [
        'Industrial OS (工控操作系统)', 
        'Control Layer (控制层)', 
        'Execution Layer (驱动层)', 
        'Sensing Layer (感知层)', 
        'Software Platform'
    ],
    'Key Technologies/Hardware': [
        'Real-time Kernel, Micro-kernel, RTOS',
        'PLC (Programmable Logic Controller), DCS',
        'Servo Drive, Inverter (变频器)',
        'Sensors, Industrial Cameras, HMI',
        'SCADA, MES, ERP integration'
    ],
    'Domestic Leaders (Report Focus)': [
        '翼辉信息 (SylixOS), 中兴通讯, RT-Thread',
        '汇川技术 (Inovance), 中控技术 (SUPCON), 信捷电气',
        '汇川技术, 禾川科技, 雷赛智能',
        '海康机器人, 华睿科技, 柯力传感',
        '中控技术, 工业富联, 树根互联'
    ],
    'Market Trends': [
        'High real-time requirement; transition from generic Linux to specialized RTOS.',
        'Domestic replacement accelerating in small/medium PLC; high-end still dominated by Siemens/Mitsubishi.',
        'High growth in lithium battery/photovoltaic equipment drive demand.',
        'Integration of "AI + Vision" for industrial inspection.',
        'Convergence of OT (Operational Tech) and IT (Info Tech).'
    ]
}

df_industrial = pd.DataFrame(industrial_control_data)

try:
    with pd.ExcelWriter(file_path, mode='a', engine='openpyxl', if_sheet_exists='replace') as writer:
        df_industrial.to_excel(writer, sheet_name='Industrial_Control_2023', index=False)
    print(f"Successfully updated {file_path} with 'Industrial_Control_2023' sheet.")
except Exception as e:
    print(f"Error updating Excel: {e}")
