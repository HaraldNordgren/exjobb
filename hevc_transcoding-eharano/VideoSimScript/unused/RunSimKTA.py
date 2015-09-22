from RunSimDefinitions import RunSimKTA
import testSeqs

# configure simulation
sim = RunSimKTA(simName              = "kta2.6r1_extMB_MVC_HP1_BALF_CS1",
                encoderExe           = "bin/lencod.exe",
                decoderExe           = "bin/ldecod.exe",
                cfgFileMain          = "cfg_kta/ktacombnew_AJ010r1_w10553_hpGOP8.cfg",
                testSet              = testSeqs.VCEGMPEG_CS1_CfP,
                maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration
                allowSeqOverride     = 1,
                cfgSeqDirOverride    = "cfg_kta/cfg_seq_CS1",
                qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                decode               = 1,
                removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
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
