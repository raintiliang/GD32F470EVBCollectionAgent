const XLSX = require('/home/rainti/project/autohomeworkcheck/node_modules/xlsx');
const workbook = XLSX.readFile('/home/rainti/.openclaw/workspace/AI_Accelerator_Comparison_2026.xlsx');
workbook.SheetNames.forEach(sheetName => {
    console.log(`
--- Sheet: ${sheetName} ---`);
    const worksheet = workbook.Sheets[sheetName];
    const data = XLSX.utils.sheet_to_json(worksheet, { header: 1 });
    data.slice(0, 15).forEach(row => console.log(JSON.stringify(row)));
});
