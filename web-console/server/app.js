const logger = require('./utils/logger');
const { APIS, reportsDataPath } = require('./utils/constants');
const { createFolderIfNotExists } = require('./utils/helper');
const setupSwagger = require('./utils/swagger');
const express = require('express');
const cors = require('cors');
const helmet = require('helmet');
const path = require('node:path');
const logRoute = require('./middleware/loggerMiddleware'); // Adjust the path to where your middleware is

const authRouter = require('./routes/authRoutes');
const ubaDeviceRouter = require('./routes/ubaDeviceRoutes');
const testRoutineRouter = require('./routes/testRoutineRoutes');
const runningTestsRouter = require('./routes/runningTestsRoutes');
const reportRouter = require('./routes/reportRoutes');
const machineRouter = require('./routes/machineRoutes');
const cellRouter = require('./routes/cellRoutes');
const pendingTasksRouter = require('./routes/pendingTasksRoutes');
const pool = require('./db');
const { clearAllLastInstantTestResults } = require('./utils/testResultsHelper');
const { releaseAllWaiters } = require('./utils/ubaCommunicatorHelper');

// Ensure the reports data path exists
createFolderIfNotExists(reportsDataPath);

const app = express();
setupSwagger(app);
app.use(helmet());

if (process.env.ENABLE_CORS_FOR_LOCALHOST) {
	logger.info('CORS is enabled for localhost environment');
	app.use(cors({
	  origin: 'http://localhost:3000', // React development server
	  methods: ['GET', 'POST', 'PUT', 'DELETE', 'PATCH'], // Allowed HTTP methods
	  credentials: true, // Allow cookies
	}));
  } else {
	logger.info('CORS is disabled');
  }
  
  // Use the logging middleware for all routes
  app.use(logRoute);
  
  // Increase limit to 200mb
  app.use(express.json({ limit: '200mb' }));
  app.use(express.urlencoded({ limit: '200mb', extended: true }));
//app.use(express.json());
//app.use(express.urlencoded({extended: false}));

// Serve API requests.
// Health check endpoint
app.get('/health', async (req, res) => {
  let connection;
  try {
    connection = await pool.getConnection();
    await connection.ping();
    res.status(200).json({ status: 'ok', db: 'up' });
  } catch (err) {
    res.status(500).json({ status: 'error', db: 'down' });
  } finally {
    if (connection) connection.release();
  }
});

//this api is used mainly for testing purposes but in general can be used to clear all last instant test results
app.get('/clear-memory', async (req, res) => {
  clearAllLastInstantTestResults();
  releaseAllWaiters();
  res.status(200).json({ success: true });
});

app.use(APIS.apiInitials, authRouter);
app.use(APIS.apiInitials, ubaDeviceRouter);
app.use(APIS.apiInitials, testRoutineRouter);
app.use(APIS.apiInitials, runningTestsRouter);
app.use(APIS.apiInitials, reportRouter);
app.use(APIS.apiInitials, machineRouter);
app.use(APIS.apiInitials, cellRouter);
app.use(APIS.apiInitials, pendingTasksRouter);

// Serve static files from the React app.
app.get(/^\/[a-z\-]{2,35}$/i, (req, res, next) => res.sendFile(path.join(__dirname, 'dist', 'index.html')));
app.use(express.static(path.join(__dirname, 'dist')));
app.use('/*', express.static(path.join(__dirname, 'dist')));

// Error handler.
app.use(function(err, req, res, next) {
	// Set locals, only providing error in development.
	res.locals.message = err.message;
	res.locals.error = req.app.get('env') === 'development' ? err : {};

	// Render the error page.
	res.status(err.status || 500);
	res.render('error');
});

//keeping server alive
process.on('uncaughtException', (error) => {
	logger.error('Uncaught Exception:', error);
});

process.on('unhandledRejection', (reason, promise) => {
	logger.error('Unhandled Rejection:', reason);
});




module.exports = app;
