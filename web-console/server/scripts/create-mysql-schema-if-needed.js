const mysql = require('mysql2/promise');
const { poolConfig } = require('../utils/dbConfiguration');

(async () => {
  const connection = await mysql.createConnection({
    host: poolConfig.host,
    user: poolConfig.user,
    password: poolConfig.password,
     port: poolConfig.port,   
  });
  console.log('==> user pw', poolConfig.user, poolConfig.password);
  
  console.log('Creating schema IF NOT EXISTS...', poolConfig.host, poolConfig.user, poolConfig.password, poolConfig.database, poolConfig.port);
  await connection.query(`
    CREATE SCHEMA IF NOT EXISTS \`${poolConfig.database}\`
    DEFAULT CHARACTER SET utf8mb4
    COLLATE utf8mb4_0900_ai_ci;
  `);

  await connection.end();
})();