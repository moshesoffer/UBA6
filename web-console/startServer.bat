REM web control batch file

@echo off
REM Start the front-end (dev server)
echo Start DB 
start cmd /k ""C:\Program Files\MySQL\MySQL Server 8.0\bin\mysqld.exe" --defaults-file=".\my.ini" --console"
REM start cmd /k ""C:\Program Files\MySQL\MySQL Server 8.0\bin\mysqld.exe" --defaults-file="C:\work\DEV\UBA6\web-console\my.ini" --console"
REM start cmd /k ""C:\Program Files\MySQL\MySQL Server 8.0\bin\mysqld.exe" --datadir="C:\work\DEV\UBA6\web-console\data" --console" 

set VITE_API_URL=http://localhost:4000/web-console
set DB_CONFIGURATION_PASSWORD=12345678

REM Start the front-end (dev server)
echo Starting front-end...
start cmd /k "npm run dev"

REM Wait a bit to make sure the front-end starts (optional)
timeout /t 2

REM Change to server directory
cd server

REM Set environment variables and start backend
echo Starting backend server...
set PORT=4000
set ENABLE_CORS_FOR_LOCALHOST=true
npm start

pause
