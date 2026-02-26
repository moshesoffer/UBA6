const request = require('supertest');
const runSchema = require('../prepareDb');
const { APIS } = require('../../utils/constants');
const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

describe('Authentication API Tests', () => {

    beforeAll(async () => {
        await withTimeout(runSchema(), AWAIT_TIMEOUT);
    });

    afterAll(async () => {
        console.log('Auth Test suite finished');
        //delay - waiting for winston server logs to finish
        //await new Promise(resolve => setTimeout(resolve, 500));
    });

    test('login - success', async () => {
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.apiInitials + '/login')
            .send({
                username: 'amicell',
                password: '1q!QazAZ'
            }), AWAIT_TIMEOUT);
        
        expect(response.status).toBe(200);
        expect(response.body).toHaveProperty('name');
        expect(response.body.name).toBe('Natasha Cherkover');
    });

    test('login - invalid credentials', async () => {
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.apiInitials + '/login')
            .send({
                username: 'wrong',
                password: 'wrong'
            }), AWAIT_TIMEOUT);
        
        expect(response.status).toBe(400);
        expect(response.text).toBe('Invalid credentials');
    });

    test('login - missing username', async () => {
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.apiInitials + '/login')
            .send({
                password: '1q!QazAZ'
            }), AWAIT_TIMEOUT);
        
        expect(response.status).toBe(400);
    });

    test('login - missing password', async () => {
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.apiInitials + '/login')
            .send({
                username: 'amicell'
            }), AWAIT_TIMEOUT);
        
        expect(response.status).toBe(400);
    });

    test('login - empty body', async () => {
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.apiInitials + '/login')
            .send({}), AWAIT_TIMEOUT);
        
        expect(response.status).toBe(400);
    });

    test('logout - success', async () => {
        const response = await withTimeout(request(global.__SERVER__)
            .post(APIS.apiInitials + '/logout'), AWAIT_TIMEOUT);
        
        expect(response.status).toBe(200);
    });
});