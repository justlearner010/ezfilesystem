@echo off
REM ============================================================
REM   简单文件系统 —— 一键运行脚本（Windows 双击运行）
REM
REM   用法：双击本文件即可，不用敲任何命令。
REM   功能：自动找编译器 → 编译 → 跑 human 版 / AI 版 /
REM         回放官方样例 / 交互式体验 / 跑全部测试用例 / 对比两版
REM
REM   编译器不用先配好 PATH：本脚本会依次尝试
REM     1) 系统 PATH 里的 gcc / clang
REM     2) Dev-C++ / CodeBlocks / MSYS2 / MinGW-w64 / TDM-GCC 等的默认安装目录
REM     3) Visual Studio 自带的 cl.exe（通过 vswhere 定位）
REM   三条都找不到时，会给出图文安装指引，不会一闪而过。
REM ============================================================

setlocal EnableExtensions
chcp 65001 >nul 2>nul
cd /d "%~dp0"

set "HAVE_PY=no"
where python >nul 2>nul && set "HAVE_PY=yes"

REM ---- 先找编译器，找不到就直接走指引页 ----
call :FIND_CC
if not defined CC goto NOCC

:MAIN
cls
echo ============================================================
echo         简单文件系统 —— 一键运行
echo         （数据结构课程设计 · 202532110113 刘亦轩）
echo ============================================================
echo   编译器：%CC%
echo   已装 python：%HAVE_PY%   （跑自动化测试用）
echo ============================================================
echo   [1] human 版（自主实现的简化版）
echo   [2] AI 版（升级版·最终版）
echo   [3] 对比两版输出（同一输入，逐行对照）
echo   [0] 退出
echo.
set /p choice=请输入编号 [0-3]：

if "%choice%"=="1" ( set "VER=human_version" & set "VNAME=human 版（简化版）" & set "SRCLIST=src\hash.c src\fs.c src\cmd.c src\main.c" & goto SUB )
if "%choice%"=="2" ( set "VER=AI_version"    & set "VNAME=AI 版（升级版）"    & set "SRCLIST=src\hash.c src\fs.c src\cmd.c src\kmp.c src\main.c" & goto SUB )
if "%choice%"=="3" goto COMPARE
if "%choice%"=="0" exit /b 0
echo 无效输入，请重新选择。
timeout /t 1 >nul
goto MAIN

:SUB
cls
echo ============================================================
echo   已选择：%VNAME%
echo ============================================================
echo   [1] 回放官方样例（推荐，对照报告第 4 节截图）
echo   [2] 交互式体验（手动输入命令）
echo   [3] 运行全部测试用例（自动比对期望输出）
echo   [0] 返回上一级
echo.
set /p sub=请输入编号 [0-3]：

if "%sub%"=="1" goto SAMPLE
if "%sub%"=="2" goto INTERACT
if "%sub%"=="3" goto TESTS
if "%sub%"=="0" goto MAIN
echo 无效输入，请重新选择。
timeout /t 1 >nul
goto SUB

REM ============================================================
REM   找编译器：PATH → 常见安装目录 → Visual Studio 的 cl
REM ============================================================
:FIND_CC
set "CC="

where gcc >nul 2>nul && set "CC=gcc"
if not defined CC ( where clang >nul 2>nul && set "CC=clang" )
if defined CC exit /b 0

REM Dev-C++ / CodeBlocks / MSYS2 / MinGW 等自带的编译器默认都不在 PATH 里，
REM 这里按常见安装位置逐个探测，找到就临时加进 PATH（不改系统设置）
for %%D in (
    "C:\MinGW\bin"
    "C:\MinGW64\bin"
    "C:\mingw64\bin"
    "C:\msys64\mingw64\bin"
    "C:\msys64\ucrt64\bin"
    "C:\msys64\clang64\bin"
    "C:\TDM-GCC-64\bin"
    "C:\Dev-Cpp\MinGW64\bin"
    "C:\Dev-Cpp\MinGW32\bin"
    "C:\Program Files\Dev-Cpp\MinGW64\bin"
    "C:\Program Files (x86)\Dev-Cpp\MinGW64\bin"
    "C:\Program Files\Dev-Cpp\MinGW32\bin"
    "C:\Program Files (x86)\Dev-Cpp\MinGW32\bin"
    "C:\Program Files\CodeBlocks\MinGW\bin"
    "C:\Program Files (x86)\CodeBlocks\MinGW\bin"
    "C:\Program Files\CodeBlocks\MinGW32\bin"
    "C:\w64devkit\bin"
    "C:\ProgramData\chocolatey\bin"
    "%LOCALAPPDATA%\Programs\mingw-w64\bin"
    "%USERPROFILE%\scoop\shims"
    "%USERPROFILE%\scoop\apps\mingw\current\bin"
    "%USERPROFILE%\scoop\apps\gcc\current\bin"
) do (
    if not defined CC if exist "%%~D\gcc.exe" (
        set "CC=%%~D\gcc.exe"
        set "PATH=%%~D;%PATH%"
    )
)
if defined CC exit /b 0

call :FIND_MSVC
exit /b 0

:FIND_MSVC
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" exit /b 1

set "VSDIR="
for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -property installationPath 2^>nul`) do set "VSDIR=%%I"
if not defined VSDIR exit /b 1

if exist "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>nul
if not exist "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" if exist "%VSDIR%\VC\Auxiliary\Build\vcvars32.bat" call "%VSDIR%\VC\Auxiliary\Build\vcvars32.bat" >nul 2>nul

where cl >nul 2>nul && set "CC=cl"
exit /b 0

REM ============================================================
REM   没找到编译器：给指引，不要一闪而过
REM ============================================================
:NOCC
cls
echo ============================================================
echo   [X] 没有找到 C 编译器（gcc / clang / cl 都没有）
echo ============================================================
echo.
echo   本程序需要先把 .c 源码编译成 exe。以下任选一种装好，再双击本文件：
echo.
echo   【最简单】装 Dev-C++（自带编译器，一路下一步即可）
echo              https://sourceforge.net/projects/orwelldevcpp/
echo.
echo   【推荐】  装 MSYS2，装完在它终端里执行：
echo              pacman -S mingw-w64-ucrt-x86_64-gcc
echo              https://www.msys2.org/
echo.
echo   【或者】  装 MinGW-w64（安装时勾选 Add to PATH）
echo              https://www.mingw-w64.org/downloads/
echo.
echo   注意：Dev-C++ / CodeBlocks / Visual Studio 装在默认位置的话，
echo   本脚本会自动去它们的目录里找编译器；上面仍找不到，多半是
echo   装在了非常规路径，可以手动把它的 bin 目录加进系统 PATH。
echo.
echo   也可以手工编译（在 human_version 或 AI_version 目录下执行）：
echo     human 版：gcc -std=c11 -Wall -Iinclude -o ezfs.exe src\hash.c src\fs.c src\cmd.c src\main.c
echo     AI  版：gcc -std=c11 -Wall -Iinclude -o ezfs.exe src\hash.c src\fs.c src\cmd.c src\kmp.c src\main.c
echo.
pause
exit /b 1

REM ============================================================
REM   编译（gcc/clang 与 MSVC cl 两条命令）
REM ============================================================
:BUILD
echo.
echo ^>^> 编译 %VER% ...
pushd "%VER%"
if exist ezfs.exe del /q ezfs.exe >nul 2>nul

if /i "%CC%"=="cl" (
    cl /nologo /D_CRT_SECURE_NO_WARNINGS /Iinclude /Fe:ezfs.exe %SRCLIST%
    if errorlevel 1 goto BUILDFAIL
    del /q *.obj >nul 2>nul
) else (
    "%CC%" -std=c11 -Wall -Iinclude -o ezfs.exe %SRCLIST%
    if errorlevel 1 goto BUILDFAIL
)

if not exist ezfs.exe goto BUILDFAIL
echo [V] %VER% 编译成功
popd
exit /b 0

:BUILDFAIL
echo.
echo [X] %VER% 编译失败，上面是编译器给出的错误信息。
echo     请把这段信息发给作者；或改用菜单里另一个版本试试。
popd
pause
exit /b 1

REM ============================================================
REM   各功能入口
REM ============================================================
:SAMPLE
call :BUILD
if errorlevel 1 goto SUB
echo.
echo ^>^> 回放官方样例 tests\sample_input.txt
echo ============================================================
pushd "%VER%"
ezfs.exe < ..\tests\sample_input.txt
popd
echo ============================================================
echo （以上输出应与报告第 4 节的终端截图一致）
pause
goto SUB

:INTERACT
call :BUILD
if errorlevel 1 goto SUB
echo.
echo ------------------------------------------------------------
echo   已进入交互模式，直接输入命令回车即可。退出：Ctrl+Z 后回车
echo.
echo   可用命令：
echo     create_file f        create_dir d        ls
echo     cd d / cd ..         ll_pre / ll_post    find_file kw
echo     open f               write_file "内容"   read_file f
echo     close_file           delete_file f       delete_dir d
echo     rename_file old new  rename_dir old new
echo ------------------------------------------------------------
pushd "%VER%"
ezfs.exe
popd
pause
goto SUB

:TESTS
call :BUILD
if errorlevel 1 goto SUB
echo.
if /i "%CC%"=="cl" (
    echo ! 当前用的是 Visual Studio 的 cl，自动测试需要 gcc/clang。
    echo   可以先用菜单 [1] 回放官方样例、[2] 交互式体验观察程序行为。
    pause
    goto SUB
)
if "%HAVE_PY%"=="yes" (
    python tests\run_tests.py %VER%
) else (
    echo ! 未检测到 python，跳过自动化测试。
    echo   装好 python 后可运行：python tests\run_tests.py %VER%
)
pause
goto SUB

:COMPARE
set "VER=human_version"
set "SRCLIST=src\hash.c src\fs.c src\cmd.c src\main.c"
call :BUILD
if errorlevel 1 goto MAIN
set "VER=AI_version"
set "SRCLIST=src\hash.c src\fs.c src\cmd.c src\kmp.c src\main.c"
call :BUILD
if errorlevel 1 goto MAIN
echo.
pushd human_version
ezfs.exe < ..\tests\sample_input.txt > "%TEMP%\ezfs_human.out" 2>&1
popd
pushd AI_version
ezfs.exe < ..\tests\sample_input.txt > "%TEMP%\ezfs_ai.out" 2>&1
popd
fc "%TEMP%\ezfs_human.out" "%TEMP%\ezfs_ai.out" >nul
if errorlevel 1 (
    echo ! 两版输出存在差异：
    fc "%TEMP%\ezfs_human.out" "%TEMP%\ezfs_ai.out"
) else (
    echo [V] 两版对官方样例的输出完全一致（外部行为等价）。
)
echo.
echo human 版输出：
type "%TEMP%\ezfs_human.out"
pause
goto MAIN
