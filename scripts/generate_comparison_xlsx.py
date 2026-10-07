import openpyxl
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side

def create_excel():
    wb = openpyxl.Workbook()
    ws = wb.active
    ws.title = "AI Accelerator Comparison"

    # Define Headers
    headers = [
        "Accelerator / 平台名称", 
        "Perf vs H100 / 相较H100性能", 
        "Perf vs H200 / 相较H200性能", 
        "Price-Perf / 性价比表现", 
        "LLM Training / LLM训练能力", 
        "AI Inference / AI推理能力", 
        "Deployment Challenges / 部署难点", 
        "Ecosystem Maturity / 生态成熟度", 
        "Strengths & Shortcomings / 优势与短板"
    ]

    # Data based on research
    data = [
        ["NVIDIA H100", "100% (Baseline)", "70%", "Medium", "High", "High", "Supply chain limits", "Very High (CUDA)", "Pro: Industry standard; Con: 80GB VRAM limit"],
        ["NVIDIA H200", "160%+", "100% (Baseline)", "Medium", "High", "High", "High cost", "Very High (CUDA)", "Pro: 141GB HBM3e; Con: Extremely expensive"],
        ["AMD MI325X", "150-200%", "120-140%", "High", "Med-High", "High", "ROCm migration", "Medium", "Pro: 256GB HBM3e; Con: Interconnect vs NVLink"],
        ["Intel Gaudi 3", "100-110%", "80-90%", "Very High", "Med-High", "High", "OneAPI adaptation", "Medium", "Pro: 1.6x perf/$, 800GbE; Con: Lower peak TFLOPS"],
        ["Google TPU v5p", "80-115%", "60-80%", "High", "High", "Med-High", "GCP platform lock", "High (JAX/XLA)", "Pro: Massive scalability; Con: No on-premise option"],
        ["Huawei Ascend 910C", "80-100%", "60-70%", "High (CN)", "Med-High", "High", "CANN platform learning", "Medium (CANN)", "Pro: Local CN support; Con: Manufacturing sanctions"],
        ["Groq LPU", "300-500%", "200-300%", "Very High", "Low", "Very High", "Small memory pooling", "Medium", "Pro: Ultra-low latency; Con: Low SRAM (capacity)"],
        ["Cerebras CS-3", "2000%+", "1200%+", "High", "Very High", "Very High", "Specialized HW racks", "Medium", "Pro: Wafer-scale bandwidth; Con: Power & Proprietary"],
        ["SambaNova SN40L", "150-250%", "120-150%", "High", "Low", "High", "Expert pathing", "Low-Med", "Pro: 12TB DDR5 memory; Con: No training support"],
        ["Tenstorrent Blackhole", "70-80%", "60-70%", "Very High", "Medium", "High", "Early dev tools", "Low (BUDA)", "Pro: RISC-V, spatial dataflow; Con: Memory bandwidth"],
        ["Biren BR100", "60-80%", "< 50%", "High (CN)", "Medium", "Medium", "Export controls", "Low-Med", "Pro: Strong FP16; Con: Lacks FP8 support"],
    ]

    # Write Headers
    for col, header in enumerate(headers, 1):
        cell = ws.cell(row=1, column=col, value=header)
        cell.font = Font(bold=True, color="FFFFFF")
        cell.fill = PatternFill(start_color="4F81BD", end_color="4F81BD", fill_type="solid")
        cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)

    # Write Data
    for row_idx, row_data in enumerate(data, 2):
        for col_idx, value in enumerate(row_data, 1):
            cell = ws.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = Alignment(vertical="center", wrap_text=True)
            if col_idx in [5, 6] and value in ["High", "Very High"]:
                cell.font = Font(bold=True, color="008000") # Green for High
            if value == "Low":
                cell.font = Font(color="FF0000") # Red for Low

    # Adjust Column Widths
    column_widths = [20, 20, 20, 15, 15, 15, 25, 20, 45]
    for i, width in enumerate(column_widths, 1):
        ws.column_dimensions[openpyxl.utils.get_column_letter(i)].width = width

    # Add Ranking Sheet or section
    ws.append([])
    ws.append(["Recommendations / 推荐总结"])
    ws.cell(row=ws.max_row, column=1).font = Font(bold=True, size=14)
    
    recs = [
        ["大规模训练最优方案", "NVIDIA H100/H200 (Standard), Google TPU v5p (GCP Only), Cerebras CS-3 (High-end)"],
        ["AI推理部署最优方案", "NVIDIA H200, AMD MI325X (Large Batch), Groq LPU (Latency focus)"],
        ["性价比最高方案", "Intel Gaudi 3, Groq (for specialized inference)"],
        ["最佳非英伟达替代方案", "AMD MI325X (General), Huawei Ascend 910C (China Market)"]
    ]
    for rec in recs:
        ws.append(rec)
        ws.cell(row=ws.max_row, column=1).font = Font(bold=True)

    wb.save("ai_accelerator_comparison.xlsx")
    print("Excel file 'ai_accelerator_comparison.xlsx' generated successfully.")

if __name__ == "__main__":
    create_excel()
