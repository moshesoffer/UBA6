const logger = require('../utils/logger');
const { cellModel } = require('../models');
const { selectQuery, createModel, updateModel, deleteModel } = require('../db/genericCRUD');

const getCells = async () => {
    return await selectQuery(cellModel.tableName, cellModel.selectAllQuery);
};

const createCell = async (cell) => {
    await createModel(cellModel, cell);
};

const updateCell = async (itemPN, cell) => {
    await updateModel(cellModel, itemPN, cell);
};

const deleteCell = async (itemPN) => {
    await deleteModel(cellModel, itemPN);
};


module.exports = {
	getCells,
	createCell,
    updateCell,
    deleteCell,

};