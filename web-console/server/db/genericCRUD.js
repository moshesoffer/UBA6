const logger = require('../utils/logger');
const Joi = require('joi');
const {validateString, validateArray,validateIsDefined, validateSchema} = require('../utils/validators');
const pool = require('.');

const selectQuery = async (tableName, query, values, openedConnection) => {
    let connection;

	try {
		if (!openedConnection) connection = await pool.getConnection();
		logger.info(`[${tableName}] Executing query`);
		const realConnection = openedConnection ? openedConnection : connection;
		const [rows,] = values ? await realConnection.execute(query, values) : await realConnection.execute(query);
		return rows;
	} catch (error) {
		logger.error(`Error [${tableName}] executing`, error);
		throw error;
	} finally {
		if (connection) {
			connection.release();
		}
	}
};

const createModel = async (model, data, openedConnection) => {
    let connection;
	let query;

	/*
	We validate that the incoming data object has all required keys.
	See the properties variable.
	We insert to DB only those values, which according to those keys.
	We validate also, that these values are not empty strings.
	*/
	let updatePlaceholders = [];
	let updateValues = [];
    let preparedFields = [];
	let dataFilteredObj = {};
	try {
		for (const field of model.createProperties) {
			if ((field in data) && validateIsDefined(data[field])) {
                let val = data[field];
                if(val && typeof val === 'string'){
                    val = val.trim();
                }
                if(val !== null && val !== undefined && val !== '') {//not allowing empty strings
                    updatePlaceholders.push(`?`);
                    updateValues.push(val);
                    preparedFields.push(`\`${field}\``);
					if(model.schema && model.schema[field]){
						dataFilteredObj[field] = val;
					}
                }
			}
		}
		if(model.schema && Object.keys(model.schema).length > 0){
			validateSchema(Joi.object(model.schema), dataFilteredObj, `createModel ${model.tableName}`);
		}

        if(model.uuid){
            query = `INSERT INTO \`${model.tableName}\` (\`id\`, ${preparedFields.join(', ')}) VALUES (UUID(), ${updatePlaceholders.join(', ')});`;
        } else {
            query = `INSERT INTO \`${model.tableName}\` (${preparedFields.join(', ')}) VALUES (${updatePlaceholders.join(', ')});`;
        }
		if (!openedConnection) connection = await pool.getConnection();
		logger.info(`[${model.tableName}] Executing query: [${query}] [${updateValues}]`);
		const [result,] = openedConnection ? await openedConnection.execute(query, updateValues) : await connection.execute(query, updateValues);
		if (result?.affectedRows !== 1) {
			throw new Error(`Error creating Model ${model.tableName}.`);
		}
	} catch (error) {
		logger.error(`Error [${model.tableName}] executing [${query}] [${updateValues}]`, error);
		throw error;
	} finally {
		if (connection) {
			connection.release();
		}
	}
}

const updateModel = async (model, pkValue, data, openedConnection) => {
	let connection;
	let query;

	/*
	We insert to DB only those values, which have defined keys and are not empty strings.
	See the properties variable.
	We validate the primary key also.
	*/
	let updateFields = [];
	let updateValues = [];
	let dataFilteredObj = {};
	let schemaFilteredObj = {};

	try {
		for (const field of model.updateProperties) {
			if ((field in data) && validateIsDefined(data[field])) {
                let val = data[field];
                if(typeof val === 'string'){
                    val = val.trim();
                }
                if(val !== null && val !== undefined && val !== '') {
                    updateFields.push(`\`${field}\` = ?`);
                    updateValues.push(val);
					if(model.schema && model.schema[field]){
						dataFilteredObj[field] = val;
						schemaFilteredObj[field] = model.schema[field];
					}
                }
				
			}
		}
		if (!validateArray(updateFields)) {
			throw new Error(`No valid fields to update [${model.tableName}]`);
		}

		if (!validateString(pkValue) || !validateString(pkValue.trim())) {
			throw new Error(`Invalid pkValue [${model.tableName}]`);
		}

		if(Object.keys(schemaFilteredObj).length > 0){//in update we dont check required. we check only what is sent and update only what is sent, we dont delete fields.
			validateSchema(Joi.object(schemaFilteredObj), dataFilteredObj, `updateModel ${model.tableName}`);
		}

		updateValues.push(pkValue.trim());
		query = `UPDATE \`${model.tableName}\` SET ${updateFields.join(', ')} WHERE \`${model.pkName}\` = ?;`;

		if (!openedConnection) connection = await pool.getConnection();
		logger.info(`updateModel [${model.tableName}] Executing query: [${query}] [${updateValues}]`);
		const [result,] = openedConnection ? await openedConnection.execute(query, updateValues) : await connection.execute(query, updateValues);
		if (result?.affectedRows !== 1) {
			throw new Error(`No rows affected for pkValue [${pkValue}] [${model.tableName}]`);
		}
	} catch (error) {
		logger.error(`Error updateModel [${model.tableName}] executing [${query}] [${updateValues}]`, error);
		throw error;
	} finally {
		if (connection) {
			connection.release();
		}
	}
};

const deleteModel = async (model, pkValue, openedConnection) => {
	let connection;

	try {
		if (!validateString(pkValue) || !validateString(pkValue.trim())) {
			throw new Error(`Invalid pkValue ${model.tableName}-${pkValue}.`);
		}
		query = `DELETE FROM \`${model.tableName}\` WHERE \`${model.pkName}\` = ?;`;
		if (!openedConnection) connection = await pool.getConnection();
		logger.info(`deleteModel [${model.tableName}] Executing query: [${query}] [${pkValue}]`);
		const [result,] = openedConnection ? await openedConnection.execute(query, [pkValue.trim(),]) : await connection.execute(query, [pkValue.trim(),]);
		if (result?.affectedRows !== 1) {
			throw new Error(`No rows deleted for pkValue [${pkValue}] [${model.tableName}]`);
		}
	} catch (error) {
		logger.error(`Error deleteModel [${model.tableName}] executing  [${query}] [${pkValue}]`, error);
		throw error;
	} finally {
		if (connection) {
			connection.release();
		}
	}
}

module.exports = {
	selectQuery,
	createModel,
    updateModel,
    deleteModel
};