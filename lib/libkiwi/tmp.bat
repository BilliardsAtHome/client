@echo off
setlocal enabledelayedexpansion

set "search_dir=."  REM Change this to the directory where you want to start the renaming process

for /r "%search_dir%" %%F in (*.hpp) do (
    set "old_name=%%~nxF"
    set "new_name=%%~nF.h"
    ren "%%F" "!new_name!"
    echo Renamed "%%F" to "!new_name!"
)

endlocal