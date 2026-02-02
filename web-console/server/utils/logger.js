const winston = require('winston');
require('winston-daily-rotate-file');

// Generate a unique identifier for each server start
//const serverStartTime = new Date().toISOString().replace(/:/g, '-');
//console.log(serverStartTime);

const logger = winston.createLogger({
  level: 'debug',
  format: winston.format.combine(
    winston.format.timestamp({ format: 'YYYY-MM-DD HH:mm:ss.SSS' }),
    winston.format.errors({ stack: true }), // Ensure stack trace is included
    winston.format.printf(({ timestamp, level, message, stack, ...metadata }) => {
      const meta = Object.keys(metadata).length ? JSON.stringify(metadata, null, 2) : '';
      //const requestId = info.requestId ? `[Request ID: ${info.requestId}] ` : '';
      return stack
        ? `[${timestamp}] [${level.toUpperCase()}]: ${message} - ${stack}`
        : `[${timestamp}] [${level.toUpperCase()}]: ${message} ${meta}`;
    })
  ),
  transports: [
    new winston.transports.DailyRotateFile({
      filename: 'logs/app-%DATE%.log', // Unique file per server start and hourly rotation
      datePattern: 'YYYY-MM-DD-HH',
      maxSize: '10m',
      maxFiles: '14d',
      zippedArchive: true
    }),
    new winston.transports.Console()
  ],
});

module.exports = logger;