REM ========================================
REM web control batch file
REM ========================================
REM Usage:
REM   startServer.bat c   -> close CMD when command finishes
REM   startServer.bat b   -> run in the background/no new window
REM   startServer.bat k   -> keep CMD open

if /I "%~1"=="d" (
    set "START_OPTION=debug"
    set "CMD_OPTION=/k"
) else if /I "%~1"=="k" (
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
start "" cmd %CMD_OPTION% ".\stopServer.bat"

REM ----------------------------------------
REM MySQL
REM ----------------------------------------

REM netstat -ano | findstr ":3306" >nul
REM if %errorlevel%==0 (
REM     echo MySQL is RUNNING
REM 
REM     taskkill /F /IM mysqld.exe >nul 2>&1
REM     timeout /t 1 /nobreak >nul
REM ) 

echo MySQL is NOT running 
echo Start DB
if "%START_OPTION%" == "debug" (
    echo start MYSQL debug
    start "" cmd %CMD_OPTION% ""C:\Program Files\MySQL\MySQL Server 8.0\bin\mysqld.exe" --defaults-file=".\my.ini" --console"
) else (
    powershell -NoProfile -WindowStyle Hidden -Command ^
        "Start-Process -FilePath 'C:\Program Files\MySQL\MySQL Server 8.0\bin\mysqld.exe' -ArgumentList '--defaults-file=\"%~dp0my.ini\"' -WindowStyle Hidden"
)

REM ----------------------------------------
REM Environment
REM ----------------------------------------
echo Setting environment...

set VITE_API_URL=http://localhost:4000/web-console
set DB_CONFIGURATION_PASSWORD=12345678

REM ----------------------------------------
REM Frontend
REM ----------------------------------------

REM netstat -ano | findstr ":5173" | findstr "LISTENING" >nul
REM if %errorlevel%==0 (
REM     echo Frontend is RUNNING
REM     echo Stopping existing frontend...
REM 
REM     for /f "tokens=5" %%P in ('netstat -ano ^| findstr ":5173" ^| findstr "LISTENING"') do (
REM         echo Killing frontend PID %%P
REM         taskkill /F /T /PID %%P
REM     )
REM 
REM     timeout /t 2 /nobreak >nul
REM )

echo Frontend is NOT running
echo Starting front-end...
if "%START_OPTION%" == "debug" (
    echo nmp run debug
    start ""  cmd %CMD_OPTION% "npm run dev"
) else (
    powershell -NoProfile -WindowStyle Hidden -Command ^
        "Start-Process -FilePath 'cmd.exe' -ArgumentList '/c','npm run dev' -WindowStyle Hidden"
)
REM Wait a bit to make sure the front-end starts (optional)
timeout /t 5


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

REM netstat -ano | findstr ":4000" >nul
REM if %errorlevel%==0 (
REM     echo Backend is RUNNING
REM     echo Stopping existing backend...
REM 
REM     for /f "tokens=5" %%P in ('netstat -ano ^| findstr ":4000" ^| findstr "LISTENING"') do (
REM         tasklist /FI "PID eq %%P" | findstr "%%P" >nul
REM 
REM         if not errorlevel 1 (
REM             echo Killing backend PID %%P
REM             taskkill /F /PID %%P >nul 2>&1
REM         )
REM     )
REM 
REM     timeout /t 1 /nobreak >nul
REM ) 

echo Backend is NOT running
echo Starting backend server...
set PORT=4000
set ENABLE_CORS_FOR_LOCALHOST=true
npm start

if "%START_OPTION%" == "debug" (
    pause
)