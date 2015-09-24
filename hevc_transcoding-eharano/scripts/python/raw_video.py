import subprocess, os
import binaries

def mux(yuv_file, width, height):
    yuv_file_no_extension = os.path.splitext(yuv_file)[0]
    muxed_file = yuv_file_no_extension + ".avi"

    ffmpeg_cmd = "%s -f rawvideo -pix_fmt yuv420p -s:v %dx%d -i %s -c:v copy %s" % (binaries.ffmpeg, width, height, yuv_file, muxed_file)
    subprocess.call(ffmpeg_cmd, shell=True)

def play_yuv(yuv_file, width, height):
    ffplay_cmd = "%s -i %s -video_size %dx%d -vcodec rawvideo -autoexit" % (binaries.ffplay, yuv_file, width, height)
    subprocess.call(ffplay_cmd, shell=True)