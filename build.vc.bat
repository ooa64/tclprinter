@echo off

if "%VCVARS%" == "" if exist "C:\Program Files\Microsoft Developer Studio\vc98\bin\vcvars32.bat" set VCVARS=C:\Program Files\Microsoft Developer Studio\vc98\bin\vcvars32.bat
if "%VCVARS%" == "" if exist "C:\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32.bat" set VCVARS=C:\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32.bat
if "%VCVARS%" == "" if exist "C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\bin\vcvars32.bat" set VCVARS=C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\bin\vcvars32.bat
if "%VCVARS%" == "" if exist "C:\Program Files\Microsoft Visual Studio 9.0\VC\bin\vcvars32.bat" set VCVARS=C:\Program Files\Microsoft Visual Studio 9.0\VC\bin\vcvars32.bat
if "%VCVARS%" == "" goto error

call "%VCVARS%"
echo Compiler:
cl > nul
if errorlevel 1 goto error

if "%TCLROOT%" == "" if exist c:\\tcl\\lib\\tcl8*.lib set TCLROOT=c:\\tcl
if "%TCLROOT%" == "" if exist c:\\tcl86\\lib\\tcl8*.lib set TCLROOT=c:\\tcl86
if "%TCLROOT%" == "" if exist c:\\tcl85\\lib\\tcl8*.lib set TCLROOT=c:\\tcl85
if "%TCLROOT%" == "" if exist c:\\tcl84\\lib\\tcl8*.lib set TCLROOT=c:\\tcl84
if "%TCLROOT%" == "" goto error

if exist %TCLROOT%\\lib\\tcl86*.lib set TCLVER=86
if exist %TCLROOT%\\lib\\tcl85*.lib set TCLVER=85
if exist %TCLROOT%\\lib\\tcl84*.lib set TCLVER=84
if "%TCLVER%" == "" goto error

echo Tcl: version %TCLVER% at %TCLROOT%

nmake -nologo -f makefile.vc USE_TCL_STUBS=1 DIALOGS=1 %1 %2 %3 %4 %5 %6 %7 %8 %9
if errorlevel 1 goto error
goto end

:error
echo ERROR
exit

:end
echo OK