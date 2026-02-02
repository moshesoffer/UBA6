const { MySqlContainer } = require('@testcontainers/mysql');

module.exports = async () => {
    const container = await new MySqlContainer().start();
    //console.log('container', container);

    // Share the database connection details with the test environment
    global.__MYSQL_CONTAINER__ = container;
    global.__MYSQL_CONFIG__ = {
        host: container.getHost(),
        port: container.getMappedPort(3306),
        user: container.getUsername(),
        password: container.getUserPassword(),
        database: container.getDatabase(),
    };

    process.env.DB_CONFIGURATION_PORT = container.getMappedPort(3306);
    process.env.DB_CONFIGURATION_DB_NAME = container.getDatabase();
    process.env.DB_CONFIGURATION_USER = container.getUsername();
    process.env.DB_CONFIGURATION_HOST = container.getHost();
    process.env.DB_CONFIGURATION_PASSWORD = container.getUserPassword();
    process.env.TEST_ENV = true;// Set the environment variable to indicate test mode, currently isnt in use
    
    const server = require('../bin/www');
    global.__SERVER__ = server;
    console.log('------------------------------------');
    console.log('MySQL Test Container started.');
    console.log('------------------------------------');
};