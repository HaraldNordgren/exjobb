from RunSimDefinitions import RunSimRSM_HEVC
from RunSimDefinitions import RunSimRSM_ICE6
import testSeqs



# configure simulation
sim0 = RunSimRSM_HEVC(simName            = "HEVC",
               encoderExe           = "bin/RSM",
               decoderExe           = "bin/TAppDecoderStatic",
               cfgFileMain          = "cfg_rsm/ConfigHEVC.txt",
               testSet              = testSeqs.RSM_320p,
               maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N
               temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
               qpSet                = [[37],[32],[27],[22]],     # May be overriden by sequence specific configuration
               allowSeqOverride     = 0,
               cfgSeqDirOverride    = "",
               qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
               decode               = 1,
               removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
               removeRec            = 1,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
               removeDec            = 1,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
               jobParamsLsfOverride = ['linux', 'sim', True])                 # only for LSF cluster: platform, queue, send results by email
#               jobParamsLsfOverride = ['windows', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                              # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

sim1 = RunSimRSM_ICE6(simName            = "ICE6",
               encoderExe           = "bin/RSM",
               decoderExe           = "bin/ldecod.exe",
               cfgFileMain          = "cfg_rsm/ConfigICE6.txt",
               testSet              = testSeqs.RSM_320p,
               maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N
               temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
               qpSet                = [[37],[32],[27],[22]],     # May be overriden by sequence specific configuration
               allowSeqOverride     = 0,
               cfgSeqDirOverride    = "",
               qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
               decode               = 1,
               removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
               removeRec            = 1,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
               removeDec            = 1,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
               jobParamsLsfOverride = ['linux', 'sim', True])                 # only for LSF cluster: platform, queue, send results by email
#               jobParamsLsfOverride = ['windows', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                              # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements


# optionally change config file parameters
# NOTE: sequence properties (number of frames, frame sizes etc.)
#       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
#sim.confMain['SearchMode'] = 1

#Fixed QP:
sim0.confMain['RateControlType'] = "RateControl_Fixed"
sim1.confMain['RateControlType'] = "RateControl_Fixed"

#Uncomment for RateControl:
#sim0.confMain['RateControlType'] = "RateControl_Simple"
#sim1.confMain['RateControlType'] = "RateControl_Simple"
#sim0.confMain['RateControl_BitRateMultiplier'] = 1000
#sim1.confMain['RateControl_BitRateMultiplier'] = 1000


# start simulations

sim0.start()
sim1.start()

