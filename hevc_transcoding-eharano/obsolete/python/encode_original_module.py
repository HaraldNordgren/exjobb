import os, sys, subprocess
import filenames, paths

import definitions.config       as config
import definitions.binaries     as binaries
import definitions.directories  as directories


def encode_original(original_file, cfg_file, QPs):

    # Input file

    original_file_basename = os.path.basename(original_file)
    original_file_shortpath = os.path.splitext(original_file_basename)[0]

    (width, height)  = filenames.extract_dimensions(original_file_shortpath)

    cfg_file_basename = os.path.basename(cfg_file)
    cfg_mode = filenames.extract_cfg_mode(cfg_file_basename)

    output_file_framerate_replaced = filenames.replace_framerate(original_file_shortpath, config.framerate)
    output_file_info_added = "%s_%s_%d-frames" % (output_file_framerate_replaced, cfg_mode, config.frames)

    output_folder = "%s/%s" % (directories.bitstream_folder, output_file_info_added)
    paths.create_if_needed(output_folder)

    for qp in QPs:

        # Output file

        output_file_shortpath = "%s_qp-%d" % (output_file_info_added, qp)
        output_file = "%s/%s.bin" % (output_folder, output_file_shortpath)

        encode_cmd = "%s -c %s -i %s -b %s -fr %s -f %s -hgt %s -wdt %s -SBH 1 --SEIDecodedPictureHash=2" % \
            (binaries.hm_encoder, cfg_file, original_file, output_file, config.framerate, config.frames, height, width)
        subprocess.call(encode_cmd, shell=True)
        #subprocess.call("touch " + output_file, shell=True)