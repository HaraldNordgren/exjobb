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
                     encoderExe           = "../SHM-3.0.1/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-3.0.1/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-3.0.1/cfg/encoder_intra_main.cfg",
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

    # start simulations
    simimain.confMain['ConformanceMode0'] = 1
    simimain.confMain['ConformanceMode1'] = 1
    simimain.confMain['AvcBase'] = 0    
    simimain.confMain['NumLayers'] = 2
    simimain.confMain['ScalabilityMask0'] = 0 # Multiview
    simimain.confMain['ScalabilityMask1'] = 1 # Scalable
    simimain.confMain['AdaptiveResolutionChange'] = 0
    simimain.confMain['NumSamplePredRefLayers1'] = 1 # number of sample pred reference layers
    simimain.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simimain.confMain['NumMotionPredRefLayers1'] = 1 # number of motion pred reference layers
    simimain.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simimain.confMain['NumActiveRefLayers1'] = 1 # number of active reference layers
    simimain.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simimain.confMain['IntraPeriod'] = 1
    #simimain.start()


# loop over QP offsets
for qpOff in qpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "QPoff=%d\n" % (qpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+qpOff])

    # configure simulation
    simimain2x = RunSimSHVC(simName              = "i_main_spatial2x_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "../SHM-3.0.1/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-3.0.1/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-3.0.1/cfg/encoder_intra_main.cfg",
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

    # start simulations
    simimain2x.confMain['ConformanceMode0'] = 1
    simimain2x.confMain['ConformanceMode1'] = 1
    simimain2x.confMain['AvcBase'] = 0    
    simimain2x.confMain['NumLayers'] = 2
    simimain2x.confMain['ScalabilityMask0'] = 0 # Multiview
    simimain2x.confMain['ScalabilityMask1'] = 1 # Scalable
    simimain2x.confMain['AdaptiveResolutionChange'] = 0
    simimain2x.confMain['NumSamplePredRefLayers1'] = 1 # number of sample pred reference layers
    simimain2x.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simimain2x.confMain['NumMotionPredRefLayers1'] = 1 # number of motion pred reference layers
    simimain2x.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simimain2x.confMain['NumActiveRefLayers1'] = 1 # number of active reference layers
    simimain2x.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
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
                     encoderExe           = "../SHM-3.0.1/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-3.0.1/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-3.0.1/cfg/encoder_randomaccess_main.cfg",
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

    # start simulations
    simramain.confMain['ConformanceMode0'] = 1
    simramain.confMain['ConformanceMode1'] = 1
    simramain.confMain['AvcBase'] = 0    
    simramain.confMain['NumLayers'] = 2
    simramain.confMain['ScalabilityMask0'] = 0 # Multiview
    simramain.confMain['ScalabilityMask1'] = 1 # Scalable
    simramain.confMain['AdaptiveResolutionChange'] = 0
    simramain.confMain['NumSamplePredRefLayers1'] = 1 # number of sample pred reference layers
    simramain.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simramain.confMain['NumMotionPredRefLayers1'] = 1 # number of motion pred reference layers
    simramain.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simramain.confMain['NumActiveRefLayers1'] = 1 # number of active reference layers
    simramain.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simramain.confMain['IntraPeriod'] = 30 # will be adjusted by sequences specific intra period
    #simramain.start()


# loop over QP offsets
for qpOff in qpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "QPoff=%d\n" % (qpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+qpOff])

    # configure simulation
    simramain2x = RunSimSHVC(simName              = "ra_main_spatial2x_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "../SHM-3.0.1/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-3.0.1/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-3.0.1/cfg/encoder_randomaccess_main.cfg",
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

    # start simulations
    simramain2x.confMain['ConformanceMode0'] = 1
    simramain2x.confMain['ConformanceMode1'] = 1
    simramain2x.confMain['AvcBase'] = 0    
    simramain2x.confMain['NumLayers'] = 2
    simramain2x.confMain['ScalabilityMask0'] = 0 # Multiview
    simramain2x.confMain['ScalabilityMask1'] = 1 # Scalable
    simramain2x.confMain['AdaptiveResolutionChange'] = 0
    simramain2x.confMain['NumSamplePredRefLayers1'] = 1 # number of sample pred reference layers
    simramain2x.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simramain2x.confMain['NumMotionPredRefLayers1'] = 1 # number of motion pred reference layers
    simramain2x.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simramain2x.confMain['NumActiveRefLayers1'] = 1 # number of active reference layers
    simramain2x.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
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
                     encoderExe           = "../SHM-3.0.1/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-3.0.1/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-3.0.1/cfg/encoder_randomaccess_main.cfg",
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

    # start simulations
    # optional
    simramainsnr.confMain['ConformanceMode0'] = 1
    simramainsnr.confMain['ConformanceMode1'] = 1
    simramainsnr.confMain['AvcBase'] = 0    
    simramainsnr.confMain['NumLayers'] = 2
    simramainsnr.confMain['ScalabilityMask0'] = 0 # Multiview
    simramainsnr.confMain['ScalabilityMask1'] = 1 # Scalable
    simramainsnr.confMain['AdaptiveResolutionChange'] = 0
    simramainsnr.confMain['NumSamplePredRefLayers1'] = 1 # number of sample pred reference layers
    simramainsnr.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simramainsnr.confMain['NumMotionPredRefLayers1'] = 1 # number of motion pred reference layers
    simramainsnr.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simramainsnr.confMain['NumActiveRefLayers1'] = 1 # number of active reference layers
    simramainsnr.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simramainsnr.confMain['IntraPeriod'] = 30 # will be adjusted by sequences specific intra period
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
                     encoderExe           = "../SHM-3.0.1/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-3.0.1/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-3.0.1/cfg/encoder_lowdelay_main.cfg",
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
    simldmain.confMain['ConformanceMode0'] = 1
    simldmain.confMain['ConformanceMode1'] = 1
    simldmain.confMain['AvcBase'] = 0    
    simldmain.confMain['NumLayers'] = 2
    simldmain.confMain['ScalabilityMask0'] = 0 # Multiview
    simldmain.confMain['ScalabilityMask1'] = 1 # Scalable
    simldmain.confMain['AdaptiveResolutionChange'] = 0
    simldmain.confMain['NumSamplePredRefLayers1'] = 1 # number of sample pred reference layers
    simldmain.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simldmain.confMain['NumMotionPredRefLayers1'] = 1 # number of motion pred reference layers
    simldmain.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simldmain.confMain['NumActiveRefLayers1'] = 1 # number of active reference layers
    simldmain.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simldmain.confMain['IntraPeriod'] = -1
    #simldmain.start()


# loop over QP offsets
for qpOff in qpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "QPoff=%d\n" % (qpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+qpOff])

    # configure simulation
    simldmain2x = RunSimSHVC(simName              = "ld_main_spatial2x_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "../SHM-3.0.1/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-3.0.1/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-3.0.1/cfg/encoder_lowdelay_main.cfg",
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
    simldmain2x.confMain['ConformanceMode0'] = 1
    simldmain2x.confMain['ConformanceMode1'] = 1
    simldmain2x.confMain['AvcBase'] = 0    
    simldmain2x.confMain['NumLayers'] = 2
    simldmain2x.confMain['ScalabilityMask0'] = 0 # Multiview
    simldmain2x.confMain['ScalabilityMask1'] = 1 # Scalable
    simldmain2x.confMain['AdaptiveResolutionChange'] = 0
    simldmain2x.confMain['NumSamplePredRefLayers1'] = 1 # number of sample pred reference layers
    simldmain2x.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simldmain2x.confMain['NumMotionPredRefLayers1'] = 1 # number of motion pred reference layers
    simldmain2x.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simldmain2x.confMain['NumActiveRefLayers1'] = 1 # number of active reference layers
    simldmain2x.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
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
                     encoderExe           = "../SHM-3.0.1/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-3.0.1/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-3.0.1/cfg/encoder_lowdelay_main.cfg",
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
    simldmainsnr.confMain['ConformanceMode0'] = 1
    simldmainsnr.confMain['ConformanceMode1'] = 1
    simldmainsnr.confMain['AvcBase'] = 0    
    simldmainsnr.confMain['NumLayers'] = 2
    simldmainsnr.confMain['ScalabilityMask0'] = 0 # Multiview
    simldmainsnr.confMain['ScalabilityMask1'] = 1 # Scalable
    simldmainsnr.confMain['AdaptiveResolutionChange'] = 0
    simldmainsnr.confMain['NumSamplePredRefLayers1'] = 1 # number of sample pred reference layers
    simldmainsnr.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simldmainsnr.confMain['NumMotionPredRefLayers1'] = 1 # number of motion pred reference layers
    simldmainsnr.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simldmainsnr.confMain['NumActiveRefLayers1'] = 1 # number of active reference layers
    simldmainsnr.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simldmainsnr.confMain['IntraPeriod'] = -1
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
                     encoderExe           = "../SHM-3.0.1/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-3.0.1/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-3.0.1/cfg/encoder_lowdelay_P_main.cfg",
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
    simldPmain.confMain['ConformanceMode0'] = 1
    simldPmain.confMain['ConformanceMode1'] = 1
    simldPmain.confMain['AvcBase'] = 0    
    simldPmain.confMain['NumLayers'] = 2
    simldPmain.confMain['ScalabilityMask0'] = 0 # Multiview
    simldPmain.confMain['ScalabilityMask1'] = 1 # Scalable
    simldPmain.confMain['AdaptiveResolutionChange'] = 0
    simldPmain.confMain['NumSamplePredRefLayers1'] = 1 # number of sample pred reference layers
    simldPmain.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simldPmain.confMain['NumMotionPredRefLayers1'] = 1 # number of motion pred reference layers
    simldPmain.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simldPmain.confMain['NumActiveRefLayers1'] = 1 # number of active reference layers
    simldPmain.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simldPmain.confMain['IntraPeriod'] = -1
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
                     encoderExe           = "../SHM-3.0.1/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-3.0.1/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-3.0.1/cfg/encoder_lowdelay_P_main.cfg",
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
    simldPmain2x.confMain['ConformanceMode0'] = 1
    simldPmain2x.confMain['ConformanceMode1'] = 1
    simldPmain2x.confMain['AvcBase'] = 0    
    simldPmain2x.confMain['NumLayers'] = 2
    simldPmain2x.confMain['ScalabilityMask0'] = 0 # Multiview
    simldPmain2x.confMain['ScalabilityMask1'] = 1 # Scalable
    simldPmain2x.confMain['AdaptiveResolutionChange'] = 0
    simldPmain2x.confMain['NumSamplePredRefLayers1'] = 1 # number of sample pred reference layers
    simldPmain2x.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simldPmain2x.confMain['NumMotionPredRefLayers1'] = 1 # number of motion pred reference layers
    simldPmain2x.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simldPmain2x.confMain['NumActiveRefLayers1'] = 1 # number of active reference layers
    simldPmain2x.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simldPmain2x.confMain['IntraPeriod'] = -1
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
                     encoderExe           = "../SHM-3.0.1/bin/TAppEncoderStatic",
                     decoderExe           = "../SHM-3.0.1/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../SHM-3.0.1/cfg/encoder_lowdelay_P_main.cfg",
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
    simldPmainsnr.confMain['ConformanceMode0'] = 1
    simldPmainsnr.confMain['ConformanceMode1'] = 1
    simldPmainsnr.confMain['AvcBase'] = 0    
    simldPmainsnr.confMain['NumLayers'] = 2
    simldPmainsnr.confMain['ScalabilityMask0'] = 0 # Multiview
    simldPmainsnr.confMain['ScalabilityMask1'] = 1 # Scalable
    simldPmainsnr.confMain['AdaptiveResolutionChange'] = 0
    simldPmainsnr.confMain['NumSamplePredRefLayers1'] = 1 # number of sample pred reference layers
    simldPmainsnr.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simldPmainsnr.confMain['NumMotionPredRefLayers1'] = 1 # number of motion pred reference layers
    simldPmainsnr.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simldPmainsnr.confMain['NumActiveRefLayers1'] = 1 # number of active reference layers
    simldPmainsnr.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simldPmainsnr.confMain['IntraPeriod'] = -1
    #simldPmainsnr.start()
