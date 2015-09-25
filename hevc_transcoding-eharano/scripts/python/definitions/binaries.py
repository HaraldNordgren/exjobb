import platform, os

if platform.system() == "Windows":

    hm_encoder              = os.path.abspath("software/HM-16.6/bin/vc9/Win32/Release/TAppEncoder.exe")
    hm_decoder              = os.path.abspath("software/HM-16.6/bin/vc9/Win32/Release/TAppDecoder.exe")
    downscaler              = os.path.abspath("bin/windows/downScale/downScale.exe")
    d65_gt_pruning          = os.path.abspath("software/d65_gt/bin/d65_dec_pruning.exe")
    d65_gt_dec_order        = os.path.abspath("software/d65_gt/bin/d65_dec_dec_order.exe")
    d65_gt_res_reconstruct  = os.path.abspath("software/d65_gt/bin/d65_dec_residual_reconstruct.exe")
    ffplay                  = os.path.abspath("bin/windows/ffmpeg/ffplay.exe")
    ffmpeg                  = os.path.abspath("bin/windows/ffmpeg/ffmpeg.exe")

elif platform.system() == "Linux":

    hm_encoder              = os.path.abspath("software/HM-16.6/bin/TAppEncoderStatic")
    hm_decoder              = os.path.abspath("software/HM-16.6/bin/TAppDecoderStatic")
    downscaler              = os.path.abspath("bin/linux/downScale/downScale")
    d65_gt_pruning          = os.path.abspath("software/d65_gt/build_pruning/d65_dec")
    d65_gt_dec_order        = os.path.abspath("software/d65_gt/build_dec_order/d65_dec")
    d65_gt_res_reconstruct  = os.path.abspath("software/d65_gt/build_residual_reconstruct/d65_dec")
    #ffplay                  = os.path.abspath("bin/linux/ffmpeg/ffplay")
    ffmpeg                  = os.path.abspath("bin/linux/ffmpeg/ffmpeg")

else:

    raise Exception("Uknown platform")