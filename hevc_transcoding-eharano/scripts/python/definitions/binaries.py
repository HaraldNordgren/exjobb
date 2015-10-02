import platform
import os.path as path

if platform.system() == "Windows":

    #downscaler              = path.normpath("bin/windows/downScale/downScale.exe")
    #downscaler              = path.normpath("software/downScale/windows/downScale.exe")

    hm_encoder              = path.normpath("software/HM-16.6/bin/vc9/Win32/Release/TAppEncoder.exe")
    hm_decoder              = path.normpath("software/HM-16.6/bin/vc9/Win32/Release/TAppDecoder.exe")
    downscaler              = path.normpath("software/d65_gt/tools/downScale/bin/Release/downScale.exe")
    d65_gt_pruning          = path.normpath("software/d65_gt/bin/d65_dec_pruning.exe")
    d65_gt_dec_order        = path.normpath("software/d65_gt/bin/d65_dec_dec_order.exe")
    d65_gt_res_reconstruct  = path.normpath("software/d65_gt/bin/d65_dec_residual_reconstruct.exe")
    ffmpeg                  = path.normpath("software/ffmpeg/ffmpeg.exe")
    #ffplay                  = path.normpath("software/ffmpeg/ffplay.exe")
    #PSNRStatic              = path.normpath("software/PSNRStatic/PSNRStatic.exe")

elif platform.system() == "Linux":

	#downscaler              = path.normpath("bin/linux/downScale/downScale")

    hm_encoder              = path.normpath("software/HM-16.6/bin/TAppEncoderStatic")
    hm_decoder              = path.normpath("software/HM-16.6/bin/TAppDecoderStatic")
    downscaler              = path.normpath("software/d65_gt/tools/downScale/bin/linux/downScale")
    d65_gt_pruning          = path.normpath("software/d65_gt/build_pruning/d65_dec")
    d65_gt_dec_order        = path.normpath("software/d65_gt/build_dec_order/d65_dec")
    d65_gt_res_reconstruct  = path.normpath("software/d65_gt/build_residual_reconstruct/d65_dec")
    ffmpeg                  = path.normpath("software/ffmpeg/ffmpeg-2.8-32bit-static/ffmpeg")
    #ffplay                  = path.normpath("software/ffmpeg/linux/ffmpeg/ffplay")
    PSNRStatic              = path.normpath("software/PSNRStatic/bin/linux/PSNRStatic")

else:

    raise Exception("Uknown platform")