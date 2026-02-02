const swaggerJsdoc = require('swagger-jsdoc');
const swaggerUi = require('swagger-ui-express');

const options = {
  definition: {
    openapi: '3.0.0',
    info: {
      title: 'Web Console API',
      version: '1.0.0',
      description: 'API documentation for Web Console Server',
    },
    servers: [
      {
        url: 'http://localhost:4000/web-console',
        description: 'Local server',
      },
    ],
  },
  apis: ['./routes/*.js'], // Adjust path if needed
};

const swaggerSpec = swaggerJsdoc(options);

function setupSwagger(app) {
  app.use('/docs', swaggerUi.serve, swaggerUi.setup(swaggerSpec));
}

module.exports = setupSwagger;
