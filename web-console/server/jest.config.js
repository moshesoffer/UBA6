module.exports = {
  testMatch: ['<rootDir>/tests/**/*.test.js'],
  testEnvironment: 'node',
  roots: ['<rootDir>/tests'],
  globalSetup: '<rootDir>/tests/jest.setup.js',
  globalTeardown: '<rootDir>/tests/jest.teardown.js',
  testTimeout: 30000,
  maxWorkers: 1
};
