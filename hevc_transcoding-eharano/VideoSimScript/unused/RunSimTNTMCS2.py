from RunSimDefinitions import RunSimTNTM
import testSeqs

# configure simulation
sim = RunSimTNTM(simName              = "tntmI8P",
                 encoderExe           = "bin/tntmenc.exe",
                 decoderExe           = "bin/tntmdec.exe",
                 cfgFileMain          = "cfg_tntm/configTNTM_I8PMPEGcfpclean.txt",
                 # cfgFileMain          = "cfg_tntm/configTNTM_I8PMPEGcfp.txt",
                 # configTNTM_I8PMPEGcfp.xt works on versions before clean up of TENTM 
                 testSet              = testSeqs.VCEGMPEG_CS2_CfP,
                 maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                 temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                 qpSet                = [[28], [23], [18], [13]],               # may be overriden by sequence specific configuration
                 allowSeqOverride     = 1,
                 cfgSeqDirOverride    = "cfg_tntm/cfg_seq_CS2_5qpnew_p4",
                 qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                 decode               = 1,
                 removeEnc            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                 removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                 removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                 jobParamsLsfOverride = ['intel', 'sim', True])                 # only for LSF cluster: platform, queue, send results by email
                                                                                # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

# optionally change config file parameters
# NOTE: sequence properties (number of frames, frame sizes etc.)
#       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
#sim.confMain['-rdoq'] = 0

# start simulations
sim.start()
