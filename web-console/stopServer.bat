REM web control batch file

@echo off
REM ========================================
REM UBA6 Frontend / DB startup
REM ========================================

REM ----------------------------------------
REM MySQL
REM ----------------------------------------

netstat -ano | findstr ":3306" >nul
if %errorlevel%==0 (
    echo MySQL is RUNNING

    echo kill mysql
    taskkill /F /IM mysqld.exe >nul 2>&1
    timeout /t 1 /nobreak >nul
) 

REM ----------------------------------------
REM Frontend
REM ----------------------------------------

netstat -ano | findstr ":3000" | findstr "LISTENING" >nul

if %errorlevel%==0 (
    echo Frontend is RUNNING
    echo Stopping existing frontend...

    for /f "tokens=5" %%P in ('netstat -ano ^| findstr ":3000" ^| findstr "LISTENING"') do (
        echo Killing frontend PID %%P
        taskkill /F /T /PID %%P
    )

    timeout /t 2 /nobreak >nul
)

REM ----------------------------------------
REM Backend
REM ----------------------------------------

netstat -ano | findstr ":4000" | findstr "LISTENING" >nul
if %errorlevel%==0 (
    echo Backend is RUNNING
    echo Stopping existing backend...

    REM Get backend Node PID
    for /f "tokens=5" %%P in ('netstat -ano ^| findstr ":4000" ^| findstr "LISTENING"') do (
        echo Killing backend PID %%P
        echo kill Backend
        taskkill /F /PID %%P >nul 2>&1
    )

    timeout /t 1 /nobreak >nul
)

REM pause

