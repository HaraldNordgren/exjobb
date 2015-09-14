@echo off

if "%6"=="" (
	echo Usage: %0 ^<original-file^> ^<cfg-file^> ^<height^> ^<width^> ^<framerate^> ^<frames^>
	exit /b
)

set hm_encoder=HM-16.6\bin\vc9\Win32\Release\TAppEncoder.exe

set original_longpath=%~p1%~n1
set original_shortpath=%~n1

set cfg_longpath=%2
set cfg_shortpath=%~n2

set height=%3
set width=%4
set framerate=%5
set frames=%6

set output_folder=bitstreams\
set output_file=%output_folder%%original_shortpath%_cfg_%cfg_shortpath%_framerate_%framerate%

%hm_encoder% -c %cfg_longpath% -i %original_longpath%.yuv -b %output_file%.bin -fr %framerate% ^
	-f %frames% -hgt %height% -wdt %width% -SBH 1