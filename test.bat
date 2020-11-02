@echo off

if "%TCLROOT%" == "" if exist c:\\tcl\\lib\\tcl??.lib set TCLROOT=c:\\tcl
if "%TCLROOT%" == "" if exist c:\\tcl86\\lib\\tcl??.lib set TCLROOT=c:\\tcl86
if "%TCLROOT%" == "" if exist c:\\tcl85\\lib\\tcl??.lib set TCLROOT=c:\\tcl85
if "%TCLROOT%" == "" if exist c:\\tcl84\\lib\\tcl??.lib set TCLROOT=c:\\tcl84
if "%TCLROOT%" == "" goto error

if exist %TCLROOT%\\lib\\tcl86.lib set TCLVER=86
if exist %TCLROOT%\\lib\\tcl85.lib set TCLVER=85
if exist %TCLROOT%\\lib\\tcl84.lib set TCLVER=84
if "%TCLVER%" == "" goto error

if "%TCLVER%" == "86" set TCL_LIBRARY=%TCLROOT%\\lib\\tcl8.6
if "%TCLVER%" == "85" set TCL_LIBRARY=%TCLROOT%\\lib\\tcl8.5
if "%TCLVER%" == "84" set TCL_LIBRARY=%TCLROOT%\\lib\\tcl8.4

echo Tcl: version %TCLVER% at %TCLROOT%

set PATH=%PATH%;%TCLROOT%\\bin
set TCLSH=tclsh%TCLVER%.exe

%TCLSH% test.tcl

:error