import pandas as pd
from openpyxl import load_workbook

# New data points from the EET China 2024 Survey context
survey_data = [
    {
        "Metric": "RISC-V Adoption Trend",
        "Details": "Significant increase in domestic industrial and automotive design-ins. Over 35% of developers express high interest or ongoing projects.",
        "Key Findings": "RISC-V is becoming the 'third pillar' alongside ARM and x86 in China's domestic ecosystem."
    },
    {
        "Metric": "RT-Thread Smart Market Position",
        "Details": "Highly recognized as a leading domestic microkernel for 'software-defined' hardware.",
        "Key Findings": "Strongest adoption in smart cockpits and high-performance industrial gateways among domestic RTOS."
    },
    {
        "Metric": "Functional Safety Priority",
        "Details": "ISO 26262 ASIL-D and IEC 61508 SIL3 have become mandatory entry requirements for automotive/industrial SoC vendors.",
        "Key Findings": "Domestic OS (RT-Thread, SylixOS) have caught up with international leaders in safety certification."
    },
    {
        "Metric": "Software-Hardware Synergy",
        "Details": "Growing shift towards 'Integrated Development' (Chip + OS + Middleware).",
        "Key Findings": "T-Head (玄铁) and RT-Thread/OpenHarmony are forming the most mature domestic RISC-V software stacks."
    }
]

# Update the Excel file
filename = 'Domestic_OS_RISCV_Survey_CN.xlsx'
df_survey = pd.DataFrame(survey_data)

with pd.ExcelWriter(filename, engine='openpyxl', mode='a', if_sheet_exists='replace') as writer:
    df_survey.to_excel(writer, index=False, sheet_name='EET China 2024 Highlights')
    
    # Auto-adjust columns width for the new sheet
    worksheet = writer.sheets['EET China 2024 Highlights']
    for idx, col in enumerate(df_survey.columns):
        series = df_survey[col]
        max_len = max((
            series.astype(str).map(len).max(),
            len(str(series.name))
            )) + 2
        worksheet.column_dimensions[chr(65 + idx)].width = min(max_len, 70)

print(f"Excel file '{filename}' updated with EET China 2024 survey data.")
