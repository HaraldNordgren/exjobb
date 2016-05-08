@echo off
cls

set OUTPUT=build\output
if not exist %OUTPUT% (mkdir %OUTPUT%)

pdflatex -output-directory %OUTPUT% src\rapport.tex
if %errorlevel% neq 0 exit /b

taskkill /im AcroRd32.exe
move /y %OUTPUT%\rapport.pdf build
start /b "C:\Program Files (x86)\Adobe\Reader 11.0\Reader\AcroRd32.exe" build\rapport.pdf

echo.
echo.

bibtex %OUTPUT%\rapport.aux

echo.
echo.

cd src
makeglossaries -d ..\build\output rapport

cd ..