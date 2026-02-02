module.exports = async () => {
    if (global.__SERVER__) {
        global.__SERVER__.close(() => {
            console.log('------------------------------------');
            console.log('Test server stopped.');
            console.log('------------------------------------');
        });
    }
    
    if (global.__MYSQL_CONTAINER__) {
      await global.__MYSQL_CONTAINER__.stop();
      console.log('------------------------------------');
      console.log('MySQL Test Container stopped.');
      console.log('------------------------------------');
    }
  };