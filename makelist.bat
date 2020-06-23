set BINDIR=bin
set OBJDIR=obj

REM set INCDIRS=include "%KINECTSDK10_DIR%inc" include\SDL include\imgui include\imgui\misc\cpp
REM set LIBDIR=lib
REM set LIBS=SDL2main.lib SDL2.lib
REM Global libs
REM set GLIBS="%KINECTSDK10_DIR%lib\amd64\kinect10.lib"
REM set SOURCE=src\main.cpp

set INCDIRS=include "%KINECTSDK10_DIR%inc"
set SOURCE=src\nogui.cpp

set BINARY=kinetic.exe

set CXX=cl /nologo /std:c++17 /D_CRT_SECURE_NO_WARNINGS /MT /EHsc

if "%mode%"=="debug" goto debug
if "%mode%"=="release" goto release
if "%mode%"=="nolog" goto nolog
goto :eof

:debug
set COMPFLAGS=/Zi /bigobj /O2
set LINKFLAGS=/SUBSYSTEM:CONSOLE /DEBUG /LIBPATH:"%KINECTSDK10_DIR%lib\amd64"
goto :eof

:release
set COMPFLAGS=/DNDEBUG /bigobj /O2
set LINKFLAGS=/SUBSYSTEM:CONSOLE /LIBPATH:"%KINECTSDK10_DIR%lib\amd64"
goto :eof

:nolog
set COMPFLAGS=/DNOLOG /DNDEBUG /bigobj /O2
set LINKFLAGS=/SUBSYSTEM:CONSOLE /LIBPATH:"%KINECTSDK10_DIR%lib\amd64"
