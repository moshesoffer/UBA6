
using System;
using System.Data;
using System.IO;
using MySql.Data.MySqlClient;
using OfficeOpenXml;

class MySqlExcelTool
{
    static string connStr = "server=localhost;user=root;password=YOUR_PASS;database=YOUR_DB;";

    static void Main()
    {
        ExcelPackage.LicenseContext = LicenseContext.NonCommercial;

        string file = "data.xlsx";

        ExportToExcel("your_table", file);
        ImportFromExcel("your_table_copy", file);

        Console.WriteLine("Done.");
    }

    // =========================
    // EXPORT → EXCEL
    // =========================
    static void ExportToExcel(string table, string filePath)
    {
        using var conn = new MySqlConnection(connStr);
        conn.Open();

        string query = $"SELECT * FROM {table}";
        using var adapter = new MySqlDataAdapter(query, conn);
        var dt = new DataTable();
        adapter.Fill(dt);

        using var package = new ExcelPackage();
        var sheet = package.Workbook.Worksheets.Add("Data");

        // Load DataTable into Excel
        sheet.Cells["A1"].LoadFromDataTable(dt, true);

        // Auto-fit columns (nice touch)
        sheet.Cells[sheet.Dimension.Address].AutoFitColumns();

        File.WriteAllBytes(filePath, package.GetAsByteArray());
    }

    // =========================
    // IMPORT ← EXCEL
    // =========================
    static void ImportFromExcel(string table, string filePath)
    {
        using var conn = new MySqlConnection(connStr);
        conn.Open();

        using var package = new ExcelPackage(new FileInfo(filePath));
        var sheet = package.Workbook.Worksheets[0];

        int rows = sheet.Dimension.Rows;
        int cols = sheet.Dimension.Columns;

        // Read headers
        string[] columns = new string[cols];
        for (int c = 1; c <= cols; c++)
        {
            columns[c - 1] = sheet.Cells[1, c].Text;
        }

        // Insert rows
        for (int r = 2; r <= rows; r++)
        {
            string colList = string.Join(",", columns);
            string paramList = string.Join(",", Array.ConvertAll(columns, c => "@" + c));

            string sql = $"INSERT INTO {table} ({colList}) VALUES ({paramList})";

            using var cmd = new MySqlCommand(sql, conn);

            for (int c = 1; c <= cols; c++)
            {
                var val = sheet.Cells[r, c].Value ?? DBNull.Value;
                cmd.Parameters.AddWithValue("@" + columns[c - 1], val);
            }

            cmd.ExecuteNonQuery();
        }
    }
}