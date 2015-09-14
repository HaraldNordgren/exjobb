@echo off

cls

echo ## Deleting old jobs and results ##
rmdir /s /q jobs
rmdir /s /q Results
echo.

echo ## Running simulations ##
python RunSimHM16.6.py