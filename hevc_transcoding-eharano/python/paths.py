import os

def create_if_needed(directory):
	if not os.path.exists(directory):
		os.makedirs(directory)