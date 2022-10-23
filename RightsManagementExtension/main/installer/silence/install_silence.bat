@ECHO OFF
rem
rem  /$$   /$$                    /$$    /$$               /$$                      /$$$$$$
rem | $$$ | $$                   | $$   | $$              | $$                     |_  $$_/
rem | $$$$| $$ /$$$$$$ /$$   /$$/$$$$$$ | $$       /$$$$$$| $$$$$$$  /$$$$$$$        | $$  /$$$$$$$  /$$$$$$$
rem | $$ $$ $$/$$__  $|  $$ /$$|_  $$_/ | $$      |____  $| $$__  $$/$$_____/        | $$ | $$__  $$/$$_____/
rem | $$  $$$| $$$$$$$$\  $$$$/  | $$   | $$       /$$$$$$| $$  \ $|  $$$$$$         | $$ | $$  \ $| $$
rem | $$\  $$| $$_____/ >$$  $$  | $$ /$| $$      /$$__  $| $$  | $$\____  $$        | $$ | $$  | $| $$
rem | $$ \  $|  $$$$$$$/$$/\  $$ |  $$$$| $$$$$$$|  $$$$$$| $$$$$$$//$$$$$$$/       /$$$$$| $$  | $|  $$$$$$$/$$
rem |__/  \__/\_______|__/  \__/  \___/ |________/\_______|_______/|_______/       |______|__/  |__/\_______|__/
rem
rem
rem Teamcenter Rights Management eXtension Installer GUI Launcher for Windows Systems
rem version : Teamcenter RMX 5.0
rem 
rem Copyright 2019, Nextlabs Inc.
rem All rights reserved - Do Not Redistribute
rem 
rem Author:: Phan Anh Tuan
rem 


TITLE Teamcenter Rights Management eXtension
echo Starting Silence Installer...
echo.

pushd %~dp0
for %%i in ("%cd0%..") do SET "START_DIR=%%~fi"
echo Current working directory: %START_DIR%

call "%START_DIR%\engine\chef\bin\chef-client.bat" --config "%START_DIR%\silence\config.rb" -o RMX_installation::main

echo Removing and Cleaning files
IF EXIST "%TEMP%\chef" rmdir "%TEMP%\chef" /S /Q

echo Installer execution completed
popd

pause
