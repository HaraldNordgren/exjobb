import os, platform


#cfg_file            = os.path.abspath("HM-16.6/cfg/encoder_randomaccess_main.cfg")
cfg_file            = os.path.abspath("HM-16.6/cfg/encoder_intra_main.cfg")

framerate           = 1
frames              = 5


if platform.system() == "Windows":
    directory_root = os.path.abspath("//vhub.rnd.ki.sw.ericsson.se")

elif platform.system() == "Linux":
    directory_root = ""

else:
    raise Exception("Uknown platform")


#hevc_directory      = "/home/eharano/hevc_transcoding-eharano"
hevc_directory      = "/home/harald/exjobb/hevc_transcoding-eharano"

bitstream_folder    = os.path.abspath(directory_root + hevc_directory + "/bitstreams")
output_folder       = os.path.abspath(directory_root + hevc_directory + "/output_data")