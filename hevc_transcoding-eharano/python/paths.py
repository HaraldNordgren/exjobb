import os, sys
import shutil

def create_if_needed(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)

def assert_hevc_directory():
    HEVC_DIRECTORY = "hevc_transcoding-eharano"

    cwd = os.getcwd()
    normpath = os.path.normpath(cwd)
    directory = os.path.basename(normpath)

    if directory != HEVC_DIRECTORY:
        print "Run from \"%s\" directory" % HEVC_DIRECTORY
        sys.exit(1)

def remove_and_recreate_directory(path):
    if os.path.exists(path):
        shutil.rmtree(path)
    
    os.mkdir(path)