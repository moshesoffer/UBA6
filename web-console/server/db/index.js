const mysql = require('mysql2/promise');
const dbConfiguration = require('../utils/dbConfiguration');
const pool = mysql.createPool(dbConfiguration.poolConfig);
module.exports = pool;