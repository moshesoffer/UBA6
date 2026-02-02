const winston = require('winston');
require('winston-daily-rotate-file');
const { getRequestId } = require('./requestContext');

winston.addColors({
  debug: 'brightBlue', // instead of blue
});

const logFormat = winston.format.printf(({ timestamp, level, message, stack, ...metadata }) => {
  const requestId = getRequestId();
  const idPrefix = requestId ? `[${requestId}] ` : '';
  const meta = Object.keys(metadata).length ? JSON.stringify(metadata, null, 2) : '';
  return stack
    ? `[${timestamp}] [${level}]: ${idPrefix}${message} - ${stack}`
    : `[${timestamp}] [${level}]: ${idPrefix}${message} ${meta}`;
});

const logger = winston.createLogger({
  level: 'debug',
  format: winston.format.combine(
    winston.format.errors({ stack: true }) // Base format for all transports
  ),
  transports: [
    // File logs - plain
    new winston.transports.DailyRotateFile({
      filename: 'logs/app-%DATE%.log',
      datePattern: 'YYYY-MM-DD-HH',
      maxSize: '10m',
      maxFiles: '14d',
      zippedArchive: true,
      format: winston.format.combine(
        winston.format.timestamp({ format: 'YYYY-MM-DD HH:mm:ss.SSS' }),
        logFormat
      )
    }),

    // Console logs - with colors
    new winston.transports.Console({
      format: winston.format.combine(
        winston.format.timestamp({ format: 'YYYY-MM-DD HH:mm:ss.SSS' }),
        winston.format.colorize({ all: true }),
        logFormat
      )
    })
  ]
});

module.exports = logger;