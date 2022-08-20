@echo off

SET me=%~dp0

SET RootDir=%me%..
SET CodeDir=%RootDir%\code
SET BuildDir=%RootDir%\build
SET Opt=%1%

call bin\setup_cl_x64.bat
IF NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (EXIT /b)


SET CommonCompilerFlags=-MT -WX -W4 -wd4533 -wd4189 -wd4702 -wd4201 -wd4505 -wd4996 -wd4100 -Zi -Oi -GR- -EHa -Gm- -std:c++17
SET CommonCompilerFlags=-DINTERNAL=1  %CommonCompilerFlags%

pushd %BuildDir%

cl %CommonCompilerFlags% %CodeDir%\sui.cpp

call sui.exe

popd
