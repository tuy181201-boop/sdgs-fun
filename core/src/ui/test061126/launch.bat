@echo off
set PATH=C:\Windows\System32;C:\Windows;%PATH%

echo =========================================
echo DIAGNOSTIC RUN
echo =========================================

echo [STEP 1] Starting Python UI Generator...
:: /k оставит окно открытым после выполнения
start "Python UI Generator" cmd /k "cd /d C:\Users\Tuy181201\Desktop && python make_ui.py"

timeout /t 2 /nobreak >nul

echo [STEP 2] Starting Lisp Engine...
"C:\msys64\usr\bin\bash.exe" -lc "cd /c/Users/Tuy181201/Desktop && /c/msys64/ucrt64/bin/sbcl.exe --script test2.lisp"

echo.
echo [STEP 3] Lisp closed. Check Python window for errors.
pause