@echo off

rem call C:\Program Files\Microsoft Visual Studio 14.0\VC\bin\vcvars32.bat

echo Compiler:
cl > nul
if errorlevel 1 goto clerror

if "%TCLROOT%" == ""      set TCLROOT=C:\Activetcl
if "%TCLVER%" == ""       set TCLVER=86

if not "%DEBUG%" == ""    set OPTS=%OPTS% DEBUG=%DEBUG%
if not "%MSVCRT%" == ""   set OPTS=%OPTS% MSVCRT=%MSVCRT%
if not "%UNICODE%" == ""  set OPTS=%OPTS% UNICODE=%UNICODE%
if not "%DIALOGS%" == ""  set OPTS=%OPTS% DIALOGS=%DIALOGS%
if not "%STATIC%" == ""   set OPTS=%OPTS% STATIC_BUILD=%STATIC%
set OPTS=%OPTS% USE_TCL_STUBS=1 THREADS=1 

nmake -nologo -f makefile.vc %OPTS% SDK_CFLAGS="%CFLAGS%" %*

if not errorlevel 1 goto ok
echo ERROR: MAKE RRROR
goto end

:sdkerror
echo ERROR: SDK NOT FOUND
goto end

:clerror
echo ERROR: COMPILER NOT FOUND
goto end

:ok
echo OK

:end
