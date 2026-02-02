const logger = require('../utils/logger'); // Ensure this points to your Winston logger instance
const { asyncLocalStorage } = require('../utils/requestContext');
const { v4: uuidv4 } = require('uuid');

const logRoute = (req, res, next) => {
    const requestId = uuidv4();
    const store = { requestId };
    
    asyncLocalStorage.run(store, () => {
        const startTime = Date.now();
        const params = JSON.stringify(req.params);
        const query = JSON.stringify(req.query);
        logger.debug(`Started [${req.originalUrl}]-[${req.method}] params:[${params}] query:[${query}] hostname:[${req.hostname}] protocol:[${req.protocol}]`);
        /*
        const body = JSON.stringify(req.body);
        const headers = JSON.stringify(req.headers);

        logger.silly(`[${req.requestId}] headers: ${headers}`);
        logger.debug(`[${req.requestId}] body:[${body}]`);
        */

        res.on('finish', () => {
            const duration = Date.now() - startTime;
            logger.debug(`Completed ${req.method} ${req.originalUrl} with status ${res.statusCode} in ${duration}ms`);
        });

        next();
    });
};

module.exports = logRoute;