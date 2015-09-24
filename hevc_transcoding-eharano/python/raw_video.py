import subprocess, os
from binaries import *

def mux(yuv_file, width, height):
    yuv_file_no_extension = os.path.splitext(yuv_file)[0]
    muxed_file = yuv_file_no_extension + ".avi"

    #ffmpeg -f rawvideo -s:v 1280x720 -i BQTerrace_1280x720_10_intra_dec.yuv -c:v copy BQTerrace_1280x720_10_intra_dec.avi
    #ffmpeg -f rawvideo -pix_fmt yuv420p -s:v 1920x1080 -i BQTerrace_1920x1080_1_randomaccess_dec.yuv -c:v copy BQTerrace_1920x1080_1_randomaccess_dec.avi

    ffmpeg_cmd = "%s -f rawvideo -pix_fmt yuv420p -s:v %dx%d -i %s -c:v copy %s" % (ffmpeg, width, height, yuv_file, muxed_file)
    subprocess.call(ffmpeg_cmd, shell=True)

def play_yuv(yuv_file, width, height):
    ffplay_cmd = "%s -i %s -video_size %dx%d -vcodec rawvideo -autoexit" % (ffplay, yuv_file, width, height)
    subprocess.call(ffplay_cmd, shell=True)