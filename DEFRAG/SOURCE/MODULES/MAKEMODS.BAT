@echo off
rem
rem This batch file creates the different modules.
rem
cd chkfat
make
cd ..\infofat
make
cd ..\sortfat
make
cd ..\dfragfat
make
cd ..\modbins
make
cd ..
