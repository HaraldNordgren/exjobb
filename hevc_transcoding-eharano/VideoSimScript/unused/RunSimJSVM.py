from RunSimDefinitions import RunSimJSVM
import testSeqs

# QP settings
qpSetEL = [38,34,30,26]
qpOffset = [-6]

# loop over QP offsets
for qpOff in qpOffset:

    # create QP set
    qpSet = []
    for qpEL in qpSetEL:
        qpSet.append([qpEL-qpOff,qpEL])

    # configure simulation
    sim = RunSimJSVM(simName              = "JSVM9.18_qpOff%d" % qpOff,             # need to have the looped variable in the name
                     encoderExe           = "bin/H264AVCEncoderLibTestStatic",
                     decoderExe           = "bin/H264AVCDecoderLibTestStatic",
                     extractorExe         = "bin/BitStreamExtractorStatic",
                     cfgFileMain          = "cfg_jsvm/cgs_gop16_ip48_sr64.cfg",
                     cfgFileLayers        = ['cfg_jsvm/baseline.BL', 'cfg_jsvm/scalable_baseline.EL'],
                     testSet              = testSeqs.svc_vt_SBB1,
                     maxNumCodedFrames    = None,                                   # None: do nothing, N: restrict number of coded frames to N
                     temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding
                     qpSet                = qpSet,                                  # may be overriden by sequence specific configuration
                     allowSeqOverride     = 0,
                     qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg
                     decode               = 1,
                     removeEnc            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp
                     removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp 
                     jobParamsLsfOverride = ['linux', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email
                                                                                    # specify 'intel' platform to make sure all jobs run on similar machines (see "lshosts | grep intel"), e.g. for encoding time measurements

    # optionally change config file parameters
    # NOTE: sequence properties (number of frames, frame sizes etc.)
    #       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)
    #sim.confMain['NumberReferenceFrames'] = 0

    # start simulations
    sim.start()
