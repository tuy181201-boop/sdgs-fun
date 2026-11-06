@echo off
setlocal EnableDelayedExpansion

:: 1. Жестко прописываем системные пути Windows
set "PATH=%SystemRoot%\System32;%SystemRoot%;%SystemRoot%\System32\Wbem;%PATH%"

:: 2. Включаем UTF-8
chcp 65001 >nul
title MCA2RAW Protocol · SDGS

:: 3. Активируем виртуальное окружение Python
call C:\venvs\mc_env\Scripts\activate.bat

:: 4. Инициализируем TUI-фреймворк
call lib\ui.bat :init

:main_menu
call lib\ui.bat :banner "MCA2RAW Protocol" "SDGS Landscape Generator"
call lib\ui.bat :menu "Главное меню" "Генерация мира (Zero-Touch)|Конвертировать MCA в RAW|Открыть GUI|Выход"
set "CHOICE=!ERRORLEVEL!"

if "!CHOICE!"=="0" (
    echo.
    set /p "SEED=Введите сид (Enter = случайный): "
    python app\server_orchestrator.py "!SEED!"
    pause
    goto main_menu
)
if "!CHOICE!"=="1" (
    echo.
    set /p "REGION_PATH=Путь к папке region: "
    set /p "SEED=Сид (имя папки): "
    set /p "OUT_PATH=Куда сохранять (Enter = D:\SDGS\data): "
    if "!OUT_PATH!"=="" set "OUT_PATH=D:\SDGS\data"
    call lib\ui.bat :progress_run "Конвертация MCA" "python -u app\mca2raw_cli.py ""!REGION_PATH!"" ""!OUT_PATH!"" ""!SEED!"""
    pause
    goto main_menu
)
if "!CHOICE!"=="2" (
    start "" pythonw app\mca2raw_gui.py
    exit /b 0
)
if "!CHOICE!"=="3" exit /b 0