@echo off

call "C:\Program Files\Microsoft Visual Studio 9.0\VC\bin\vcvars32.bat" 

if "%TCLROOT%" == "" if exist c:\\atcl\\bin\\tcl84.dll set TCLROOT=c:\\atcl
if "%TCLROOT%" == "" if exist c:\\tcl84\\bin\\tcl84.dll set TCLROOT=c:\\tcl84
if "%TCLROOT%" == "" if exist c:\\tcl\\bin\\tcl84.dll set TCLROOT=c:\\tcl

nmake -nologo -f makefile.vc USE_TCL_STUBS=1 STATIC_BUILD=0 %1 %2 %3 %4 %5 %6 %7 %8 %9
