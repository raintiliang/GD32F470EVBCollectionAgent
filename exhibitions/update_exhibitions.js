const XLSX = require('xlsx');
const fs = require('fs');

const wb = XLSX.utils.book_new();

// 通用样式设置函数（由于xlsx库基础版样式受限，主要通过数据结构优化）
function createSheet(data, sheetName) {
    const ws = XLSX.utils.aoa_to_sheet(data);
    // 设置列宽
    const wscols = [
        {wch: 25}, // 展会名称
        {wch: 20}, // 时间
        {wch: 20}, // 地点
        {wch: 25}, // 网址
        {wch: 60}, // 主题/关注领域
        {wch: 30}  // 重点品牌关注 (SUPCON/HollySys)
    ];
    ws['!cols'] = wscols;
    XLSX.utils.book_append_sheet(wb, ws, sheetName);
}

// 表一：能源工业展会
const energyData = [
    ["展会名称", "时间", "地点", "网址", "主题/关注领域", "重点品牌关注 (SUPCON/HollySys)"],
    ["EP Shanghai 2026 (第22届上海国际电力展)", "2026年12月", "上海新国际博览中心", "www.epchinashow.com", "电力能源全产业链：发电、输配电、储存、电力自动化控制系统", "和利时(电力强项)、中控技术"],
    ["CIPPE 2026 (中国国际石油石化装备展)", "2026年3月26-28日", "北京·中国国际展览中心(顺义馆)", "www.cippe.com.cn", "含子展CIEECA(自动化专区)；石油天然气全产业链自动化", "中控技术、和利时"],
    ["CIOOE 2026 (北京国际海洋石油展)", "2026年3月26-28日", "北京·中国国际展览中心(顺义馆)", "www.cippe.com.cn", "海洋石油天然气开采设备及自动化控制技术", "中控技术"],
    ["CIPE 2026 (北京国际油气储运展)", "2026年3月26-28日", "北京·中国国际展览中心(顺义馆)", "www.cippe.com.cn", "管道与储运技术；油气管输、长距离监控SCADA系统", "中控技术"],
    ["HEIE 2026 (北京国际氢能装备展)", "2026年3月26-28日", "北京·中国国际展览中心(顺义馆)", "www.cippe.com.cn", "氢能技术装备；新能源控制系统", "中控技术"],
    ["Nuclear Industry China 2026 (核工业展)", "2026年4月 (已结) / 2028", "北京", "www.nic-expo.net", "核电仪控系统(I&C)、核级数字化控制平台DCS", "和利时(核电优势)"]
];
createSheet(energyData, "能源工业展会");

// 表二：石油化工展会
const petroData = [
    ["展会名称", "时间", "地点", "网址", "主题/关注领域", "重点品牌关注 (SUPCON/HollySys)"],
    ["CIPPE 2026 (石油石化大展)", "2026年3月26-28日", "北京·中国国际展览中心(顺义馆)", "www.cippe.com.cn", "石油化工全产业链；CIEECA自动化专区(工艺过程自动控制)", "中控技术、和利时"],
    ["ICIF China 2026 (中国国际化工展)", "2026年9月 (预计)", "上海", "www.icif.cn", "能源与石油化工、精细化工、数字化-智能制造、化工工程", "中控技术、和利时"],
    ["CING 2026 (北京国际天然气技术展)", "2026年3月26-28日", "北京·中国国际展览中心(顺义馆)", "www.cippe.com.cn", "天然气开采、储运、应用技术装备及控制系统", "中控技术"],
    ["CISPE 2026 (石化安全防护展)", "2026年3月26-28日", "北京·中国国际展览中心(顺义馆)", "www.cippe.com.cn", "石化安全生产防护、SIS安全仪表系统", "中控技术、和利时"],
    ["CITTE 2026 (北京国际地下工程展)", "2026年3月26-28日", "北京·中国国际展览中心(顺义馆)", "www.cippe.com.cn", "地下工程、非开挖技术(石化储运相关自动化)", "中控技术"]
];
createSheet(petroData, "石油化工展会");

XLSX.writeFile(wb, "Exhibition_Summary_v2.xlsx");
