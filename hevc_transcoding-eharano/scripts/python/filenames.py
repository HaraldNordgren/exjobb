import re

dimensions_regex    = re.compile("(.*_)(\d+)x(\d+)(.*)")
framerate_regex     = re.compile("(.*_)(\d+)($|_.*)")
cfg_regex           = re.compile(".*encoder_([a-z]+)_.*")

def extract_dimensions(filename):
    m = dimensions_regex.match(filename)

    if not m:
        raise Exception('Input dimensions could not be determined')

    width = int(m.group(2))
    height = int(m.group(3))

    return (width, height)

def extract_cfg_mode(filename):
    m = cfg_regex.match(filename)

    if not m:
        raise Exception('Cfg mode could not be determined')

    return m.group(1)

def extract_framerate(filename):
    m = framerate_regex.match(filename)

    if not m:
        raise Exception('Framerate could not be determined')

    return int(m.group(2))

def replace_dimensions(original_name, scaled_width, scaled_height):
    m = dimensions_regex.match(original_name)
    
    if not m:
        raise Exception('Input dimensions could not be determined')

    name_start = m.group(1)
    name_end = m.group(4)

    return "%s%dx%d%s" % (name_start, scaled_width, scaled_height, name_end)

def replace_framerate(original_name, new_framerate):
    m = framerate_regex.match(original_name)
    
    if not m:
        raise Exception('Framerate could not be determined')

    name_start = m.group(1)
    name_end = m.group(3)

    return "%s%d%s" % (name_start, new_framerate, name_end)