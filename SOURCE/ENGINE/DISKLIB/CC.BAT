@echo off

REM You will have to define OS if Windows 95/98

if "%OS%"=="Windows_NT" goto nt
if "%OS%"=="Windows_95" goto w95
if "%OS%"=="Windows_98" goto w98

:dos
if not "%DJGPP%"==""  gcc -g -Wall %2 %3 %4 %5 %1.c dos\disklib.a -o %1.exe
if not "%WATCOM%"=="" wcl /ml /d2 /w4 %2 %3 %4 %5 %1.c dos\disklib.lib
if not "%MSVC%"=="" cl /AL /Zi /W4 %2 %3 %4 %5 %1.c dos\disklib.lib
goto end

:w95
:w98
if not "%WATCOM%"=="" wcl386 /d2 /w4 %2 %3 %4 %5 %1.c win32\disklib.lib
if not "%MSDEVDIR%"=="" cl /Zi /W4 %2 %3 %4 %5 %1.c win32\disklib.lib
if not "%MSVC%"=="" cl /Zi /W4 %2 %3 %4 %5 %1.c win32\disklib.lib
goto end

:nt
if not "%WATCOM%"=="" wcl386 /w4 /D_WINNT %2 %3 %4 %5 %1.c win32\disklib.lib
:if not "%WATCOM%"=="" wcl386 /d2 /w4 %2 %3 %4 %5 %1.c win32\disklib.lib
goto end

:end
