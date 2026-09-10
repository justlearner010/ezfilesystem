@echo off
REM ============================================================
REM   简单文件系统 —— 一键运行脚本（Windows 双击）
REM
REM   用法：双击本文件即可。
REM   功能：编译并运行 human 版 / AI 版，可回放官方样例、
REM         交互式体验、跑全部测试用例、对比两版输出。
REM
REM   注意：需要 C 编译器。若未安装 gcc，请先安装 MinGW-w64
REM         （https://www.mingw-w64.org/）并勾选加入 PATH。
REM ============================================================
setlocal EnableDelayedExpansion
cd /d "%~dp0"

REM ---- 环境检查 ----
where gcc >nul 2>nul
if errorlevel 1 (
    echo.
    echo [X] 未检测到 gcc 编译器。
    echo.
    echo     请先安装 MinGW-w64 并勾选 "Add to PATH"：
    echo     https://www.mingw-w64.org/
    echo     或安装 TDM-GCC / MSYS2 后重新双击本文件。
    echo.
    pause
    exit /b 1
)
set "HAVE_PY=no"
where python >nul 2>nul && set "HAVE_PY=yes"

:MAIN
cls
echo ============================================================
echo         简单文件系统 —— 一键运行
echo         （数据结构课程设计 · 202532110113 刘亦轩）
echo ============================================================
echo   [1] human 版（自主实现的简化版）
echo   [2] AI 版（升级版·最终版）
echo   [3] 对比两版输出（同一输入，逐行对照）
echo   [0] 退出
echo.
set /p choice=请输入编号 [0-3]：

if "%choice%"=="1" ( set "VER=human_version" & set "VNAME=human 版（简化版）" & goto SUB )
if "%choice%"=="2" ( set "VER=AI_version"    & set "VNAME=AI 版（升级版）"    & goto SUB )
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

:BUILD
echo.
echo ^>^> 编译 %VER% ...
pushd "%VER%"
if exist ezfs.exe del ezfs.exe
if "%VER%"=="AI_version" (
    gcc -std=c11 -Wall -g -Iinclude -o ezfs.exe src\hash.c src\fs.c src\cmd.c src\kmp.c src\main.c
) else (
    gcc -std=c11 -Wall -g -Iinclude -o ezfs.exe src\hash.c src\fs.c src\cmd.c src\main.c
)
if errorlevel 1 (
    echo [X] %VER% 编译失败。
    popd
    pause
    goto SUB
)
echo [V] %VER% 编译成功
popd
exit /b 0

:SAMPLE
call :BUILD
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
echo.
if "%HAVE_PY%"=="yes" (
    python tests\run_tests.py %VER%
) else (
    echo ! 未检测到 python，跳过自动化测试。
    echo   安装 python 后可运行：python tests\run_tests.py %VER%
)
pause
goto SUB

:COMPARE
set "VER=human_version"
call :BUILD
set "VER=AI_version"
call :BUILD
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
