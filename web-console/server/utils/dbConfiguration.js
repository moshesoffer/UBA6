const logger = require('./logger');
const host = process.env.DB_CONFIGURATION_HOST || 'localhost';
const user = process.env.DB_CONFIGURATION_USER || 'root';
const password = process.env.DB_CONFIGURATION_PASSWORD || '12345678';
const database = process.env.DB_CONFIGURATION_DB_NAME || 'Amicell';
const port = process.env.DB_CONFIGURATION_PORT || 3306;
logger.info(`host: `, { host, user, password, database, port });

exports.poolConfig = {
	host,
	port,
	user,
	password,
	database,
	waitForConnections: true,
	connectionLimit: 10,
	queueLimit: 0,
	enableKeepAlive: true,
	keepAliveInitialDelay: 0,
	timezone: '+00:00',
  	decimalNumbers: true
};
