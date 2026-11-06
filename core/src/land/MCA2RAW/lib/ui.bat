@echo off
:: ============================================================================
::  lib\ui.bat — TUI framework для Windows 10/11
::  Требует: chcp 65001 (вызывается в родительском скрипте)
::  Использование: call lib\ui.bat :init
:: ============================================================================
if "%~1"=="" goto :eof
goto %~1

:: ---------------------------------------------------------------------------
:init
:: Пуленепробиваемый способ получить ESC-символ
for /F "tokens=1,2 delims=#" %%a in ('"prompt #$H#$E# & echo on & for %%b in (1) do rem"') do (
  set "ESC=%%b"
)
if not defined ESC set "ESC="

:: Палитра цветов (ANSI)
set "R=%ESC%[0m"   & set "B=%ESC%[1m"   & set "DIM=%ESC%[2m"
set "IT=%ESC%[3m"  & set "UL=%ESC%[4m"  & set "REV=%ESC%[7m"
set "RED=%ESC%[31m" & set "GRN=%ESC%[32m" & set "YEL=%ESC%[33m"
set "BLU=%ESC%[34m" & set "MAG=%ESC%[35m" & set "CYN=%ESC%[36m" & set "WHT=%ESC%[37m"
set "BRED=%ESC%[91m" & set "BGRN=%ESC%[92m" & set "BYEL=%ESC%[93m"
set "BBLU=%ESC%[94m" & set "BMAG=%ESC%[95m" & set "BCYN=%ESC%[96m" & set "BWHT=%ESC%[97m"

:: Управляющие последовательности
set "CLR=%ESC%[2K"
set "HOME=%ESC%[1G"
set "SAVE=%ESC%[s"
set "REST=%ESC%[u"
set "CLRDOWN=%ESC%[J"

:: Определение ширины терминала (через tput из MSYS2, если есть)
set "TW=80"
if exist "C:\msys64\usr\bin\tput.exe" (
    for /f %%w in ('"C:\msys64\usr\bin\tput.exe" cols 2^>nul') do set "TW=%%w"
)

:: Проверка наличия быстрого ридера клавиш (key.exe)
set "KEYEXE=%~dp0key.exe"
if not exist "%KEYEXE%" if exist "%~dp0key.c" if exist "C:\msys64\ucrt64\bin\gcc.exe" (
    "C:\msys64\ucrt64\bin\gcc.exe" -O2 -o "%KEYEXE%" "%~dp0key.c" 2>nul
)
goto :eof

:: ---------------------------------------------------------------------------
:hr
echo   %DIM%------------------------------------------------------%R%
goto :eof

:hr2
echo   %DIM%======================================================%R%
goto :eof

:: ---------------------------------------------------------------------------
:banner
:: :banner "Title" "Subtitle"
if not defined ESC call :init
echo.
echo   %BMAG%%B%г======================================================¬%R%
echo   %BMAG%%B%¦%R%   %BCYN%%B%%~2%R%
if not "%~3"=="" echo   %BMAG%%B%¦%R%   %DIM%%~3%R%
echo   %BMAG%%B%L======================================================-%R%
echo.
goto :eof

:: ---------------------------------------------------------------------------
:ok   & echo   %BGRN%[ OK ]%R%  %~2 & goto :eof
:warn & echo   %BYEL%[WARN]%R%  %~2 & goto :eof
:err  & echo   %BRED%[FAIL]%R%  %~2 & goto :eof
:info & echo   %BCYN%[INFO]%R%  %~2 & goto :eof
:step & echo   %BBLU%[%~2/%~3]%R%  %~4 & goto :eof

:: ---------------------------------------------------------------------------
:box
:: :box "Title" "Line1" "Line2" ...
if not defined ESC call :init
setlocal EnableDelayedExpansion
set "title=%~2"
set /a inner=TW-4
set "dash="
for /l %%i in (1,1,!inner!) do set "dash=!dash!="
echo   %BMAG%г!dash!¬%R%
echo   %BMAG%¦%R% %B%!title!%R%
echo   %BMAG%¦!dash!¦%R%
shift & shift
:box_body
if "%~1"=="" goto box_end
set "line=%~1                                                                                "
set "line=!line:~0,%inner%!"
echo   %BMAG%¦%R% !line! %BMAG%¦%R%
shift
goto box_body
:box_end
echo   %BMAG%L!dash!-%R%
endlocal & goto :eof

:: ---------------------------------------------------------------------------
:progress
:: :progress "msg" 40 — отрисовать один кадр полосы
if not defined ESC call :init
setlocal EnableDelayedExpansion
call :_pbar %~3 "%~2"
endlocal & goto :eof

:: ---------------------------------------------------------------------------
:progress_run
:: :progress_run "msg" "команда" — команда печатает 0-100 в stdout
if not defined ESC call :init
setlocal EnableDelayedExpansion
set "msg=%~2" & set "cmd=%~3"
set "logfile=%TEMP%\sdgs_prog_%RANDOM%.log"
if exist "!logfile!" del "!logfile!"
echo   %DIM%!msg!%R%
set "last=-1"
for /f "delims=" %%l in ('!cmd! 2^>^&1') do (
    set "pct=%%l" & set "isnum=1"
    for /f "delims=0123456789" %%z in ("!pct!") do set "isnum=0"
    if "!isnum!"=="1" (
        if not "!pct!"=="!last!" ( set "last=!pct!" & call :_pbar !pct! "" )
    ) else ( >> "!logfile!" echo(!pct! )
)
<nul set /p "=!CLR!"
echo.
if exist "!logfile!" ( echo   %DIM%-- вывод --%R% & type "!logfile!" & del "!logfile!" )
echo   %BGRN%[ DONE ]%R%
endlocal & goto :eof

:: ---------------------------------------------------------------------------
:spinner_run
:: :spinner_run "msg" "команда" — фоновый запуск + анимация спиннера
if not defined ESC call :init
setlocal EnableDelayedExpansion
set "msg=%~2" & set "cmd=%~3"
set "sentinel=%TEMP%\sdgs_spin_%RANDOM%_%RANDOM%.flag"
set "helper=%TEMP%\sdgs_spin_%RANDOM%.bat"
set "logfile=%TEMP%\sdgs_spin_%RANDOM%.log"
if exist "!sentinel!" del "!sentinel!"
> "!helper!" echo @echo off
>> "!helper!" echo ( !cmd! ) ^> "!logfile!" 2^>^&1
>> "!helper!" echo type nul ^> "!sentinel!"
start "" /b cmd /c ""!helper!""
set "frames=|/-\" & set /a i=0
:spin_wait
if exist "!sentinel!" goto spin_done
set /a i=i%%4
for %%c in (!i!) do set "ch=!frames:~%%c,1!"
<nul set /p "=!CLR%   %DIM%!msg!%R% !ch!!HOME!"
set /a i+=1
ping -n 2 127.0.0.1 >nul
goto spin_wait
:spin_done
<nul set /p "=!CLR!"
echo   %BGRN%[ DONE ]%R%  !msg!
if exist "!logfile!" ( echo   %DIM%-- вывод --%R% & type "!logfile!" & del "!logfile!" )
del "!sentinel!" 2>nul & del "!helper!" 2>nul
endlocal & goto :eof

:: ---------------------------------------------------------------------------
:menu
:: :menu "Title" "Item1|Item2|Item3"  > ERRORLEVEL = индекс 0..N-1, или -1 (Esc)
if not defined ESC call :init
setlocal EnableDelayedExpansion
set "title=%~2" & set "rest=%~3" & set "count=0"
:_menu_split
for /f "tokens=1* delims=|" %%a in ("!rest!") do (
    set "item[!count!]=%%a"
    set /a count+=1
    set "rest=%%b"
    if not "%%b"=="" goto _menu_split
)
set /a maxidx=count-1
if !maxidx! lss 0 ( endlocal & exit /b -1 )
set "sel=0"
echo.
echo   %B%%BCYN%!title!%R%
echo.
<nul set /p "=!SAVE!"
call :_menu_draw
:_menu_wait
if exist "%KEYEXE%" (
    "%KEYEXE%" & set "key=!ERRORLEVEL!"
) else (
    for /f %%k in ('powershell -NoProfile -Command "[int][Console]::ReadKey($true).Key"') do set "key=%%k"
)
if "!key!"=="72" ( set /a sel=sel-1 & if !sel! lss 0 set "sel=!maxidx!" & <nul set /p "=!REST!!CLRDOWN!" & call :_menu_draw & goto _menu_wait )
if "!key!"=="80" ( set /a sel=sel+1 & if !sel! gtr !maxidx! set "sel=0" & <nul set /p "=!REST!!CLRDOWN!" & call :_menu_draw & goto _menu_wait )
if "!key!"=="13" goto _menu_done
if "!key!"=="27" ( echo. & endlocal & exit /b -1 )
goto _menu_wait
:_menu_done
echo.
set "result=!sel!"
endlocal & exit /b %result%

:_menu_draw
set "i=0"
:_menu_draw_loop
if !i! gtr !maxidx! goto :eof
set /a display_idx=i+1
if !i!==!sel! (
    echo   %BCYN%%B%  ? !display_idx!. !item[%i%]!%R%
) else (
    echo        !display_idx!. !item[%i%]!
)
set /a i+=1
goto _menu_draw_loop

:: ---------------------------------------------------------------------------
:table
:: :table "H1|H2|H3" "R1C1|R1C2|R1C3" ... — таблица с адаптивной шириной
if not defined ESC call :init
setlocal EnableDelayedExpansion
set "headers=%~2" & shift & shift
set "h_rest=!headers!" & set "col_count=0"
:_table_split_h
for /f "tokens=1* delims=|" %%a in ("!h_rest!") do (
    set "hdr[!col_count!]=%%a"
    set /a col_count+=1
    set "h_rest=%%b"
    if not "%%b"=="" goto _table_split_h
)
set /a colw=(TW-8)/col_count
if !colw! lss 8 set "colw=8"
set "blank=                                                                                "
set "hdr_line="
set /a last=col_count-1
for /l %%i in (0,1,!last!) do (
    set "h=!hdr[%%i]!!blank!"
    set "h=!h:~0,%colw%!"
    set "hdr_line=!hdr_line! %BMAG%!h!%R% ¦"
)
echo !hdr_line!
set /a dashes=col_count*(colw+3)
set "hr=" & for /l %%i in (1,1,!dashes!) do set "hr=!hr!-"
echo   %DIM%!hr!%R%
:_table_rows
if "%~1"=="" goto _table_end
set "r_rest=%~1"
set "row_line="
:_table_split_r
for /f "tokens=1* delims=|" %%a in ("!r_rest!") do (
    set "cell=%%a!blank!"
    set "cell=!cell:~0,%colw%!"
    set "row_line=!row_line! !cell! ¦"
    set "r_rest=%%b"
    if not "%%b"=="" goto _table_split_r
)
echo !row_line!
shift
goto _table_rows
:_table_end
echo   %DIM%!hr!%R%
endlocal & goto :eof

:: ---------------------------------------------------------------------------
:confirm
:: :confirm "msg"  > 0 = Yes, 1 = No/Esc
if not defined ESC call :init
<nul set /p "=   %BYEL%[?]%R% %~2 %DIM%(y/n)%R% "
choice /c yn /n >nul
if errorlevel 2 ( echo %BRED%No%R%  & exit /b 1 )
echo %BGRN%Yes%R%
exit /b 0

:: ---------------------------------------------------------------------------
:_pbar
:: Внутренняя: :_pbar pct [prefix] — перерисовка полосы на месте
setlocal EnableDelayedExpansion
set /a pct=%~1
if !pct! lss 0 set "pct=0"
if !pct! gtr 100 set "pct=100"
set /a barw=40
set /a fill=pct*barw/100
set "bar="
for /l %%i in (1,1,!barw!) do (
    if %%i leq !fill! (set "bar=!bar!-") else (set "bar=!bar!-")
)
<nul set /p "=!CLR%   %~2 %BCYN%[!bar!]%R% %B%!pct!%%%R%!HOME!"
endlocal & goto :eof