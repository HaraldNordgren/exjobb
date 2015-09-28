import sys
sys.path.insert(0, "..")

import filenames


print filenames.extract_cfg_mode("encoder_randomaccess_main_rext.cfg")
print filenames.extract_cfg_mode("software/HM-16.6/cfg/encoder_randomaccess_main.cfg")