REM ========================================
REM web control batch file
REM ========================================
REM Usage:
REM   startServer.bat c   -> close CMD when command finishes
REM   startServer.bat b   -> run in the background/no new window
REM   startServer.bat k   -> keep CMD open

if /I "%~1"=="k" (
    set "START_OPTION="
    set "CMD_OPTION=/k"
) else if /I "%~1"=="b" (
    set "START_OPTION=/b"
    set "CMD_OPTION=/c"
) else (
    set "START_OPTION="
    set "CMD_OPTION=/c"
)

@echo off
echo CMD mode: %CMD_OPTION% %START_OPTION%

REM ========================================
REM UBA6 Frontend / DB startup
REM ========================================
start "%START_OPTION%" cmd %CMD_OPTION% ".\stopServer.bat"

REM ----------------------------------------
REM MySQL
REM ----------------------------------------

netstat -ano | findstr ":3306" >nul
if %errorlevel%==0 (
    echo MySQL is RUNNING

    taskkill /F /IM mysqld.exe >nul 2>&1
    timeout /t 1 /nobreak >nul
) 
echo MySQL is NOT running 
echo Start DB 
start "%START_OPTION%" cmd %CMD_OPTION% ""C:\Program Files\MySQL\MySQL Server 8.0\bin\mysqld.exe" --defaults-file=".\my.ini" --console"

REM ----------------------------------------
REM Environment
REM ----------------------------------------
echo Setting environment...

set VITE_API_URL=http://localhost:4000/web-console
set DB_CONFIGURATION_PASSWORD=12345678

REM ----------------------------------------
REM Frontend
REM ----------------------------------------

netstat -ano | findstr ":5173" | findstr "LISTENING" >nul
if %errorlevel%==0 (
    echo Frontend is RUNNING
    echo Stopping existing frontend...

    for /f "tokens=5" %%P in ('netstat -ano ^| findstr ":5173" ^| findstr "LISTENING"') do (
        echo Killing frontend PID %%P
        taskkill /F /T /PID %%P
    )

    timeout /t 2 /nobreak >nul
)
echo Frontend is NOT running
echo Starting front-end...
start "%START_OPTION%"  cmd %CMD_OPTION% "npm run dev"
REM Wait a bit to make sure the front-end starts (optional)
timeout /t 2


REM run service
REM @echo off
REM taskkill /IM UBAService.exe /F 2>nul
REM start "" /B "C:\work\DEV\UBA6\uba6_windwos_tools\UBAService\bin\Debug\net8.0\UBAService.exe"
REM exit

REM Change to server directory
cd server

REM ----------------------------------------
REM Backend
REM ----------------------------------------

netstat -ano | findstr ":4000" >nul
if %errorlevel%==0 (
    echo Backend is RUNNING
    echo Stopping existing backend...

    for /f "tokens=5" %%P in ('netstat -ano ^| findstr ":4000" ^| findstr "LISTENING"') do (
        tasklist /FI "PID eq %%P" | findstr "%%P" >nul

        if not errorlevel 1 (
            echo Killing backend PID %%P
            taskkill /F /PID %%P >nul 2>&1
        )
    )

    timeout /t 1 /nobreak >nul
) 
echo Backend is NOT running
echo Starting backend server...
set PORT=4000
set ENABLE_CORS_FOR_LOCALHOST=true
npm start

pause
