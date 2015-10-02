import os, sys, shutil

def create_if_needed(directory):
    try:
        os.makedirs(directory)
    except OSError:
        pass

    #if not os.path.exists(directory):
    #    os.makedirs(directory)

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

def create_full_directory(root_folder, modular_folder):
    full_folder = "%s/%s" % (root_folder, modular_folder)
    create_if_needed(full_folder)

    return full_folder

def get_full_file(root_folder, modular_folder):
    return "%s/%s" % (root_folder, modular_folder)