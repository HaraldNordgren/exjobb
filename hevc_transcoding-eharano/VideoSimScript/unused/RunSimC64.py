from RunSimDefinitions import RunSimC64
import testSeqs

# configure simulation
sim = RunSimC64(simName              = "C64.r1_sp2_CAVLC_NoRC",
                encoderExe           = "bin/c64_enc.exe",
                decoderExe           = "bin/d64_dec.exe",
                cfgFileMain          = "cfg_c64/dummy.cfg",                    # Config file not used by C64
                testSet              = testSeqs.MPEG_vidyo,
                maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration
                allowSeqOverride     = 0,
                cfgSeqDirOverride    = "",
                qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                decode               = 1,
                removeEnc            = 1,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                removeRec            = 1,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                removeDec            = 1,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                jobParamsLsfOverride = ['windows', 'sim', False])              # only for LSF cluster: platform, queue, send results by email
                                                                               # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

# C64 is configured here, since no configuration file is used
# NOTE: sequence properties (number of frames, frame sizes etc.)
#       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)

# Number of reference pics (required parameter)
sim.confMain['RefFrames'] = 1

# Optional parameters
sim.confMain['Speed'] = 2
sim.confMain['Cabac'] = 0
sim.confMain['RCType'] = 0
sim.confMain['RCBitrate'] = 0
sim.confMain['SliceByteThreshold'] = 50000

# start simulations
sim.start()
