const logger = require('../utils/logger');
const { getPendingTasks, queryUbaDevice, pendingTasksExecuted } = require('../services/pendingTasksService');

//being called from uba service
exports.getPendingTasks = async (req, res) => {
    try {
        const pendingTasks = await getPendingTasks(req.query?.machineMac);
        res.json(pendingTasks);
    } catch (error) {
		logger.error('getPendingTasks', error);
		res.sendStatus(500);
	}
    
};

//being called from uba service
exports.pendingTasksExecuted = async (req, res) => {
    try {
        await pendingTasksExecuted(req.body);
        res.status(200).json({ success: true });
    } catch (error) {
        logger.error('pendingTasksExecuted', error);
        return res.status(400).json({ error: error.message });
    }
};

//this is being trigger from web ui CM
exports.queryUbaDevice = async (req, res) => {
	try {
		const result = await queryUbaDevice(req.body);
		res.status(201).json( result );//the result is coming from uba device it self
	} catch (error) {
		logger.error('queryUbaDevice', error);
		return res.status(400).json({ error: error.message });
	}
};
