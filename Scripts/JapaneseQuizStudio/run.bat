@echo off
cd /d "%~dp0"
where py >nul 2>nul
if %errorlevel%==0 (
  py app.py
) else (
  python app.py
)
