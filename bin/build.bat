@echo off

rem USAGE
rem
rem   build [filename] [-run] [-w32] [-o] [-game] [-ship] [-bun] [args...]
rem
rem ARGUMENTS
rem
rem
rem   -run 
rem     Run the output, whatever it is.
rem
rem   -o    
rem     Run with my favourite optimization flags 
rem
rem   -game
rem     Build as game.dll. Expects only 1 .cpp file
rem
rem   -w32
rem     Build with common win32 stuff. Expects only 1 .cpp file
rem   
rem   -ship
rem     Special build for shipping a game. Expects 1 .cpp file. 
rem     Will build w32_game.cpp and the given 1.cpp file.
rem     
rem   TODO: Maybe allow an output flag?
rem
rem TODO
rem - output flag to specify output?
rem - When building, place all intermiediately file elsewhere
rem

setlocal EnableDelayedExpansion

set me=%~dp0
set root_dir=%me%..
set code_dir=%root_dir%\code
set build_dir=%root_dir%\build
set ship_file="" 

set build=0
set run=0
set w32=0
set optimize=0 
set game=0
set ship=0
set filename=%~1

:label_parse_next
shift
IF "%~1"=="" (
  GOTO label_parse_end
)
IF "%~1"=="-run" (
  set run=1
  GOTO label_parse_next
)
IF "%~1"=="-w32" (
  set build=1
  set w32=1
  GOTO label_parse_next
)
IF "%~1"=="-bun" (
  set build=1
  set run=1
  GOTO label_parse_next
)
IF "%~1"=="-o" ( 
  set optimize=1
  GOTO label_parse_next
)
IF "%~1"=="-game" (
  set build=1
  set game=1
  GOTO label_parse_next
)

IF "%~1"=="-ship" (  
  set build=1
  set ship=1
  GOTO label_parse_next
)

rem ELSE there is arguments
IF DEFINED args (
  set args=%args% %~1
) ELSE (
  set args=%~1
)
GOTO label_parse_next

:label_parse_end



rem set compiler_flags=-MT -WX -W4 -wd4189 -wd4702 -wd4201 -wd4505 -wd4996 -wd4100 -Zi  -GR -EHa  -std:c++17
set compiler_flags=-std=c++17 -Wall -Wno-unused-function -Wno-parentheses -Wno-macro-redefined -Wno-deprecated-declarations -Wno-missing-braces
set linker_flags=
set output_name=%filename%.exe

echo ******************* 


if %build%==1 (

  rem Optimization
  if %optimize%==1 (
    echo Version  : Release
    set compiler_flags=-O2 -DASSERTIVE=0 -DFOOLISH=0 !compiler_flags!
  ) else (
    echo Version  : Internal 
    set compiler_flags=-DASSERTIVE=1 -DFOOLISH=1 !compiler_flags!
    set compiler_flags=!compiler_flags! -g -gcodeview
  )

  rem Different build types
  if %game%==1 (
    echo Build    : Game
    set compiler_flags=!compiler_flags! -LD
    set linker_flags=-shared 
    set output_name=game.dll
    goto end_build_type
  )

  if %w32%==1 (
    echo Build    : Win32
    set linker_flags=-luser32 -lopengl32 -lgdi32 -lwinmm -lole32 -limm32 -lshell32
    goto end_build_type
  )

  if %ship%==1 (
    echo Build    : Ship
    set compiler_flags=!compiler_flags! -DHOT_RELOAD=0 
    set linker_flags=-luser32 -lopengl32 -lgdi32 -lwinmm -lole32 -limm32 -lshell32
    set ship_file=%code_dir%\%filename%_ship.cpp 
    goto end_build_type
  )
  echo Build    : Normal 
)


:end_build_type
echo ******************* 



pushd %build_dir%

IF %ship%==1 (
  echo #include "%filename%.cpp" > %ship_file% 
  type %code_dir%\w32_game.cpp >>  %ship_file%  
  clang++ !compiler_flags! %ship_file% !linker_flags! -o %output_name%
  rem del %ship_file% 
) else (
  clang++ !compiler_flags! %code_dir%\%filename%.cpp !linker_flags! -o %output_name%
)

if %run%==1 (
  echo ******************* 
  echo Run      : %output_name%
  echo Args     : %args%
  echo ******************* 
  CALL %filename%.exe %args%
)
popd

