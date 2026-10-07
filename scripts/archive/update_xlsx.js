const XLSX = require('/home/rainti/project/autohomeworkcheck/node_modules/xlsx');
const path = '/home/rainti/.openclaw/workspace/AI_Accelerator_Comparison_2026.xlsx';
const workbook = XLSX.readFile(path);

// 1. Update "综合对比" sheet
const sheet1Name = '综合对比';
const ws1 = workbook.Sheets[sheet1Name];
const data1 = XLSX.utils.sheet_to_json(ws1, { header: 1 });

for (let i = 0; i < data1.length; i++) {
    const name = data1[i][0] ? data1[i][0].toString() : '';
    if (name.includes('Huawei Ascend') && name.includes('910C')) {
        // Memory column is index 4
        data1[i][4] = "96-128 GB
HBM3
(双芯)";
        break;
    }
}
workbook.Sheets[sheet1Name] = XLSX.utils.aoa_to_sheet(data1);

// 2. Update "优劣势与部署难点" sheet
const sheet2Name = '优劣势与部署难点';
const ws2 = workbook.Sheets[sheet2Name];
const data2 = XLSX.utils.sheet_to_json(ws2, { header: 1 });

for (let i = 0; i < data2.length; i++) {
    const name = data2[i][0] ? data2[i][0].toString() : '';
    if (name.includes('Huawei Ascend 910C')) {
        if (data2[i][1]) {
            data2[i][1] = data2[i][1].toString().replace('64GB HBM2e(双芯)', '96-128GB HBM3(双芯)');
        }
        break;
    }
}
workbook.Sheets[sheet2Name] = XLSX.utils.aoa_to_sheet(data2);

XLSX.writeFile(workbook, path);
console.log('Workbook updated successfully.');
