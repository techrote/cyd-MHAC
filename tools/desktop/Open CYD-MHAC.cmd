@echo off
setlocal
cd /d "%~dp0"
set "SDL_VIDEODRIVER="
set "SDL_AUDIODRIVER="
mhac-desktop.exe
set "rc=%errorlevel%"
if not "%rc%"=="0" (
  echo CYD-MHAC exited with error %rc%.
  pause
)
exit /b %rc%
