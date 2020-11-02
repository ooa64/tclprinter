@echo off

set PATH=C:\MinGW\bin;C:\MinGW\msys\1.0\bin
set TCLROOT=c:/tcl

mingw32-make.exe -f makefile.mingw TCLROOT=%TCLROOT% %1 %2 %3 %4 %5 %6 %7 %8 %9
