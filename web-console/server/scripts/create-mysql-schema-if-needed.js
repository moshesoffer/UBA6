const mysql = require('mysql2/promise');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');
const { poolConfig } = require('../utils/dbConfiguration');

(async () => {
  const connection = await withTimeout(mysql.createConnection({
    host: poolConfig.host,
    user: poolConfig.user,
    password: poolConfig.password,
     port: poolConfig.port,   
  }), AWAIT_TIMEOUT);
  console.log('==> user pw', poolConfig.user, poolConfig.password, poolConfig.port);
  
  console.log('Creating schema IF NOT EXISTS...', poolConfig.host, poolConfig.user, poolConfig.password, poolConfig.database, poolConfig.port);
  await withTimeout(connection.query(`
    CREATE SCHEMA IF NOT EXISTS \`${poolConfig.database}\`
    DEFAULT CHARACTER SET utf8mb4
    COLLATE utf8mb4_0900_ai_ci;
  `), AWAIT_TIMEOUT);

  await withTimeout(connection.end(), AWAIT_TIMEOUT);
})();