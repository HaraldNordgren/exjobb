import re

dimensions_regex = re.compile("(.*_)(\d+)x(\d+)(.*)")
framerate_regex = re.compile("(.*_)(\d+)($|_.*)")
cfg_regex = re.compile("encoder_([a-z]+)_.*")

def extract_dimensions(filename):
	m = dimensions_regex.match(filename)
	
	if m:
		width = int(m.group(2))
		height = int(m.group(3))
	else:
		raise Exception('Input dimensions could not be determined')

	return (width, height)

def extract_cfg_mode(filename):
	m = cfg_regex.match(filename)

	if m:
		return m.group(1)
	else:
		raise Exception('Cfg mode could not be determined')

def replace_dimensions(original_name, scaled_width, scaled_height):
	m = dimensions_regex.match(original_name)
	
	if m:
		name_start = m.group(1)
		name_end = m.group(4)
	else:
		raise Exception('Input dimensions could not be determined')

	return "%s%dx%d%s" % (name_start, scaled_width, scaled_height, name_end)

def replace_framerate(original_name, new_framerate):
	m = framerate_regex.match(original_name)
	
	if m:
		name_start = m.group(1)
		name_end = m.group(3)
	else:
		raise Exception('Framerate could not be determined')

	return "%s%d%s" % (name_start, new_framerate, name_end)