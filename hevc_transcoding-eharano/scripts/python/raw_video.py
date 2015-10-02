import subprocess, os
import filenames, command_line

import definitions.binaries as binaries


def mux(yuv_file):
    yuv_file_no_extension = os.path.splitext(yuv_file)[0]
    muxed_file = yuv_file_no_extension + ".avi"

    (width, height) = filenames.extract_dimensions(yuv_file)

    ffmpeg_cmd = "%s -loglevel error -f rawvideo -pix_fmt yuv420p -s:v %dx%d -i %s -c:v copy %s" % \
        (binaries.ffmpeg, width, height, yuv_file, muxed_file)
    command_line.call_indented(ffmpeg_cmd)

    return muxed_file

def play_yuv(yuv_file):
    (width, height) = filenames.extract_dimensions(yuv_file)

    ffplay_cmd = "%s -i %s -video_size %dx%d -vcodec rawvideo -autoexit" % (binaries.ffplay, yuv_file, width, height)
    #subprocess.call(ffplay_cmd, shell=True)
    command_line.call_indented(ffplay_cmd)

def decode(bin_file):
    bin_file_no_extension = os.path.splitext(bin_file)[0]
    decoded_file = "%s_dec.yuv" % bin_file_no_extension

    decode_cmd = "%s -b %s -o %s" % (binaries.hm_decoder, bin_file, decoded_file)
    command_line.call_indented(decode_cmd)

    return decoded_file