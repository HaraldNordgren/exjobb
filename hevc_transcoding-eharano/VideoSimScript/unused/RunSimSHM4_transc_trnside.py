from RunSimDefinitions import RunSimSHVC_TRN
import testSeqs

# QP settings
qpSetBL = [34,30,26,22]
#SIqpOffset = [0,2]
#SIqpOffset = [0,2,4]
SIqpOffset = [0,2]

#qpSetBL = [34]
#SIqpOffset = [0]

# SNR scalability
qpSetBLSNR = [38,34,30,26]
#SIqpOffsetSNR = [-6,-4]
#qpSetBLSNR = [32,28,24,20]
#SIqpOffsetSNR = [6,4]

qpSetBLSNR = [38]
SIqpOffsetSNR = [-4]



# loop over QP offsets
for SIqpOff in SIqpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "SIqpOff=%d\n" % (SIqpOff)
    qpSet = []
    #qpSet.append([qpBL+SIqpOff,qpBL])
    qpSet.append([qpBL,qpBL+SIqpOff])

    # configure simulation
    simimain = RunSimSHVC_TRN(simName              = "i_main_spatial1m5x_SIqpOff%d" % SIqpOff,             # need to have the looped variable in the name
                     encoderExe           = "../hevc_transcoding-eplkan/bin/TranscoderStatic_12tap_6b_singlam",
                     decoderExe           = "../HM-12.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../hevc_transcoding-eplkan/cfg/encoder_intra_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SPATIAL_1m5X_TRN, #SHVC_SPATIAL_1m5X,
                     maxNumCodedFrames    = 1,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_intra",                     
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False],                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements
                     cfgFileMainTrn        = "cfg_shvc/transcoding.cfg")                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)

    # start simulations
    simimain.confMain['ConformanceMode0'] = 1
    simimain.confMain['ConformanceMode1'] = 1
    simimain.confMain['AvcBase'] = 0    
    simimain.confMain['NumLayers'] = 2
    simimain.confMain['ScalabilityMask1'] = 0 # Multiview
    simimain.confMain['ScalabilityMask2'] = 1 # Scalable
    simimain.confMain['ScalabilityMask3'] = 1 # Auxiliary pictures
    simimain.confMain['AuxId1'] = 5 # TRN as side info Auxiliary pictures    
    simimain.confMain['RepFormatIdx0'] = 0
    simimain.confMain['RepFormatIdx1'] = 1
    simimain.confMain['AdaptiveResolutionChange'] = 0
    simimain.confMain['NumSamplePredRefLayers1'] = 0 # number of sample pred reference layers
    simimain.confMain['SamplePredRefLayerIds1'] = 0  # reference layer id
    simimain.confMain['NumMotionPredRefLayers1'] = 0 # number of motion pred reference layers
    simimain.confMain['MotionPredRefLayerIds1'] = 0  # reference layer id
    simimain.confMain['NumActiveRefLayers1'] = 0     # number of active reference layers
    simimain.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simimain.confMain['RDOQ0'] = 1 
    simimain.confMain['RDOQTS0'] = 1 
    simimain.confMain['RDOQ1'] = 0 
    simimain.confMain['RDOQTS1'] = 0 
    simimain.confMain['SignHideFlag0'] = 1 
    simimain.confMain['SignHideFlag1'] = 1
    simimain.confMain['IntraPeriod'] = 1
    simimain.confMain['TranscodedStreamLayer'] = 1
    simimain.confMain['SEIpictureDigest'] = 0        
    #simimain.start()


# loop over QP offsets
for SIqpOff in SIqpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "SIqpOff=%d\n" % (SIqpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+SIqpOff])
    #qpSet.append([qpBL+SIqpOff,qpBL])

    # configure simulation
    simimain2x = RunSimSHVC_TRN(simName              = "i_main_spatial2x_SIqpOff%d" % SIqpOff,             # need to have the looped variable in the name
                     encoderExe           = "../hevc_transcoding-eplkan/bin/TranscoderStatic_12tap_6b_singlam",
                     decoderExe           = "../HM-12.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../hevc_transcoding-eplkan/cfg/encoder_intra_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SPATIAL_2X_HD_TRN, #SHVC_SPATIAL_2X_CACTUS_TRN, #SHVC_SPATIAL_2X_RH, #SHVC_SPATIAL_2X_CACTUS_TRN, #SHVC_SPATIAL_2X_RH, #SHVC_SPATIAL_2X_CACTUS_TRN, #SHVC_SPATIAL_2X_KIMONO_TRN, #SHVC_SPATIAL_2X_RH, #SHVC_SPATIAL_2X_KIMONO, #SHVC_SPATIAL_2X,
                     maxNumCodedFrames    = 1,#None,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_intra_trn",        #NOTE             
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,#2                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,#2                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False],                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements
                     cfgFileMainTrn        = "cfg_shvc/transcoding.cfg")                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)

    # start simulations
    simimain2x.confMain['ConformanceMode0'] = 1
    simimain2x.confMain['ConformanceMode1'] = 1
    simimain2x.confMain['AvcBase'] = 0    
    simimain2x.confMain['NumLayers'] = 2
    simimain2x.confMain['ScalabilityMask1'] = 0 # Multiview
    simimain2x.confMain['ScalabilityMask2'] = 1 # Scalable
    simimain2x.confMain['ScalabilityMask3'] = 1 #1 Auxiliary pictures
    simimain2x.confMain['AuxId1'] = 5 # Auxiliary pictures
    simimain2x.confMain['RepFormatIdx0'] = 0
    simimain2x.confMain['RepFormatIdx1'] = 1
    simimain2x.confMain['AdaptiveResolutionChange'] = 0
    simimain2x.confMain['NumSamplePredRefLayers1'] = 0 # number of sample pred reference layers
    simimain2x.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simimain2x.confMain['NumMotionPredRefLayers1'] = 0 # number of motion pred reference layers
    simimain2x.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simimain2x.confMain['NumActiveRefLayers1'] = 0 # number of active reference layers
    simimain2x.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simimain2x.confMain['RDOQ0'] = 1 
    simimain2x.confMain['RDOQTS0'] = 1 
    simimain2x.confMain['RDOQ1'] = 0 #0 
    simimain2x.confMain['RDOQTS1'] = 0 #0 

    simimain2x.confMain['SignHideFlag0'] = 1 
    simimain2x.confMain['SignHideFlag1'] = 1#0 
    simimain2x.confMain['IntraPeriod'] = 1
    simimain2x.confMain['TranscodedStreamLayer'] = 1
    simimain2x.confMain['SEIpictureDigest'] = 0        
    #simimain2x.start()

# loop over QP offsets
for SIqpOff in SIqpOffsetSNR:

  # create QP set
  for qpBL in qpSetBLSNR:
    print "SIqpOff=%d\n" % (SIqpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+SIqpOff])
    #qpSet.append([qpBL+SIqpOff,qpBL])

    # configure simulation
    simimainsnr = RunSimSHVC_TRN(simName              = "i_main_snr_SIqpOff%d" % SIqpOff,             # need to have the looped variable in the name
                     encoderExe           = "../hevc_transcoding-eplkan/bin/TranscoderStatic_12tap_6b_singlam",
                     decoderExe           = "../HM-12.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../hevc_transcoding-eplkan/cfg/encoder_intra_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SNR_HD_TRN,#SHVC_SNR_BDRILL, #KIMONO, #BDRILL, #.SHVC_SNR,
                     maxNumCodedFrames    = 1,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_intra",                     
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False],                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements
                     cfgFileMainTrn        = "cfg_shvc/transcoding.cfg")                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
    #sim.confMain['NumberReferenceFrames'] = 0

    # start simulations
    # optional
    simimainsnr.confMain['ConformanceMode0'] = 1
    simimainsnr.confMain['ConformanceMode1'] = 1
    simimainsnr.confMain['AvcBase'] = 0    
    simimainsnr.confMain['NumLayers'] = 2
    simimainsnr.confMain['ScalabilityMask1'] = 0 # Multiview
    simimainsnr.confMain['ScalabilityMask2'] = 1 # Scalable
    simimainsnr.confMain['ScalabilityMask3'] = 1 #1 Auxiliary pictures
    simimainsnr.confMain['AuxId1'] = 5 # Auxiliary pictures
    #
    simimainsnr.confMain['RepFormatIdx0'] = 0
    simimainsnr.confMain['RepFormatIdx1'] = 0
    simimainsnr.confMain['AdaptiveResolutionChange'] = 0
    simimainsnr.confMain['NumSamplePredRefLayers1'] = 0 # number of sample pred reference layers
    simimainsnr.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simimainsnr.confMain['NumMotionPredRefLayers1'] = 0 # number of motion pred reference layers
    simimainsnr.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simimainsnr.confMain['NumActiveRefLayers1'] = 0 # number of active reference layers
    simimainsnr.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simimainsnr.confMain['IntraPeriod'] = 1
    simimainsnr.confMain['RDOQ0'] = 1 # just for simplicity
    simimainsnr.confMain['RDOQTS0'] = 1 # just for simplicity
    simimainsnr.confMain['RDOQ1'] = 0 # just for simplicity
    simimainsnr.confMain['RDOQTS1'] = 0 # just for simplicity

    simimainsnr.confMain['SignHideFlag0'] = 1 # just for simplicity
    simimainsnr.confMain['SignHideFlag1'] = 1# just for simplicity
    simimainsnr.confMain['TranscodedStreamLayer'] = 1
    simimainsnr.confMain['SEIpictureDigest'] = 0        
    
    #simimainsnr.start()

# loop over QP offsets
for SIqpOff in SIqpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "SIqpOff=%d\n" % (SIqpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+SIqpOff])
    #qpSet.append([qpBL+SIqpOff,qpBL])

    # configure simulation
    simramain = RunSimSHVC_TRN(simName              = "ra_main_spatial1m5x_SIqpOff%d" % SIqpOff,             # need to have the looped variable in the name
                     encoderExe           = "../hevc_transcoding-eplkan/bin/TranscoderStatic_12tap_6b_singlam",
                     decoderExe           = "../HM-12.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../hevc_transcoding-eplkan/cfg/encoder_randomaccess_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SPATIAL_1m5X_TRN,
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
                     jobParamsLsfOverride = ['intel', 'sim', False],                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements
                     cfgFileMainTrn        = "cfg_shvc/transcoding.cfg")                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)

    # start simulations
    simramain.confMain['ConformanceMode0'] = 1
    simramain.confMain['ConformanceMode1'] = 1
    simramain.confMain['AvcBase'] = 0    
    simramain.confMain['NumLayers'] = 2
    simramain.confMain['ScalabilityMask1'] = 0 # Multiview
    simramain.confMain['ScalabilityMask2'] = 1 # Scalable
    simramain.confMain['ScalabilityMask3'] = 1 # Auxiliary pictures
    simramain.confMain['AuxId1'] = 5 # Auxiliary pictures
    simramain.confMain['RepFormatIdx0'] = 0
    simramain.confMain['RepFormatIdx1'] = 1
    simramain.confMain['AdaptiveResolutionChange'] = 0
    simramain.confMain['NumSamplePredRefLayers1'] = 0 # number of sample pred reference layers
    simramain.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simramain.confMain['NumMotionPredRefLayers1'] = 0 # number of motion pred reference layers
    simramain.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simramain.confMain['NumActiveRefLayers1'] = 0 # number of active reference layers
    simramain.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simramain.confMain['RDOQ0'] = 1 
    simramain.confMain['RDOQTS0'] = 1 
    simramain.confMain['RDOQ1'] = 0 
    simramain.confMain['RDOQTS1'] = 0 

    simramain.confMain['SignHideFlag0'] = 1 
    simramain.confMain['SignHideFlag1'] = 1
    simramain.confMain['IntraPeriod'] = 30 # will be adjusted by sequences specific intra period
    simramain.confMain['TranscodedStreamLayer'] = 1
    simramain.confMain['SEIpictureDigest'] = 0        
    simramain.start()


# loop over QP offsets
for SIqpOff in SIqpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "SIqpOff=%d\n" % (SIqpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+SIqpOff])
    #qpSet.append([qpBL+SIqpOff,qpBL])

    # configure simulation
    simramain2x = RunSimSHVC_TRN(simName              = "ra_main_spatial2x_SIqpOff%d_2" % SIqpOff,             # need to have the looped variable in the name
                     encoderExe           = "../hevc_transcoding-eplkan/bin/TranscoderStatic_12tap_6b_singlam",
                     decoderExe           = "../HM-12.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../hevc_transcoding-eplkan/cfg/encoder_randomaccess_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SPATIAL_2X_HD_TRN,
                     maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 1,
                     cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess_trn",                     
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['intel', 'sim', False],                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements
                     cfgFileMainTrn        = "cfg_shvc/transcoding.cfg")                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)

    # start simulations
    simramain2x.confMain['ConformanceMode0'] = 1
    simramain2x.confMain['ConformanceMode1'] = 1
    simramain2x.confMain['AvcBase'] = 0    
    simramain2x.confMain['NumLayers'] = 2
    simramain2x.confMain['ScalabilityMask1'] = 0 # Multiview
    simramain2x.confMain['ScalabilityMask2'] = 1 # Scalable
    simramain2x.confMain['ScalabilityMask3'] = 1 # Auxiliary pictures
    simramain2x.confMain['AuxId1'] = 5 # Auxiliary pictures
    simramain2x.confMain['RepFormatIdx0'] = 0
    simramain2x.confMain['RepFormatIdx1'] = 1
    simramain2x.confMain['AdaptiveResolutionChange'] = 0
    simramain2x.confMain['NumSamplePredRefLayers1'] = 0  # number of sample pred reference layers
    simramain2x.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simramain2x.confMain['NumMotionPredRefLayers1'] = 0 # number of motion pred reference layers
    simramain2x.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simramain2x.confMain['NumActiveRefLayers1'] = 0 # number of active reference layers
    simramain2x.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simramain2x.confMain['RDOQ0'] = 1 
    simramain2x.confMain['RDOQTS0'] = 1 
    simramain2x.confMain['RDOQ1'] = 0 
    simramain2x.confMain['RDOQTS1'] = 0 

    simramain2x.confMain['SignHideFlag0'] = 1 
    simramain2x.confMain['SignHideFlag1'] = 1
    simramain2x.confMain['IntraPeriod'] = 30 # will be adjusted by sequences specific intra period
    simramain2x.confMain['TranscodedStreamLayer'] = 1
    simramain2x.confMain['SEIpictureDigest'] = 0        
    simramain2x.start()

# loop over QP offsets
for SIqpOff in SIqpOffset:

  # create QP set
  for qpBL in qpSetBL:
    print "SIqpOff=%d\n" % (SIqpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+SIqpOff])
    #qpSet.append([qpBL+SIqpOff,qpBL])

    # configure simulation
    simramain3x = RunSimSHVC_TRN(simName              = "ra_main_spatial3x_SIqpOff%d" % SIqpOff,             # need to have the looped variable in the name
                     encoderExe           = "../hevc_transcoding-eplkan/bin/TranscoderStatic_12tap_6b_singlam",
                     decoderExe           = "../HM-12.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../hevc_transcoding-eplkan/cfg/encoder_randomaccess_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SPATIAL_3X_HD_TRN, #SHVC_SPATIAL_1m5X_TRN,
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
                     jobParamsLsfOverride = ['intel', 'sim', False],                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements
                     cfgFileMainTrn        = "cfg_shvc/transcoding.cfg")                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)

    # start simulations
    simramain3x.confMain['ConformanceMode0'] = 1
    simramain3x.confMain['ConformanceMode1'] = 1
    simramain3x.confMain['AvcBase'] = 0    
    simramain3x.confMain['NumLayers'] = 2
    simramain3x.confMain['ScalabilityMask1'] = 0 # Multiview
    simramain3x.confMain['ScalabilityMask2'] = 1 # Scalable
    simramain3x.confMain['ScalabilityMask3'] = 1 # Auxiliary pictures
    simramain3x.confMain['AuxId1'] = 5 # Auxiliary pictures
    simramain3x.confMain['RepFormatIdx0'] = 0
    simramain3x.confMain['RepFormatIdx1'] = 1
    simramain3x.confMain['AdaptiveResolutionChange'] = 0
    simramain3x.confMain['NumSamplePredRefLayers1'] = 0 # number of sample pred reference layers
    simramain3x.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simramain3x.confMain['NumMotionPredRefLayers1'] = 0 # number of motion pred reference layers
    simramain3x.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simramain3x.confMain['NumActiveRefLayers1'] = 0 # number of active reference layers
    simramain3x.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simramain3x.confMain['RDOQ0'] = 1 
    simramain3x.confMain['RDOQTS0'] = 1 
    simramain3x.confMain['RDOQ1'] = 0 
    simramain3x.confMain['RDOQTS1'] = 0 

    simramain3x.confMain['SignHideFlag0'] = 1 
    simramain3x.confMain['SignHideFlag1'] = 1
    simramain3x.confMain['IntraPeriod'] = 30 # will be adjusted by sequences specific intra period
    simramain3x.confMain['TranscodedStreamLayer'] = 1
    simramain3x.confMain['SEIpictureDigest'] = 0        
    simramain3x.start()

# loop over QP offsets
for SIqpOff in SIqpOffsetSNR:

  # create QP set
  for qpBL in qpSetBLSNR:
    print "SIqpOff=%d\n" % (SIqpOff)
    qpSet = []
    qpSet.append([qpBL,qpBL+SIqpOff])
    #qpSet.append([qpBL+SIqpOff,qpBL])

    # configure simulation
    simramainsnr = RunSimSHVC_TRN(simName              = "ra_main_snr_SIqpOff%d" % SIqpOff,             # need to have the looped variable in the name
                     encoderExe           = "../hevc_transcoding-eplkan/bin/TranscoderStatic_12tap_6b_singlam",
                     decoderExe           = "../HM-12.0/bin/TAppDecoderStatic",
                     extractorExe         = None,
                     cfgFileMain          = "../hevc_transcoding-eplkan/cfg/encoder_randomaccess_main.cfg",
                     cfgFileLayers        = ['cfg_shvc/layer0.cfg', 'cfg_shvc/layer1.cfg'], # Not used
                     testSet              = testSeqs.SHVC_SNR_HD_TRN, #SHVC_SNR,
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
                     jobParamsLsfOverride = ['intel', 'sim', False],                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements
                     cfgFileMainTrn        = "cfg_shvc/transcoding.cfg")                 # only for LSF cluster: platform, queue, send results by email
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
    simramainsnr.confMain['ScalabilityMask1'] = 0 # Multiview
    simramainsnr.confMain['ScalabilityMask2'] = 1 # Scalable
    simramainsnr.confMain['ScalabilityMask3'] = 1 # Auxiliary pictures
    simramainsnr.confMain['AuxId1'] = 5 # Auxiliary pictures
    simramainsnr.confMain['RepFormatIdx0'] = 0
    simramainsnr.confMain['RepFormatIdx1'] = 0
    simramainsnr.confMain['AdaptiveResolutionChange'] = 0
    simramainsnr.confMain['NumSamplePredRefLayers1'] = 0 # number of sample pred reference layers
    simramainsnr.confMain['SamplePredRefLayerIds1'] = 0 # reference layer id
    simramainsnr.confMain['NumMotionPredRefLayers1'] = 0 # number of motion pred reference layers
    simramainsnr.confMain['MotionPredRefLayerIds1'] = 0 # reference layer id
    simramainsnr.confMain['NumActiveRefLayers1'] = 0 # number of active reference layers
    simramainsnr.confMain['PredLayerIds1'] = 0 # inter-layer prediction layer index within available reference layers
    simramainsnr.confMain['IntraPeriod'] = 30 # will be adjusted by sequences specific intra period
    simramainsnr.confMain['RDOQ0'] = 1 
    simramainsnr.confMain['RDOQTS0'] = 1 
    simramainsnr.confMain['RDOQ1'] = 0 
    simramainsnr.confMain['RDOQTS1'] = 0 

    simramainsnr.confMain['SignHideFlag0'] = 1 
    simramainsnr.confMain['SignHideFlag1'] = 1
    simramainsnr.confMain['TranscodedStreamLayer'] = 1
    simramainsnr.confMain['SEIpictureDigest'] = 0        
    #simramainsnr.start()



