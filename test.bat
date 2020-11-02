@echo off

if "%TCLROOT%" == "" if exist c:\\atcl\\bin\\tcl84.dll set TCLROOT=c:\\atcl
if "%TCLROOT%" == "" if exist c:\\tcl84\\bin\\tcl84.dll set TCLROOT=c:\\tcl84
if "%TCLROOT%" == "" if exist c:\\tcl\\bin\\tcl84.dll set TCLROOT=c:\\tcl

set TCL_LIBRARY=%TCLROOT%\\lib\\tcl8.4
set PATH=%PATH%;%TCLROOT%\\bin

if not exist tclprintersh10.exe goto nosh
tclprintersh10.exe test.tcl
goto end
:nosh

if not exist tclprintersh10sx.exe goto noshsx
tclprintersh10sx.exe test.tcl
goto end
:noshsx

%TCLROOT%\bin\tclsh84.exe test.tcl

:end