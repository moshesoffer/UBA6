/* This is used by tests only 
   db-migrate never run down files automatically 
   and npm run migrate:rollback (db-migrate --config ./database.js down) is only running the last successful migration
*/
DROP TABLE IF EXISTS `TestRoutines`;
DROP TABLE IF EXISTS `Reports`;
DROP TABLE IF EXISTS `InstantTestResults`;
DROP TABLE IF EXISTS `RunningTests`;
DROP TABLE IF EXISTS `UBADevices`;
DROP TABLE IF EXISTS `Machines`;
DROP TABLE IF EXISTS `CellPartNumbers`;