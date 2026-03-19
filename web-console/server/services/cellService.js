const logger = require('../utils/logger');
const { cellModel } = require('../models');
const { selectQuery, createModel, updateModel, deleteModel } = require('../db/genericCRUD');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

const getCells = async () => {
    return await withTimeout( selectQuery(cellModel.tableName, cellModel.selectAllQuery), AWAIT_TIMEOUT);
};

const createCell = async (cell) => {
    await withTimeout( createModel(cellModel, cell), AWAIT_TIMEOUT);
};

const updateCell = async (itemPN, cell) => {
    await withTimeout( updateModel(cellModel, itemPN, cell), AWAIT_TIMEOUT);
};

const deleteCell = async (itemPN) => {
    await withTimeout( deleteModel(cellModel, itemPN), AWAIT_TIMEOUT);
};


module.exports = {
	getCells,
	createCell,
    updateCell,
    deleteCell,

};