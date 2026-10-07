
import pandas as pd

file_path = 'Domestic_OS_RISCV_Survey_CN.xlsx'

# Data from Tencent Cloud Developer Article: Global Embedded OS Comparison
global_os_data = {
    'System Name': [
        'VxWorks', 
        'QNX', 
        'FreeRTOS', 
        'Zephyr', 
        'RT-Thread', 
        'μITRON', 
        'Embedded Linux', 
        'Windows IoT'
    ],
    'Origin/Vendor': [
        'Wind River (USA)', 
        'BlackBerry (Canada)', 
        'Amazon/Real Time Engineers (USA)', 
        'Linux Foundation (International)', 
        'RT-Thread (China)', 
        'TRON Forum (Japan)', 
        'Open Source Community', 
        'Microsoft (USA)'
    ],
    'Type': [
        'Hard RTOS', 
        'Hard RTOS (Microkernel)', 
        'RTOS', 
        'RTOS', 
        'RTOS', 
        'RTOS Standard', 
        'General OS (Non-hard real-time)', 
        'General OS'
    ],
    'Real-time Rating': [
        'Extremely High', 
        'Extremely High', 
        'High', 
        'High', 
        'High', 
        'High', 
        'Medium', 
        'Medium/Low'
    ],
    'Key Applications': [
        'Aerospace, Defense, Industrial Control', 
        'Automotive (Infotainment/Instrument), Medical', 
        'General IoT, Consumer Electronics', 
        'Resource-constrained IoT, Bluetooth', 
        'Consumer, Industrial, Automotive', 
        'Consumer Electronics (Japan Dominant)', 
        'Networking, Gateway, High-perf Embedded', 
        'HMI, Retail, Smart Signage'
    ],
    'Open Source Status': [
        'Commercial', 
        'Commercial', 
        'MIT License', 
        'Apache 2.0', 
        'Apache 2.0 / Commercial', 
        'Standard Specification', 
        'GPL', 
        'Commercial'
    ]
}

df_global = pd.DataFrame(global_os_data)

try:
    with pd.ExcelWriter(file_path, mode='a', engine='openpyxl', if_sheet_exists='replace') as writer:
        df_global.to_excel(writer, sheet_name='Global_Embedded_OS_Comp', index=False)
    print(f"Successfully updated {file_path} with 'Global_Embedded_OS_Comp' sheet.")
except Exception as e:
    print(f"Error updating Excel: {e}")
