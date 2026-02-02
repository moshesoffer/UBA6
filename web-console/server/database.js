//this is being used by db-migrate, mysql schema migration scripts framework
const { poolConfig } = require('./utils/dbConfiguration');
console.log('Loading db-migrate config from database.js', poolConfig.host, poolConfig.user, poolConfig.password, poolConfig.database, poolConfig.port);

module.exports = {
  dev: {
    driver: 'mysql',
    host: poolConfig.host,
    user: poolConfig.user,
    password: poolConfig.password,
    database: poolConfig.database,
    port: poolConfig.port,
    multipleStatements: true,
  }
};