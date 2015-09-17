import os, re
import subprocess
import glob

from binaries import *

regex = re.compile("(.*_)(\d+)x(\d+)(.*)")

def convert_dimensions(width, height, parameter):
	if (parameter == 0):
		if (width % 2 != 0 or height % 2 != 0):
			raise Exception("Dimension not divisible by 2")
		return (width / 2, height / 2)

	if (parameter == 1):
		if (width % 3 != 0 or height % 3 != 0):
			raise Exception("Dimension not divisible by 3")
		return (2 * width / 3, 2 * height / 3)

	raise Exception("Invalid parameter")

def convert_dimensions_2(width, height, parameters):
	for p in parameters:
		(width, height) = convert_dimensions(width, height, p)
	return (width, height)


def dimensions_from_filename(filename):
	dimensions = regex.match(filename)
	if dimensions:
		width = int(dimensions.group(2))
		height = int(dimensions.group(3))
	else:
		raise Exception('Input dimensions could not be determined')

	return (width, height)

def filename_replace_dimensions(original_name, scaled_width, scaled_height):
	m = regex.match(original_name)
	if m:
		name_start = m.group(1)
		name_end = m.group(4)
	else:
		raise Exception('Input dimensions could not be determined')

	return "%s%dx%d%s" % (name_start, scaled_width, scaled_height, name_end)

def perform_downscaling(width, height, input_file, output_file, downscale_parameters):
	
	downscale_tmp_start = "downscale_tmp_"

	downscale_files = [input_file]
	for i in range(len(downscale_parameters)-1):
		downscale_files.append(downscale_tmp_start + str(i))
	downscale_files.append(output_file)

	for i in range(len(downscale_parameters)):
		downscaling_cmd = "%s %s %s %s %s %d" % (downscaler, width, height, downscale_files[i], downscale_files[i+1], downscale_parameters[i])
		subprocess.call(downscaling_cmd, shell=True)

		print (width, height)
		(width, height) = convert_dimensions(width, height, downscale_parameters[i])
		
		print "converted"
		print (width, height)
		print "\n\n"

	#for tmp_file in glob.glob(downscale_tmp_start + "*"):
	#	os.remove(tmp_file)