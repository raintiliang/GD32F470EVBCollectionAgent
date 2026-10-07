const xlsx = require('xlsx');

const data = [
  {
    "厂商 / 产品": "NVIDIA H100 (SXM)",
    "架构": "Hopper",
    "工艺节点": "4N (TSMC)",
    "FP16 TFLOPS (Dense)": 989,
    "FP16 TFLOPS (Sparse)": 1979,
    "显存 (HBM)": "80 GB HBM3",
    "内存带宽 (TB/s)": 3.35,
    "TDP (W)": 700,
    "生态成熟度": "⭐⭐⭐⭐⭐",
    "核心亮点": "CUDA生态主导, NVLink, 大规模部署标准"
  },
  {
    "厂商 / 产品": "NVIDIA H200 (SXM)",
    "架构": "Hopper H200",
    "工艺节点": "4N (TSMC)",
    "FP16 TFLOPS (Dense)": 989,
    "FP16 TFLOPS (Sparse)": 1979,
    "显存 (HBM)": "141 GB HBM3e",
    "内存带宽 (TB/s)": 4.8,
    "TDP (W)": 700,
    "生态成熟度": "⭐⭐⭐⭐⭐",
    "核心亮点": "H100升级, 显存容量与带宽大幅提升"
  },
  {
    "厂商 / 产品": "AMD Instinct MI300X",
    "架构": "CDNA 3",
    "工艺节点": "5nm (TSMC)",
    "FP16 TFLOPS (Dense)": 1307,
    "FP16 TFLOPS (Sparse)": 2614,
    "显存 (HBM)": "192 GB HBM3",
    "内存带宽 (TB/s)": 5.3,
    "TDP (W)": 750,
    "生态成熟度": "⭐⭐⭐",
    "核心亮点": "单卡显存领先, 推理性价比极高"
  },
  {
    "厂商 / 产品": "AMD Instinct MI325X",
    "架构": "CDNA 3",
    "工艺节点": "5nm (TSMC)",
    "FP16 TFLOPS (Dense)": 1307,
    "FP16 TFLOPS (Sparse)": 2610,
    "显存 (HBM)": "256 GB HBM3e",
    "内存带宽 (TB/s)": 6.0,
    "TDP (W)": 750,
    "生态成熟度": "⭐⭐⭐",
    "核心亮点": "256GB显存行业最高, 带宽6TB/s"
  },
  {
    "厂商 / 产品": "Intel Gaudi 3",
    "架构": "Gaudi 3",
    "工艺节点": "5nm (TSMC)",
    "FP16 TFLOPS (Dense)": 1835,
    "FP16 TFLOPS (Sparse)": "N/A",
    "显存 (HBM)": "128 GB HBM2e",
    "内存带宽 (TB/s)": 3.7,
    "TDP (W)": 600,
    "生态成熟度": "⭐⭐",
    "核心亮点": "集成24x200Gb RoCE网口, 组网成本优势"
  },
  {
    "厂商 / 产品": "Google TPU v5p",
    "架构": "TPU v5p",
    "工艺节点": "Custom (Google)",
    "FP16 TFLOPS (Dense)": 459,
    "FP16 TFLOPS (Sparse)": "N/A",
    "显存 (HBM)": "95 GB HBM3",
    "内存带宽 (TB/s)": 2.76,
    "TDP (W)": 450,
    "生态成熟度": "⭐⭐⭐",
    "核心亮点": "Google云端训练主力, 分布式优化极佳"
  },
  {
    "厂商 / 产品": "Huawei Ascend 910C",
    "架构": "Da Vinci v2",
    "工艺节点": "7nm (SMIC)",
    "FP16 TFLOPS (Dense)": "~800",
    "FP16 TFLOPS (Sparse)": "N/A",
    "显存 (HBM)": "128 GB HBM",
    "内存带宽 (TB/s)": "~3.2",
    "TDP (W)": "~600",
    "生态成熟度": "⭐",
    "核心亮点": "国产最强替代, 算力对标H100, 受制于CANN生态"
  },
  {
    "厂商 / 产品": "Biren BR100",
    "架构": "BR100",
    "工艺节点": "7nm",
    "FP16 TFLOPS (Dense)": 1000,
    "FP16 TFLOPS (Sparse)": "N/A",
    "显存 (HBM)": "64 GB HBM2e",
    "内存带宽 (TB/s)": 2.0,
    "TDP (W)": 400,
    "生态成熟度": "⭐",
    "核心亮点": "国产高算力GPU, 算力指标强劲, 供应受限"
  }
];

const ws = xlsx.utils.json_to_sheet(data);
const wb = xlsx.utils.book_new();
xlsx.utils.book_append_sheet(wb, ws, "Verified_AI_Accelerators");

xlsx.writeFile(wb, "AI_Accelerator_Comparison_Verified_2026.xlsx");
console.log("Success: AI_Accelerator_Comparison_Verified_2026.xlsx generated.");
