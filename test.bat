@echo off

if "%TCLROOT%" == "" set TCLROOT=C:\Activetcl
if "%TCLVER%" == ""  set TCLVER=86

set TCLSH=tclsh.exe
if exist %TCLROOT%\bin\tclsh%TCLVER%.exe set TCLSH=tclsh%TCLVER%.exe
if exist %TCLROOT%\bin\tclsh%TCLVER%t.exe set TCLSH=tclsh%TCLVER%t.exe
if exist %TCLROOT%\bin\tclsh%TCLVER%tg.exe set TCLSH=tclsh%TCLVER%tg.exe

set TCL_LIBRARY=%TCLROOT%\lib\tcl%TCLVER:~0,1%.%TCLVER:~1%
set PATH=%PATH%;%TCLROOT%\bin

echo Tcl: version %TCLVER% at %TCLROOT%, shell %TCLSH%, library %TCL_LIBRARY%

%TCLSH% test.tcl %* 

