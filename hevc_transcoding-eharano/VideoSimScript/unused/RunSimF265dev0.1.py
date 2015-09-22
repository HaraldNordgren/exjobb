from RunSimDefinitions import RunSimF265

import testSeqs
import jobCtrl

# configure simulation

# main 8bit

simramain = RunSimF265(simName              = "ra_main_f265",
                          encoderExe           = "../src/f265-dev/f265/build/f265cli",
                          decoderExe           = "../src/HM-14.0/bin/TAppDecoderStatic",
                          cfgFileMain          = "cfg_f265/ra_main_f265.txt",
                          #testSet              = testSeqs.MPEG_kimono,
                          testSet              = testSeqs.JCTVC_RA_HM6_ALL,
                          maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N
                          temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                          qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration
                          allowSeqOverride     = 1,
                          cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess",
                          qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                          decode               = 1,
                          removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                          removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                          removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                          #jobTypeOverride       = jobCtrl.simpleJob,
                          jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email

simldmain = RunSimF265(simName              = "ld_main_f265",
                          encoderExe           = "../src/f265-dev/f265/build/f265cli",
                          decoderExe           = "../src/HM-14.0/bin/TAppDecoderStatic",
                          cfgFileMain          = "cfg_f265/ld_main_f265.txt",
                          #testSet              = testSeqs.MPEG_kimono,
                          testSet              = testSeqs.JCTVC_LD_HM6_ALL,
                          maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N
                          temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                          qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration
                          allowSeqOverride     = 1,
                          cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess",
                          qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                          decode               = 1,
                          removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                          removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                          removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                          #jobTypeOverride       = jobCtrl.simpleJob,
                          jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email

simdefaultmain = RunSimF265(simName              = "default_main_f265",
                          encoderExe           = "../src/f265-dev/f265/build/f265cli",
                          decoderExe           = "../src/HM-14.0/bin/TAppDecoderStatic",
                          cfgFileMain          = "cfg_f265/default_main_f265.txt",
                          #testSet              = testSeqs.MPEG_kimono,
                          testSet              = testSeqs.JCTVC_LD_HM6_ALL,
                          maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N
                          temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                          qpSet                = [[37], [32], [27], [22]],                 # may be overriden by sequence specific configuration
                          allowSeqOverride     = 1,
                          cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess",
                          qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                          decode               = 1,
                          removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                          removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                          removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                          #jobTypeOverride       = jobCtrl.simpleJob,
                          jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email

##
##simldPmain = RunSimF265(simName              = "ld_P_main_f265",
##                          encoderExe           = "../HM-10.0/bin/TAppEncoderStatic",
##                          decoderExe           = "../HM-10.0/bin/TAppDecoderStatic",
##                          cfgFileMain          = "../HM-10.0/cfg/encoder_lowdelay_P_main.cfg",
##                          testSet              = testSeqs.JCTVC_LD_HM6_ALL,
##                          #testSet              = testSeqs.VCEGMPEG_ClassF,
##                          maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N
##                          temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
##                          qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration
##                          allowSeqOverride     = 1,
##                          cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",
##                          qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
##                          decode               = 1,
##                          removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
##                          removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
##                          removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
##                          jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
##
##



# optionally change config file parameters
# NOTE: sequence properties (number of frames, frame sizes etc.)
#       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)




# start simulations

# main 8 bit
simramain.confMain['hm-gop'] = r'/proj/video_data/videosim/emartpe/f265-dev-v0.1/VideoSimScript/cfg_f265/encoder_randomaccess_main.cfg'
simramain.start()
simldmain.start()
simdefaultmain.start()










