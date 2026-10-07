import pandas as pd

data = [
    {
        "OS Platform": "RT-Thread / Smart",
        "Type": "RTOS / Hybrid Microkernel",
        "Primary Deployment": "Industrial, Auto (Cockpit/MCU), IoT",
        "Maturity": "Mass Deployment",
        "License": "Apache 2.0",
        "Safety Cert (FuSa)": "ISO 26262 ASIL-D, IEC 61508 SIL3",
        "RISC-V Support Status": "Excellent: Native support for RV32/RV64, various BSPs (SiFive, Allwinner D1).",
        "Community & Backing": "High activity (GitHub/Gitee); RT-Thread Foundation.",
        "Key Vendors/OEMs": "BYD, NIO, SAIC, State Grid.",
        "Reference Link": "https://www.rt-thread.org/"
    },
    {
        "OS Platform": "Zephyr Project",
        "Type": "RTOS",
        "Primary Deployment": "IoT, Industrial, Wearables",
        "Maturity": "Production (LTS)",
        "License": "Apache 2.0",
        "Safety Cert (FuSa)": "Ongoing (Functional Safety WG); targeted SIL3",
        "RISC-V Support Status": "Strong: Extensive RISC-V BSPs; core support for privileged/unprivileged modes.",
        "Community & Backing": "Linux Foundation; 1500+ contributors.",
        "Key Vendors/OEMs": "Antmicro, Google, Intel, NXP.",
        "Reference Link": "https://docs.zephyrproject.org/latest/boards/riscv/index.html"
    },
    {
        "OS Platform": "FreeRTOS",
        "Type": "RTOS",
        "Primary Deployment": "IoT, Automotive MCUs",
        "Maturity": "Mass Deployment",
        "License": "MIT",
        "Safety Cert (FuSa)": "Via SafeRTOS (Proprietary)",
        "RISC-V Support Status": "Native: Standard ports for RISC-V IMC; widely used in RV microcontrollers.",
        "Community & Backing": "Amazon AWS; massive global ecosystem.",
        "Key Vendors/OEMs": "Espressif (ESP32-C series), SiFive.",
        "Reference Link": "https://www.freertos.org/Using-FreeRTOS-on-RISC-V.html"
    },
    {
        "OS Platform": "QNX (BlackBerry)",
        "Type": "Microkernel RTOS",
        "Primary Deployment": "Automotive (ADAS/Cockpit), Medical",
        "Maturity": "Mass Deployment",
        "License": "Proprietary",
        "Safety Cert (FuSa)": "ISO 26262 ASIL-D, IEC 61508 SIL3",
        "RISC-V Support Status": "Ongoing: Announced support for RISC-V; focus on software-defined vehicles.",
        "Community & Backing": "Corporate; closed source.",
        "Key Vendors/OEMs": "NIO, Li Auto, BMW, Ford, VW.",
        "Reference Link": "https://blogs.blackberry.com/en/2023/11/blackberry-qnx-embraces-risc-v"
    },
    {
        "OS Platform": "VxWorks",
        "Type": "RTOS",
        "Primary Deployment": "Aerospace, Defense, Industrial",
        "Maturity": "Mass Deployment",
        "License": "Proprietary",
        "Safety Cert (FuSa)": "DO-178C, IEC 61508, ISO 26262",
        "RISC-V Support Status": "Existing: First commercial RTOS to support RISC-V (32/64-bit).",
        "Community & Backing": "Wind River; corporate ecosystem.",
        "Key Vendors/OEMs": "ABB, Bosch, Raytheon.",
        "Reference Link": "https://www.windriver.com/news/press-releases/wind-river-announces-risc-v-support-for-vxworks"
    },
    {
        "OS Platform": "AGL (Automotive Grade Linux)",
        "Type": "Linux-style Application OS",
        "Primary Deployment": "Automotive Infotainment, Cockpit",
        "Maturity": "Production",
        "License": "GPLv2",
        "Safety Cert (FuSa)": "ASIL support via hypervisors",
        "RISC-V Support Status": "Existing: Support via Yocto/RISC-V layers; active testing on RV64.",
        "Community & Backing": "Linux Foundation; 150+ member companies.",
        "Key Vendors/OEMs": "Toyota, Mazda, Suzuki, Denso.",
        "Reference Link": "https://www.automotivelinux.org/"
    },
    {
        "OS Platform": "OpenHarmony",
        "Type": "Distributed OS (Hybrid)",
        "Primary Deployment": "IoT, Smart Home, Industrial",
        "Maturity": "Prototype to Production",
        "License": "Apache 2.0 / Mulan",
        "Safety Cert (FuSa)": "IEC 61508 (L3/L4 targeted)",
        "RISC-V Support Status": "Growing: Active ports for RISC-V (e.g., Dashan/T-Head chips).",
        "Community & Backing": "OpenAtom Foundation; Huawei-backed.",
        "Key Vendors/OEMs": "Huawei, Chinasoft, various IoT vendors.",
        "Reference Link": "https://gitee.com/openharmony/community/tree/master/sig/sig_riscv"
    },
    {
        "OS Platform": "NuttX (Apache)",
        "Type": "POSIX RTOS",
        "Primary Deployment": "Drones, IoT, Automotive",
        "Maturity": "Production",
        "License": "Apache 2.0",
        "Safety Cert (FuSa)": "Used in safety-critical (Space/Drones)",
        "RISC-V Support Status": "Strong: Tier 1 support for RISC-V; used extensively in PX4/Drone ecosystem.",
        "Community & Backing": "Apache Software Foundation.",
        "Key Vendors/OEMs": "Sony, Xiaomi, PX4 Community.",
        "Reference Link": "https://nuttx.apache.org/docs/latest/platforms/riscv/index.html"
    },
    {
        "OS Platform": "SylixOS",
        "Type": "Hard RTOS",
        "Primary Deployment": "Industrial Control, Defense, Power Grid",
        "Maturity": "Production",
        "License": "Proprietary / Shared Source",
        "Safety Cert (FuSa)": "IEC 61508, EN 50128",
        "RISC-V Support Status": "Existing: Mature support for RISC-V; used in domestic (China) industrial apps.",
        "Community & Backing": "RealEvo; domestic Chinese ecosystem.",
        "Key Vendors/OEMs": "State Grid, Aerospace/Defense OEMs.",
        "Reference Link": "https://www.sylixos.com/"
    }
]

df = pd.DataFrame(data)

# Create a Excel writer object
with pd.ExcelWriter('OS_RISCV_Survey.xlsx', engine='openpyxl') as writer:
    df.to_excel(writer, index=False, sheet_name='OS Survey')
    
    # Auto-adjust columns width
    worksheet = writer.sheets['OS Survey']
    for idx, col in enumerate(df.columns):
        series = df[col]
        max_len = max((
            series.astype(str).map(len).max(),
            len(str(series.name))
            )) + 2
        worksheet.column_dimensions[chr(65 + idx)].width = min(max_len, 50) # Cap at 50

print("Excel file 'OS_RISCV_Survey.xlsx' created successfully.")
