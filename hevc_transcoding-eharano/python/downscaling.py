import os, glob
import subprocess

from binaries import *


def do_conversion(width, height, parameter):
    if (parameter == 0):
        if (width % 2 != 0 or height % 2 != 0):
            raise Exception("Dimension not divisible by 2")
        return (width / 2, height / 2)

    if (parameter == 1):
        if (width % 3 != 0 or height % 3 != 0):
            raise Exception("Dimension not divisible by 3")
        return (2 * width / 3, 2 * height / 3)

    raise Exception("Invalid parameter")

def convert_dimensions(width, height, parameters):
    for p in parameters:
        (width, height) = do_conversion(width, height, p)
    return (width, height)

def perform_downscaling(width, height, input_file, output_file, downscale_parameters):
    
    output_folder = os.path.split(output_file)[0]
    downscale_tmp_start = "%s/downscaler_tmp_" % output_folder

    downscale_files = [input_file]
    for i in range(len(downscale_parameters) - 1):
        downscale_files.append(downscale_tmp_start + str(i))
    downscale_files.append(output_file)

    for i in range(len(downscale_parameters)):
        downscaling_cmd = "%s %s %s %s %s %d 1" % (downscaler, width, height, downscale_files[i], downscale_files[i+1], downscale_parameters[i])
        subprocess.call(downscaling_cmd, shell=True)

        (width, height) = do_conversion(width, height, downscale_parameters[i])


    for tmp_file in glob.glob(downscale_tmp_start + "*"):
        os.remove(tmp_file)

def get_height_divisible_by_eight(nbr):
    return nbr / 8 * 8