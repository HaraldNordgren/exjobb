from RunSimDefinitions import RunSimSamsung
import testSeqs

# configure simulation
simra37 = RunSimSamsung(simName              = "ra",
                                   encoderExe           = "../Software/HM-1.0-dev/bin/TAppEncoderStatic",
                                   decoderExe           = "../Software/HM-1.0-dev/bin/TAppDecoderStatic",
                                   cfgFileMain          = "../Software/HM-1.0-dev/cfg/encoder_randomaccess.cfg",
                                   testSet              = testSeqs.VCEGMPEG_CS1_CfP_SQ,
                                   #testSet              = testSeqs.BQSquare,
                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N
                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration
                                   allowSeqOverride     = 1,
                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_CS1_4qp",
                                   qpNum                = 0,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                                   decode               = 1,
                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   jobParamsLsfOverride = ['linux', 'sim_24h', False])                 # only for LSF cluster: platform, queue, send results by email

simra32 = RunSimSamsung(simName              = "ra",
                                   encoderExe           = "../Software/HM-1.0-dev/bin/TAppEncoderStatic",
                                   decoderExe           = "../Software/HM-1.0-dev/bin/TAppDecoderStatic",
                                   cfgFileMain          = "../Software/HM-1.0-dev/cfg/encoder_randomaccess.cfg",
                                   testSet              = testSeqs.VCEGMPEG_CS1_CfP_SQ,
                                   #testSet              = testSeqs.BQSquare,
                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N
                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration
                                   allowSeqOverride     = 1,
                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_CS1_4qp",
                                   qpNum                = 1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                                   decode               = 1,
                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   jobParamsLsfOverride = ['linux', 'sim_24h', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                                             


simld37 = RunSimSamsung(simName              = "ld",
                                   encoderExe           = "../Software/HM-1.0-dev/bin/TAppEncoderStatic",
                                   decoderExe           = "../Software/HM-1.0-dev/bin/TAppDecoderStatic",
                                   cfgFileMain          = "../Software/HM-1.0-dev/cfg/encoder_lowdelay.cfg",
                                   testSet              = testSeqs.VCEGMPEG_CS2_CfP_SQVidyo,
                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N
                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration
                                   allowSeqOverride     = 0,
                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_CS2",
                                   qpNum                = 0,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                                   decode               = 1,
                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   jobParamsLsfOverride = ['linux', 'sim_24h', False])                 # only for LSF cluster: platform, queue, send results by email


simld32 = RunSimSamsung(simName              = "ld",
                                   encoderExe           = "../Software/HM-1.0-dev/bin/TAppEncoderStatic",
                                   decoderExe           = "../Software/HM-1.0-dev/bin/TAppDecoderStatic",
                                   cfgFileMain          = "../Software/HM-1.0-dev/cfg/encoder_lowdelay.cfg",
                                   testSet              = testSeqs.VCEGMPEG_CS2_CfP_SQVidyo,
                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N
                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration
                                   allowSeqOverride     = 0,
                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_CS2",
                                   qpNum                = 1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                                   decode               = 1,
                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   jobParamsLsfOverride = ['linux', 'sim_24h', False])                 # only for LSF cluster: platform, queue, send results by email

                                                                                             


simldloco37 = RunSimSamsung(simName              = "ld_loco",
                                   encoderExe           = "../Software/HM-1.0-dev/bin/TAppEncoderStatic",
                                   decoderExe           = "../Software/HM-1.0-dev/bin/TAppDecoderStatic",
                                   cfgFileMain          = "../Software/HM-1.0-dev/cfg/encoder_lowdelay_loco.cfg",
                                   testSet              = testSeqs.VCEGMPEG_CS2_CfP_SQ,
                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N
                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration
                                   allowSeqOverride     = 0,
                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_CS2",
                                   qpNum                = 0,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                                   decode               = 1,
                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   jobParamsLsfOverride = ['linux', 'sim_24h', False])                 # only for LSF cluster: platform, queue, send results by email


simldloco32 = RunSimSamsung(simName              = "ld_loco",
                                   encoderExe           = "../Software/HM-1.0-dev/bin/TAppEncoderStatic",
                                   decoderExe           = "../Software/HM-1.0-dev/bin/TAppDecoderStatic",
                                   cfgFileMain          = "../Software/HM-1.0-dev/cfg/encoder_lowdelay_loco.cfg",
                                   testSet              = testSeqs.VCEGMPEG_CS2_CfP_SQ,
                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N
                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration
                                   allowSeqOverride     = 0,
                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_CS2",
                                   qpNum                = 1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                                   decode               = 1,
                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                                   jobParamsLsfOverride = ['linux', 'sim_24h', False])                 # only for LSF cluster: platform, queue, send results by email


# optionally change config file parameters
# NOTE: sequence properties (number of frames, frame sizes etc.)
#       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
#sim.confMain['SearchMode'] = 1

# command parameters can be modified by setting parameters to 'commandOpts'
#sim.confMain['commandOpts'] = '-s 64'

# start simulations
simldloco37.start()
simra37.start()
simld37.start()

simldloco32.start()
simra32.start()
simld32.start()


