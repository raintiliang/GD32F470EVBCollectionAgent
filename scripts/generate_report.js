const ExcelJS = require('exceljs');
const fs = require('fs');

async function generateReport() {
    // Simulated historical data for CSI 300 (May 2025 - May 2026)
    const data = [
        { Month: "2025-05", Price: 3907.20, MA200: 3650.00 },
        { Month: "2025-06", Price: 3872.24, MA200: 3680.00 },
        { Month: "2025-07", Price: 3974.72, MA200: 3720.00 },
        { Month: "2025-08", Price: 4052.88, MA200: 3780.00 },
        { Month: "2025-09", Price: 4091.95, MA200: 3850.00 },
        { Month: "2025-10", Price: 4168.08, MA200: 3920.00 },
        { Month: "2025-11", Price: 4135.25, MA200: 4000.00 },
        { Month: "2025-12", Price: 4193.08, MA200: 4100.00 },
        { Month: "2026-01", Price: 4235.65, MA200: 4250.00 },
        { Month: "2026-02", Price: 4652.73, MA200: 4380.00 },
        { Month: "2026-03", Price: 4762.64, MA200: 4520.00 },
        { Month: "2026-04", Price: 4807.31, MA200: 4650.00 },
        { Month: "2026-05", Price: 4859.59, MA200: 4715.44 }
    ];

    const baseAmount = 2000;

    function getMultiplier(price, ma) {
        const dev = (price - ma) / ma;
        if (dev > 0.15) return 0.6;
        if (dev > 0.05) return 0.8;
        if (dev > -0.05) return 1.0;
        if (dev > -0.15) return 1.6;
        return 2.0;
    }

    const workbook = new ExcelJS.Workbook();
    const worksheet = workbook.addWorksheet('Algorithmic DCA Report');

    worksheet.columns = [
        { header: 'Month', key: 'month', width: 15 },
        { header: 'CSI300 Price', key: 'price', width: 15 },
        { header: '200-day MA', key: 'ma', width: 15 },
        { header: 'Deviation (%)', key: 'deviation', width: 15 },
        { header: 'Multiplier', key: 'multiplier', width: 12 },
        { header: 'Invest Amount (RMB)', key: 'amount', width: 22 },
        { header: 'Shares Acquired', key: 'shares', width: 18 }
    ];

    let totalInvested = 0;
    let totalShares = 0;

    data.forEach(entry => {
        const multiplier = getMultiplier(entry.Price, entry.MA200);
        const investAmount = baseAmount * multiplier;
        const shares = investAmount / entry.Price;
        const deviation = ((entry.Price / entry.MA200 - 1) * 100).toFixed(2) + '%';

        totalInvested += investAmount;
        totalShares += shares;

        worksheet.addRow({
            month: entry.Month,
            price: entry.Price,
            ma: entry.MA200,
            deviation: deviation,
            multiplier: multiplier,
            amount: investAmount,
            shares: parseFloat(shares.toFixed(4))
        });
    });

    // Styling Headers
    worksheet.getRow(1).eachCell((cell) => {
        cell.fill = {
            type: 'pattern',
            pattern: 'solid',
            fgColor: { argb: 'FF4F81BD' }
        };
        cell.font = { color: { argb: 'FFFFFFFF' }, bold: true };
        cell.alignment = { horizontal: 'center' };
    });

    // Summary Rows
    worksheet.addRow([]);
    const summaryRow = worksheet.addRow(['Total Summary', '', '', '', '', totalInvested, parseFloat(totalShares.toFixed(4))]);
    summaryRow.font = { bold: true };
    worksheet.mergeCells(`A${summaryRow.number}:E${summaryRow.number}`);

    const finalPrice = data[data.length - 1].Price;
    const totalValue = totalShares * finalPrice;
    const valueRow = worksheet.addRow(['Portfolio Value (May 2026)', '', '', '', '', parseFloat(totalValue.toFixed(2))]);
    valueRow.font = { bold: true, color: { argb: 'FFFF0000' } };
    worksheet.mergeCells(`A${valueRow.number}:E${valueRow.number}`);

    const filePath = '/home/rainti/.openclaw/workspace/CSI300_Algorithmic_DCA_Report_2026.xlsx';
    await workbook.xlsx.writeFile(filePath);
    console.log(`File saved to ${filePath}`);
}

generateReport();
