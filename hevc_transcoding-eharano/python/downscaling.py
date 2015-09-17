import os, re
import subprocess

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
	old_file = input_file
	tmp_file = input_file + ".tmp"

	for p in downscale_parameters:
		downscaling_cmd = "%s %s %s %s %s %d" % (downscaler, width, height, old_file, tmp_file, p)
		subprocess.call(downscaling_cmd)

		print (width, height)
		(width, height) = convert_dimensions(width, height, p)
		print "converted"
		print (width, height)
		
		old_file = tmp_file

		print "\n\n"

	os.rename(tmp_file, output_file)