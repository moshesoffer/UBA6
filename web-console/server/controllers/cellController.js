const logger = require('../utils/logger');
const { getCells, createCell, updateCell, deleteCell } = require('../services/cellService');

exports.getCells = async (req, res) => {
	try {
		const result = await getCells();
		res.json(result);
	} catch (error) {
		logger.error('getCells', error);
		res.sendStatus(500);
	}
};

exports.createCell = async (req, res) => {
	try {
		await createCell(req.body);
		res.status(201).json( { success: true } );
	} catch (error) {
		logger.error('createCell', error);
		res.sendStatus(500);
	}
};

exports.updateCell = async (req, res) => {
	try {
		await updateCell(req.params?.itemPN, req.body);
		res.end();
	} catch (error) {
		logger.error('updateCell', error);
		res.sendStatus(500);
	}
};

exports.deleteCell = async (req, res) => {
	try {
		await deleteCell(req.params?.itemPN);
		res.status(204).end();
	} catch (error) {
		logger.error('deleteCell', error);
		res.sendStatus(500);
	}
};

