
import pandas as pd

file_path = 'Domestic_OS_RISCV_Survey_CN.xlsx'

# Data from HuaAn Securities Report (2022-11-18)
auto_os_data = {
    'Layer/Category': [
        'Cabin OS (座舱操作系统)', 
        'Real-time OS (实时操作系统)', 
        'Middleware (中间件)', 
        'Hypervisor (虚拟化)', 
        'Industry Standard'
    ],
    'Key Domestic Players': [
        '斑马智行 (AliOS), 华为 (HarmonyOS), 百度 (DuerOS)',
        '中兴通讯 (ZTE), RT-Thread, 翼辉信息 (SylixOS)',
        '普华基础软件 (iSoft), 东软睿驰 (NeuSAR), 经纬恒润',
        '华为, 中兴, 黑芝麻智能',
        'AUTOSAR (Classic & Adaptive)'
    ],
    'Technical Highlights': [
        'Rich ecosystem, Android compatibility, Focus on UI/UX and interaction.',
        'High safety (ISO 26262), low latency, used in Chassis/Power/BMS.',
        'SOA architecture support, decoupling hardware and software.',
        'Enables multi-OS coexistence (e.g., QNX + Linux) on a single SoC.',
        'Standardization for ECUs; Domestic players making breakthroughs in CP/AP.'
    ],
    'Market Context': [
        'Driven by domestic EV demand.',
        'High entry barrier due to safety certifications.',
        'Rapid growth as SDV evolves.',
        'High concentration by QNX (Foreign); Domestic replacement starting.',
        'Essential for tiered supply chains.'
    ]
}

df_auto = pd.DataFrame(auto_os_data)

try:
    with pd.ExcelWriter(file_path, mode='a', engine='openpyxl', if_sheet_exists='replace') as writer:
        df_auto.to_excel(writer, sheet_name='Automotive_OS_2022', index=False)
    print(f"Successfully updated {file_path} with 'Automotive_OS_2022' sheet.")
except Exception as e:
    print(f"Error updating Excel: {e}")
