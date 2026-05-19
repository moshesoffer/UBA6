const request = require('supertest');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

const clearMemInServer = async () => {
    const response = await  request(global.__SERVER__).get('/clear-memory');
    expect(response.status).toBe(200);
};
module.exports = {
   clearMemInServer
};