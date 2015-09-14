@echo off

if "%6"=="" (
	echo Usage: %~nx0 ^<hq-bitstream^> ^<decoded-hq-video^> ^<height^> ^<width^> ^<config-file^> ^<transcoded-output^> 
	exit /b
)

::Define binaries
set HOME_PATH=C:\Users\eharano\hevc_transcoding-eharano
set hm_encoder=%HOME_PATH%\HM-16.6\bin\vc9\Win32\Release\TAppEncoder.exe
set hm_decoder=%HOME_PATH%\HM-16.6\bin\vc9\Win32\Release\TAppDecoder.exe
set downscaler=%HOME_PATH%\d65_gt\tools\downScale\bin\Release\downScale.exe
set d65_gt_pruning=%HOME_PATH%\d65_gt\bin\d65_dec_pruning.exe
set d65_gt_dec_order=%HOME_PATH%\d65_gt\bin\d65_dec_dec_order.exe
set d65_gt_res_reconstruct=%HOME_PATH%\d65_gt\bin\d65_dec_residual_reconstruct.exe

::Catch input arguments
set hq_bitstream=%1
set hq_bitstream_shortpath=%~n1
set decoded_file=%2
set decoded_file_shortpath=%~n2
set width=%3
set height=%4
set cfg=%5
set transcoded_file=%6
set transcoded_folder=%~dp6

::Calculate downscaled dimensions
set /a half_width=width/2
set /a half_height=height/2

::Define filenames
set downscaled_file=%transcoded_folder%\%hq_bitstream_shortpath%_downscaled_to_%half_width%x%half_height%
set rdoq_0_file=%downscaled_file%_rdoq_0
set pruned_file=%rdoq_0_file%_pruned
set hq_bitstream_decoded_dec_order=%transcoded_folder%\%hq_bitstream_shortpath%_decoded_dec_order
set hq_bitstream_decoded_dec_order_downscaled=%hq_bitstream_decoded_dec_order%_downscaled_to_%half_width%x%half_height%
set reconstructed_file=%transcoded_folder%\%hq_bitstream_shortpath%_guided_transcoding_size_%half_width%x%half_height%

::Downscale to half-size
%downscaler% %width% %height% %decoded_file% %downscaled_file%.yuv 0

::Re-encode with RDOQ=0
set options=-fr 5 -f 10 -wdt %half_width% -hgt %half_height% --RDOQ=0 -SBH 0 --RDOQTS=0
%hm_encoder% -c %cfg% -i %downscaled_file%.yuv -b %rdoq_0_file%.bin %options%

::Prune
%d65_gt_pruning% -i %rdoq_0_file%.bin -n %pruned_file%.bin

::Decode HQ bitstream in decoding order and downscale
%d65_gt_dec_order% -i %hq_bitstream% -o %hq_bitstream_decoded_dec_order%.yuv
%downscaler% %width% %height% %hq_bitstream_decoded_dec_order%.yuv ^
	%hq_bitstream_decoded_dec_order_downscaled%.yuv 0

::Perform guided transcoding and decode
%d65_gt_res_reconstruct% -i %pruned_file%.bin -u %hq_bitstream_decoded_dec_order_downscaled%.yuv -n %reconstructed_file%.bin
%hm_decoder% -b %reconstructed_file%.bin -o %transcoded_file%

::Clean-up
del %downscaled_file%.yuv
del %rdoq_0_file%.bin
del %pruned_file%.bin
del %hq_bitstream_decoded_dec_order%.yuv
del %hq_bitstream_decoded_dec_order_downscaled%.yuv
del %reconstructed_file%.bin