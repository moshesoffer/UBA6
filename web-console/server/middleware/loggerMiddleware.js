const logger = require('../utils/logger'); // Ensure this points to your Winston logger instance
//const { v4: uuidv4 } = require('uuid');

const logRoute = (req, res, next) => {
    //req.requestId = uuidv4(); // Generate a unique ID for each request
    const startTime = Date.now();
    // Log the start of the request
    logger.debug(`Started [${req.originalUrl}]-[${req.method}] hostname:[${req.hostname}] protocol:[${req.protocol}]`);

    /*const params = JSON.stringify(req.params);
    const query = JSON.stringify(req.query);
    const body = JSON.stringify(req.body);
    const headers = JSON.stringify(req.headers);

    logger.silly(`headers: ${headers}`);
    logger.debug(`params:[${params}] query:[${query}] body:[${body}]`);
    */
    res.on('finish', () => {
        const duration = Date.now() - startTime;
        logger.debug(`Completed ${req.method} ${req.originalUrl} with status ${res.statusCode} in ${duration}ms`);
    });

    next();
};

module.exports = logRoute;