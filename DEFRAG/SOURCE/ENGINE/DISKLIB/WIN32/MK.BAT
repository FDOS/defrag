@echo off

if "%OS%"=="Windows_NT" goto nt

rem I have three compilers (gosh it's nice). To keep my DOS
rem environment uncluttered I use only one at a time. Which
rem one is current is determined by the environment variables
rem as you can see below. This batch file makes it easy to
rem compile and test d_disk without too much typing...

if not "%WATCOM%"=="" wmake -f makefile.wcl %1 %2 %3 %4 %5
if not "%DJGPP%"==""   make -fmakefile.gcc %1 %2 %3 %4 %5
if not "%MSVC%"==""   nmake -f makefile.msc %1 %2 %3 %4 %5
if not "%MSDEVDIR%"=="" nmake -f makefile.msc %1 %2 %3 %4 %5
goto end

:nt
if not "%WATCOM%"=="" wmake -f makefile.wnt %1 %2 %3 %4 %5
if not "%DJGPP%"==""   make -fmakefile.gnt %1 %2 %3 %4 %5
if not "%MSVC%"==""   nmake -f makefile.mnt %1 %2 %3 %4 %5
if not "%MSDEVDIR%"=="" nmake -f makefile.mnt %1 %2 %3 %4 %5

:end
