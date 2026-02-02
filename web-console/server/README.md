#####Some info about the project
Steps on a clean environment and how the flow should be in order to run the first test:

Add station = Machine
Add uba device - this will also add a runningTest for each channel for example AB(so running test for A and for B)
    UBADevice-RunningTest is 1 to many. RunningTests has unique constraint on ubaSN+channel
Add cell P/N - if needed (in general it’s a fixed closed list)
Add a test routine
Run test
    When starting a test then first deleting running tests on the related ubaSNs + channels
Doing actions on a running test
    if the running test is on both channels then going to find the other channel running test and do the action on it as well. not only on runningTestID
Add station = Machine
Add uba device - this will also add a runningTest for each channel for example AB(so running test for A and for B)
    UBADevice-RunningTest is 1 to many. RunningTests has unique constraint on ubaSN+channel
Add cell P/N - if needed (in general it’s a fixed closed list)
Add a test routine
Run test
    When starting a test then first deleting running tests on the related ubaSNs + channels
Doing actions on a running test
    if the running test is on both channels then going to find the other channel running test and do the action on it as well. not only on runningTestID


#####install
nvm v20
`npm install`
#####run server in prod
`npm start` 
#####run server in dev
`npm run dev` in mac
ENABLE_CORS_FOR_LOCALHOST that is in dev not sure is needed
`$env:PORT=4000; $env:ENABLE_CORS_FOR_LOCALHOST='true'; npm start;` in windows

to debug server side change 
"start": "npm run migrate && node --inspect ./bin/www",
and run npm run dev
then in vscode F1->attach to node proccess

#####run test
`npm test`

#####Links
http://localhost:4000/docs is swagger
http://localhost:4000/health check health and db connection ping
http://localhost:3000/ is the UI

#####to add new migration script be in server folder and run
npx db-migrate create v2_add_new_table --sql-file --config ./database.js
v2_add_new_table is the name of the script that you want
it will create down.sql file: should be ignore and not put anything in it (maybe also can be deleted)
also it will create a js file and its exported down function can be deleted

#####run just the file
`node ./path/to/file.js`

for example:

`node  node .\try\a.js`
