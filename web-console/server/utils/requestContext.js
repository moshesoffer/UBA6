const { AsyncLocalStorage } = require('async_hooks');

const asyncLocalStorage = new AsyncLocalStorage();

module.exports = {
  asyncLocalStorage,
  getRequestId: () => {
    const store = asyncLocalStorage.getStore();
    return store?.requestId;
  }
};