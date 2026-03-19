const AWAIT_TIMEOUT = 5000;

function withTimeout(promise, ms) {
  return new Promise((resolve, reject) => {
    const timer = setTimeout(() => {
      reject(new Error(`Timeout after ${ms}ms`));
    }, ms);

    promise
      .then(result => {
        clearTimeout(timer);
        resolve(result);
      })
      .catch(err => {
		    console.log(`timeout, err: ${err}`);
        clearTimeout(timer);
        reject(err);
      });
  });
}

module.exports = { withTimeout, AWAIT_TIMEOUT };
