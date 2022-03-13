@echo off

SET me=%~dp0

SET RootDir=%me%..
SET CodeDir=%RootDir%\code
SET BuildDir=%RootDir%\build
SET Opt=%1

if not exist %BuildDir% mkdir %BuildDir%

SET CommonCompilerFlags=-MT -WX -W4 -wd4116 -wd4189 -wd4702 -wd4201 -wd4505 -wd4996 -wd4100 -Zi -GR -EHa -std:c++17
SET CommonCompilerFlags=-DSLOW -DINTERNAL  %CommonCompilerFlags%

SET CommonLinkerFlags=-incremental:no -opt:ref

SET CommonLinkerFlags=user32.lib opengl32.lib gdi32.lib  %CommonLinkerFlags%

pushd %BuildDir%
cl %CommonCompilerFlags% %CodeDir%\win_renderer_opengl.cxx -LD -link %CommonLinkerFlags% /OUT:renderer.dll

popd
