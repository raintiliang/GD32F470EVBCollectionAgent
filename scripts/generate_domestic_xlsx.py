import pandas as pd

data = [
    {
        "OS Platform": "RT-Thread / Smart",
        "Domestic Vendor": "Shanghai Realsilicon (睿赛德科技)",
        "Type": "RTOS / Hybrid Microkernel",
        "Primary Deployment": "Industrial Control, Automotive Cockpit/MCU, IoT, New Energy",
        "Key Chinese Chips Supported": "T-Head (E902/E906/C906), StarFive (JH7110), Allwinner (D1/T113), Rockchip (RK3568)",
        "RISC-V Support Status": "Excellent. Deep collaboration with T-Head and StarFive. Mature BSPs for domestic RISC-V chips.",
        "Maturity & Deployment": "Mass Deployment (Billions of devices). Used in BYD, SAIC, Li Auto.",
        "Safety Cert (FuSa)": "ISO 26262 ASIL-D, IEC 61508 SIL3",
        "License": "Apache 2.0 (Open Source)",
        "Reference Link": "https://www.rt-thread.org/"
    },
    {
        "OS Platform": "SylixOS",
        "Domestic Vendor": "Acoinfo (翼辉信息)",
        "Type": "Hard Real-time OS (Large)",
        "Primary Deployment": "Industrial Automation, Power Grid, Aerospace, Rail Transit, Defense",
        "Key Chinese Chips Supported": "Loongson (龙芯), FeiTeng (飞腾), T-Head (玄铁), Zhaoxin (兆芯), Rockchip",
        "RISC-V Support Status": "Excellent. Fully compatible with RISC-V 32/64. Active in domestic industrial PLC/controller ports.",
        "Maturity & Deployment": "High. Key supplier for State Grid (国家电网) and aerospace missions.",
        "Safety Cert (FuSa)": "IEC 61508, EN 50128, GJB 7714",
        "License": "Proprietary / Shared Source",
        "Reference Link": "https://www.acoinfo.com/"
    },
    {
        "OS Platform": "OpenHarmony",
        "Domestic Vendor": "OpenAtom Foundation / Huawei",
        "Type": "Distributed OS (L0-L5)",
        "Primary Deployment": "IoT, Smart Home, Industrial Handhelds, Automotive Cockpit",
        "Key Chinese Chips Supported": "T-Head (XuanTie series), Rockchip (RK3568/RK3588), HiSilicon (Starlight)",
        "RISC-V Support Status": "Growing. Dedicated RISC-V SIG. Success on T-Head Dashan platforms.",
        "Maturity & Deployment": "Rapid Growth. Expanding from consumer to industrial (MineHarmony).",
        "Safety Cert (FuSa)": "IEC 61508 (Targeted/Ongoing for industrial versions)",
        "License": "Apache 2.0 / Mulan (Open Source)",
        "Reference Link": "https://www.openharmony.cn/"
    },
    {
        "OS Platform": "Kylin Industrial OS",
        "Domestic Vendor": "Kylinsoft (麒麟软件)",
        "Type": "Linux-style / Real-time Enhanced",
        "Primary Deployment": "Industrial Desktop, Energy, Government, Automotive Infotainment",
        "Key Chinese Chips Supported": "FeiTeng, Loongson, Kunpeng, Sunway, T-Head",
        "RISC-V Support Status": "Active. Support for RISC-V via openKylin community and industrial editions.",
        "Maturity & Deployment": "Mass Deployment in government/infrastructure. Leading domestic server OS.",
        "Safety Cert (FuSa)": "EAL4+ (Security), Functional safety for industrial variants.",
        "License": "GPL / Proprietary",
        "Reference Link": "https://www.kylinos.cn/"
    },
    {
        "OS Platform": "iSoft (普华)",
        "Domestic Vendor": "iSoft Infrastructure Software (普华基础软件)",
        "Type": "AUTOSAR / Linux / RTOS",
        "Primary Deployment": "Automotive Electronic Control Units (ECU), ADAS, Powertrain",
        "Key Chinese Chips Supported": "GigaDevice (GD32), Chipways, Xinwang (芯旺微)",
        "RISC-V Support Status": "Existing. Focus on RISC-V based automotive MCUs and AUTOSAR stacks.",
        "Maturity & Deployment": "Significant in domestic automotive supply chain. National team background.",
        "Safety Cert (FuSa)": "ISO 26262 ASIL-D",
        "License": "Proprietary",
        "Reference Link": "https://www.isoft.com.cn/"
    },
    {
        "OS Platform": "UniOS (统信)",
        "Domestic Vendor": "UnionTech (统信软件)",
        "Type": "Linux-style Application OS",
        "Primary Deployment": "Industrial Human-Machine Interface (HMI), Edge Computing, Government",
        "Key Chinese Chips Supported": "Loongson, FeiTeng, Rockchip, T-Head",
        "RISC-V Support Status": "Active. deepin-riscv active porting; industrial versions targeting RV64.",
        "Maturity & Deployment": "Production. High adoption in domestic PC/Desktop and edge servers.",
        "Safety Cert (FuSa)": "EAL4+ (Security)",
        "License": "GPL / Proprietary",
        "Reference Link": "https://www.uniontech.com/"
    },
    {
        "OS Platform": "NewStart (中兴新支点)",
        "Domestic Vendor": "ZTE NewStart (中兴新支点)",
        "Type": "Linux / RTOS / Embedded OS",
        "Primary Deployment": "Telecommunications, Rail Transit, Automotive (Cockpit/ADAS)",
        "Key Chinese Chips Supported": "ZTE Microelectronics, FeiTeng, Rockchip",
        "RISC-V Support Status": "Existing. Integration with domestic telecom-grade RISC-V hardware.",
        "Maturity & Deployment": "High. Deployed in high-speed rail systems and ZTE infrastructure.",
        "Safety Cert (FuSa)": "ISO 26262, IEC 61508",
        "License": "Proprietary / GPL",
        "Reference Link": "https://www.gd-linux.com/"
    }
]

df = pd.DataFrame(data)

# Create a Excel writer object
filename = 'Domestic_OS_RISCV_Survey_CN.xlsx'
with pd.ExcelWriter(filename, engine='openpyxl') as writer:
    df.to_excel(writer, index=False, sheet_name='Domestic OS Survey')
    
    # Auto-adjust columns width
    worksheet = writer.sheets['Domestic OS Survey']
    for idx, col in enumerate(df.columns):
        series = df[col]
        max_len = max((
            series.astype(str).map(len).max(),
            len(str(series.name))
            )) + 2
        worksheet.column_dimensions[chr(65 + idx)].width = min(max_len, 60) # Cap at 60

print(f"Excel file '{filename}' created successfully.")
