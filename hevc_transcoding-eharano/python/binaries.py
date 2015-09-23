import os, platform


def get_abspath(binary_path):
    return os.path.abspath(system_path + "/" + binary_path)


if platform.system() == "Windows":

    system_path             = "bin/windows"

    hm_encoder              = get_abspath("HM-16.6/TAppEncoder.exe")
    hm_decoder              = get_abspath("HM-16.6/TAppDecoder.exe")
    downscaler              = get_abspath("downScale/downScale.exe")
    d65_gt_pruning          = get_abspath("d65_gt/d65_dec_pruning.exe")
    d65_gt_dec_order        = get_abspath("d65_gt/d65_dec_dec_order.exe")
    d65_gt_res_reconstruct  = get_abspath("d65_gt/d65_dec_residual_reconstruct.exe")
    ffplay                  = get_abspath("ffmpeg/ffplay.exe")
    ffmpeg                  = "echo ffmpeg"

elif platform.system() == "Linux":

    system_path             = "bin/linux"

    #hm_encoder              = get_abspath("HM-16.6/TAppEncoderStatic")
    #hm_decoder              = get_abspath("HM-16.6/TAppDecoderStatic")
    #downscaler              = get_abspath("downScale/downScale")
    #d65_gt_pruning          = get_abspath("d65_gt/d65_dec_pruning")
    #d65_gt_dec_order        = get_abspath("d65_gt/d65_dec_dec_order")
    #d65_gt_res_reconstruct  = get_abspath("d65_gt/d65_dec_residual_reconstruct")
    #ffplay                  = "echo ffplay"

    hm_encoder              = os.path.abspath("HM-16.6/bin/TAppEncoderStatic")
    hm_decoder              = os.path.abspath("HM-16.6/bin/TAppDecoderStatic")
    downscaler              = os.path.abspath("bin/linux/downScale/downScale")
    d65_gt_pruning          = os.path.abspath("d65_gt/build_pruning/d65_dec")
    d65_gt_dec_order        = os.path.abspath("d65_gt/build_dec_order/d65_dec")
    d65_gt_res_reconstruct  = os.path.abspath("d65_gt/build_residual_reconstruct/d65_dec")
    ffplay                  = os.path.abspath("bin/linux/ffmpeg/ffplay")
    ffmpeg                  = os.path.abspath("bin/linux/ffmpeg/ffmpeg")

else:

    raise Exception("Uknown platform")