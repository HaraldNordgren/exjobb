from RunSimDefinitions import RunSimSHVC
import testSeqs

# QP settings
qpSetBL = [34,30,26,22]
qpOffset = [0,2]

# SNR scalability
qpSetBLSNR = [38,34,30,26]
qpOffsetSNR = [-6,-4]

# loop over QP offsets
for qpOff in qpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "QPoff=%d\n" % (qpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+qpOff])

    # configure simulation
    simimain = RunSimSHVC(simName              = "i_main_spatial1m5x_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "../SHM-2.0/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-2.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-2.0/cfg/encoder_intra_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SPATIAL_1m5X,
                     maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_intra",                     
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
    #sim.confMain['NumberReferenceFrames'] = 0

    # start simulations
    simimain.confMain['ConformanceMode0'] = 1
    simimain.confMain['ConformanceMode1'] = 1
    simimain.confMain['NumDirectRefLayers1'] = 1
    simimain.confMain['AvcBase'] = 0    
    simimain.confMain['RefLayerIds1'] = 0    
    simimain.confMain['IntraPeriod'] = 1
    simimain.start()


# loop over QP offsets
for qpOff in qpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "QPoff=%d\n" % (qpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+qpOff])

    # configure simulation
    simimain2x = RunSimSHVC(simName              = "i_main_spatial2x_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "../SHM-2.0/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-2.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-2.0/cfg/encoder_intra_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SPATIAL_2X,
                     maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_intra",                     
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
    #sim.confMain['NumberReferenceFrames'] = 0

    # start simulations
    simimain2x.confMain['ConformanceMode0'] = 1
    simimain2x.confMain['ConformanceMode1'] = 1
    simimain2x.confMain['NumDirectRefLayers1'] = 1
    simimain2x.confMain['AvcBase'] = 0    
    simimain2x.confMain['RefLayerIds1'] = 0    
    simimain2x.confMain['IntraPeriod'] = 1
    simimain2x.start()

# loop over QP offsets
for qpOff in qpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "QPoff=%d\n" % (qpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+qpOff])

    # configure simulation
    simramain = RunSimSHVC(simName              = "ra_main_spatial1m5x_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "../SHM-2.0/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-2.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-2.0/cfg/encoder_randomaccess_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SPATIAL_1m5X,
                     maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess",                     
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
    #sim.confMain['NumberReferenceFrames'] = 0

    # start simulations
    simramain.confMain['ConformanceMode0'] = 1
    simramain.confMain['ConformanceMode1'] = 1    
    simramain.confMain['NumDirectRefLayers1'] = 1
    simramain.confMain['AvcBase'] = 0    
    simramain.confMain['RefLayerIds1'] = 0    
    simramain.confMain['IntraPeriod'] = 30 # will be adjusted by sequences specific intra period
    simramain.start()


# loop over QP offsets
for qpOff in qpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "QPoff=%d\n" % (qpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+qpOff])

    # configure simulation
    simramain2x = RunSimSHVC(simName              = "ra_main_spatial2x_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "../SHM-2.0/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-2.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-2.0/cfg/encoder_randomaccess_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SPATIAL_2X,
                     maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess",                     
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
    #sim.confMain['NumberReferenceFrames'] = 0

    # start simulations
    simramain2x.confMain['ConformanceMode0'] = 1
    simramain2x.confMain['ConformanceMode1'] = 1    
    simramain2x.confMain['NumDirectRefLayers1'] = 1
    simramain2x.confMain['AvcBase'] = 0    
    simramain2x.confMain['RefLayerIds1'] = 0    
    simramain2x.confMain['IntraPeriod'] = 30 # will be adjusted by sequences specific intra period
    simramain2x.start()


# loop over QP offsets
for qpOff in qpOffsetSNR:

  # create QP set
  for qpBL in qpSetBLSNR:
    print "QPoff=%d\n" % (qpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+qpOff])

    # configure simulation
    simramainsnr = RunSimSHVC(simName              = "ra_main_snr_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "../SHM-2.0/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-2.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-2.0/cfg/encoder_randomaccess_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SNR,
                     maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess",                     
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
    #sim.confMain['NumberReferenceFrames'] = 0

    # start simulations
    # optional
    #simramainsnr.confMain['ConformanceMode0'] = 1
    #simramainsnr.confMain['ConformanceMode1'] = 1
    #simramainsnr.confMain['NumDirectRefLayers1'] = 1
    #simramainsnr.confMain['AvcBase'] = 0    
    #simramainsnr.confMain['RefLayerIds1'] = 0    
    #simramainsnr.confMain['IntraPeriod'] = 30 # will be adjusted by sequences specific intra period
    #simramainsnr.start()


# loop over QP offsets
for qpOff in qpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "QPoff=%d\n" % (qpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+qpOff])

    # configure simulation
    simldmain = RunSimSHVC(simName              = "ld_main_spatial1m5x_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "../SHM-2.0/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-2.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-2.0/cfg/encoder_lowdelay_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SPATIAL_1m5X,
                     maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",                     
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
    #sim.confMain['NumberReferenceFrames'] = 0

    # start simulations
    # optional
    simldmain.confMain['ConformanceMode0'] = 1
    simldmain.confMain['ConformanceMode1'] = 1
    simldmain.confMain['NumDirectRefLayers1'] = 1
    simldmain.confMain['AvcBase'] = 0    
    simldmain.confMain['RefLayerIds1'] = 0    
    simldmain.confMain['IntraPeriod'] = -1
    simldmain.start()


# loop over QP offsets
for qpOff in qpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "QPoff=%d\n" % (qpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+qpOff])

    # configure simulation
    simldmain2x = RunSimSHVC(simName              = "ld_main_spatial2x_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "../SHM-2.0/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-2.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-2.0/cfg/encoder_lowdelay_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SPATIAL_2X,
                     maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",                     
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
    #sim.confMain['NumberReferenceFrames'] = 0

    # start simulations
    # optional
    simldmain2x.confMain['ConformanceMode0'] = 1
    simldmain2x.confMain['ConformanceMode1'] = 1
    simldmain2x.confMain['NumDirectRefLayers1'] = 1
    simldmain2x.confMain['AvcBase'] = 0    
    simldmain2x.confMain['RefLayerIds1'] = 0    
    simldmain2x.confMain['IntraPeriod'] = -1
    simldmain2x.start()

# loop over QP offsets
for qpOff in qpOffsetSNR:

  # create QP set
  for qpBL in qpSetBLSNR:
    print "QPoff=%d\n" % (qpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+qpOff])

    # configure simulation
    simldmainsnr = RunSimSHVC(simName              = "ld_main_snr_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "../SHM-2.0/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-2.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-2.0/cfg/encoder_lowdelay_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SNR,
                     maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",                     
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
    #sim.confMain['NumberReferenceFrames'] = 0

    # start simulations
    # optional
    #simldmainsnr.confMain['ConformanceMode0'] = 1
    #simldmainsnr.confMain['ConformanceMode1'] = 1
    #simldmainsnr.confMain['NumDirectRefLayers1'] = 1
    #simldmainsnr.confMain['AvcBase'] = 0    
    #simldmainsnr.confMain['RefLayerIds1'] = 0         
    #simldmainsnr.confMain['IntraPeriod'] = -1
    #simldmainsnr.start()


# loop over QP offsets
for qpOff in qpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "QPoff=%d\n" % (qpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+qpOff])

    # configure simulation
    simldPmain = RunSimSHVC(simName              = "ld_P_main_spatial1m5x_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "../SHM-2.0/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-2.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-2.0/cfg/encoder_lowdelay_P_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SPATIAL_1m5X,
                     maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",                     
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
    #sim.confMain['NumberReferenceFrames'] = 0

    # start simulations
    # optional
    #simldPmain.confMain['ConformanceMode0'] = 1
    #simldPmain.confMain['ConformanceMode1'] = 1
    #simldPmain.confMain['NumDirectRefLayers1'] = 1
    #simldPmain.confMain['AvcBase'] = 0    
    #simldPmain.confMain['RefLayerIds1'] = 0    
    #simldPmain.confMain['IntraPeriod'] = -1
    #simldPmain.start()


# loop over QP offsets
for qpOff in qpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "QPoff=%d\n" % (qpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+qpOff])

    # configure simulation
    simldPmain2x = RunSimSHVC(simName              = "ld_P_main_spatial2x_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "../SHM-2.0/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-2.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-2.0/cfg/encoder_lowdelay_P_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SPATIAL_2X,
                     maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",                     
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
    #sim.confMain['NumberReferenceFrames'] = 0

    # start simulations
    # optional
    #simldPmain2x.confMain['ConformanceMode0'] = 1
    #simldPmain2x.confMain['ConformanceMode1'] = 1
    #simldPmain2x.confMain['NumDirectRefLayers1'] = 1
    #simldPmain2x.confMain['AvcBase'] = 0    
    #simldPmain2x.confMain['RefLayerIds1'] = 0    
    #simldPmain2x.confMain['IntraPeriod'] = -1
    #simldPmain2x.start()

# loop over QP offsets
for qpOff in qpOffsetSNR:

  # create QP set
  for qpBL in qpSetBLSNR:
    print "QPoff=%d\n" % (qpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+qpOff])

    # configure simulation
    simldPmainsnr = RunSimSHVC(simName              = "ld_P_main_snr_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "../SHM-2.0/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-2.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-2.0/cfg/encoder_lowdelay_P_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SNR,
                     maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",                     
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
    #sim.confMain['NumberReferenceFrames'] = 0

    # start simulations
    # optional
    #simldPmainsnr.confMain['ConformanceMode0'] = 1
    #simldPmainsnr.confMain['ConformanceMode1'] = 1
    #simldPmainsnr.confMain['NumDirectRefLayers1'] = 1
    #simldPmainsnr.confMain['AvcBase'] = 0    
    #simldPmainsnr.confMain['RefLayerIds1'] = 0    
    #simldPmainsnr.confMain['IntraPeriod'] = -1
    #simldPmainsnr.start()

