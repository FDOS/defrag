@echo off
rem
rem This batch file compiles the current version of defrag.
rem
echo Compiling fat transformation engine.
cd engine\lib
call makefte
cd ..\..
echo Compiling environment checking routines.
cd environ
make
cd ..
echo Compiling miscellanous routines.
cd misc
make
cd ..
echo Compiling modules
cd modules
call makemods
cd ..
echo Compiling Microsoft look alike interface.
cd msdefint
make
cd ..
echo Compiling command line interface.
cd cmdefint
make
cd ..
echo Compiling defrag start up code.
cd main
make
cd ..
echo Compiling module gate.
cd modlgate
make
cd ..
echo Putting everything together
call ..\bin\delete defrag.lib
tlib defrag.lib + environ\environ.lib
tlib defrag.lib + cmdefint\cmdefint.lib
tlib defrag.lib + msdefint\msdefint.lib
tlib defrag.lib + modules\modbins\modules.lib
tlib defrag.lib + misc\misc.lib
tlib defrag.lib + modlgate\modlgate.lib
tlib defrag.lib + engine\lib\fte.lib
echo Creating defrag
tcc -ml -M -edefrag.exe main\defrag.obj defrag.lib
copy defrag.exe ..\bin


