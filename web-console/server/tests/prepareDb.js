const fs = require('fs');
const path = require('path');
const mysql = require('mysql2/promise');

const runSchema = async () => {
    console.log('start running schema');
    const sqlFilePath = path.join(__dirname, 'resources', 'schema.sql');
    let connection = await mysql.createConnection(global.__MYSQL_CONFIG__);
    try {
        const sql = fs.readFileSync(sqlFilePath, 'utf8');
        const statements = sql.split(';').map(statement => statement.trim()).filter(statement => statement.length);
        for (const statement of statements) {
            await connection.query(statement);
        }
        console.log('SQL file executed successfully!');
    } catch (error) {
        console.error('Error executing SQL file:', error);
        throw error;
    } finally {
        await connection.end();
    }
};

module.exports = runSchema;