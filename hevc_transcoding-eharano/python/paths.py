import os, sys
import shutil

def create_if_needed(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)

def remove_and_recreate_directory(directory):
    if os.path.exists(directory):
        shutil.rmtree(directory)
    
    os.makedirs(directory)

def assert_hevc_directory():
    cwd = os.getcwd()
    normpath = os.path.normpath(cwd)
    directory = os.path.basename(normpath)

    hevc_directory = "hevc_transcoding-eharano"

    if directory != hevc_directory:
        print "Run from \"%s\" directory" % hevc_directory
        sys.exit(1)