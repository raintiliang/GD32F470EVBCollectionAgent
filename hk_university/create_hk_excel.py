import openpyxl
from openpyxl.styles import Font, Alignment, PatternFill

# Create a new workbook
wb = openpyxl.Workbook()
ws = wb.active
ws.title = "HK Uni Enrollment 2024-25"

# Define headers
headers = [
    "University (中文)", 
    "University (English)", 
    "Enrollment Mode", 
    "Est. Mainland Quota (2024/25)", 
    "Min. Score Requirement (General)", 
    "Key Majors / Faculties open to Mainland"
]

# Write headers
for col, header in enumerate(headers, 1):
    cell = ws.cell(row=1, column=col, value=header)
    cell.font = Font(bold=True, color="FFFFFF")
    cell.fill = PatternFill(start_color="4F81BD", end_color="4F81BD", fill_type="solid")
    cell.alignment = Alignment(horizontal="center", vertical="center")

# Data
data = [
    [
        "香港都会大学", 
        "HKMU", 
        "Independent Recruitment (自主招生)", 
        "1100 - 1500", 
        "Second Tier (二本/本科线) or above", 
        "人文社会科学院 (Criminology/Psychology), 创意艺术, 商学院, 教育及语文, 护理及健康"
    ],
    [
        "岭南大学", 
        "Lingnan University (LU)", 
        "Independent Recruitment (自主招生)", 
        "~150 - 200", 
        "First Tier (一本/特招线) or above", 
        "文学院 (History/Literature), 商学院, 社会科学院"
    ],
    [
        "香港浸会大学", 
        "HKBU", 
        "Independent Recruitment (自主招生)", 
        "~150 - 160", 
        "First Tier (一本/特招线) or above", 
        "传理学院 (Media/Journalism), 文学院, 社会科学院, 理学院, 工商管理学院"
    ],
    [
        "香港树仁大学", 
        "HKSYU", 
        "Independent Recruitment (自主招生)", 
        "~150 - 200", 
        "Second Tier (二本/本科线) or above", 
        "文学院 (History/Chinese), 社会科学院 (Sociology/Counseling/Psychology), 商学院"
    ],
    [
        "东华学院", 
        "Tung Wah College (TWC)", 
        "Independent Recruitment (自主招生)", 
        "~100", 
        "Second Tier (二本/本科线) or above", 
        "护理学院, 医疗及健康科学学院, 管理学院, 人文学院"
    ],
    [
        "香港珠海学院", 
        "HKCHC", 
        "Independent Recruitment (自主招生)", 
        "~100 - 200", 
        "Second Tier (二本/本科线) or above", 
        "文学与社会科学院 (Journalism/Chinese), 商学院, 理工学院"
    ]
]

# Write data
for row_idx, row_data in enumerate(data, 2):
    for col_idx, value in enumerate(row_data, 1):
        cell = ws.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = Alignment(wrap_text=True, vertical="top")

# Adjust column widths
widths = [20, 25, 30, 25, 30, 60]
for i, width in enumerate(widths, 1):
    ws.column_dimensions[openpyxl.utils.get_column_letter(i)].width = width

# Save the workbook
file_path = "/home/rainti/.openclaw/workspace/HK_Uni_Mainland_Enrollment_2024_2025.xlsx"
wb.save(file_path)
print(f"Excel file saved to {file_path}")
