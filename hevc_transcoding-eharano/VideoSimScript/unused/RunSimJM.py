from RunSimDefinitions import RunSimJM
import testSeqs

# configure simulation
sim = RunSimJM(simName              = "jm16.2_classCD",
               encoderExe           = "bin/lencod.exe",
               decoderExe           = "bin/ldecod.exe",
               cfgFileMain          = "cfg_jm/alpha_AJ10r1.cfg",
               testSet              = testSeqs.Class_CD_CS1_CfP,
               maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
               temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
               qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration
               allowSeqOverride     = 1,
               cfgSeqDirOverride    = "cfg_jm/cfg_seq_CS1",
               qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
               decode               = 1,
               removeEnc            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
               removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
               removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
               jobParamsLsfOverride = ['linux', 'sim', True])                 # only for LSF cluster: platform, queue, send results by email
                                                                              # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

# optionally change config file parameters
# NOTE: sequence properties (number of frames, frame sizes etc.)
#       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
#sim.confMain['SearchMode'] = 1

# start simulations
sim.start()
