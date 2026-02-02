export const getItem = keyName => window.localStorage.getItem(keyName);

export const setItem = (keyName, item) => window.localStorage.setItem(keyName, item);

export const removeItem = keyName => window.localStorage.removeItem(keyName);
