const AWAIT_TIMEOUT = 5000;

//const withTimeout = (promise, ms) => {
//  return new Promise((resolve, reject) => {
//    const timer = setTimeout(() => {
//      reject(new Error(`Timeout after ${ms}ms`));
//    }, ms);
//
//    promise
//      .then(result => {
//        clearTimeout(timer);
//        resolve(result);
//      })
//      .catch(err => {
//        console.log('timeout, err: ' + err);
//        clearTimeout(timer);
//        reject(err);
//      });
//  });
//};

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
        console.log('timeout, err: ' + err);
        clearTimeout(timer);
        reject(err);
      });
  });
}

//function withTimeout(promiseOrFn, ms) {
//  const p = typeof promiseOrFn === 'function'
//    ? promiseOrFn()
//    : promiseOrFn;
//
//  return Promise.race([
//    p,
//    new Promise((_, reject) =>
//      setTimeout(() => reject(new Error(`Timeout after ${ms}ms`)), ms)
//    )
//  ]);
//}

module.exports = { withTimeout, AWAIT_TIMEOUT };

// old version
//function withTimeout(promise, ms) {
//  return Promise.race([
//    promise,
//    new Promise((_, reject) =>
//      setTimeout(() => reject(new Error(`Timeout after ${ms}ms`)), ms)
//    )
//  ]);
//}

