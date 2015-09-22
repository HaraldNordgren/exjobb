from RunSimDefinitions import RunSimSamsungimport testSeqs# configure simulation# lossy YUV and RGB# YUV  8bitsimramain = RunSimSamsung(simName              = "ra_yuv",                                   encoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppEncoderStaticSCM1.0",                                   decoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppDecoderStaticSCM1.0",                                   cfgFileMain          = "../HM-14.0+RExt-7.0+SCM-1.0/cfg/encoder_randomaccess_main_scc.cfg",                                   testSet              = testSeqs.sc_all_yuv,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email                                                                                             simldmain = RunSimSamsung(simName              = "ld_yuv",                                   encoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppEncoderStaticSCM1.0",                                   decoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppDecoderStaticSCM1.0",                                   cfgFileMain          = "../HM-14.0+RExt-7.0+SCM-1.0/cfg/encoder_lowdelay_main_scc.cfg",                                   testSet              = testSeqs.sc_all_yuv,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by emailsimimain = RunSimSamsung(simName              = "i_yuv",                                   encoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppEncoderStaticSCM1.0",                                   decoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppDecoderStaticSCM1.0",                                   cfgFileMain          = "../HM-14.0+RExt-7.0+SCM-1.0/cfg/encoder_intra_main_scc.cfg",                                   testSet              = testSeqs.sc_all_yuv,                                    maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_intra",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email# RGB  8bitsimramainrgb = RunSimSamsung(simName              = "ra_rgb",                                   encoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppEncoderStaticSCM1.0",                                   decoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppDecoderStaticSCM1.0",                                   cfgFileMain          = "../HM-14.0+RExt-7.0+SCM-1.0/cfg/encoder_randomaccess_main_scc.cfg",                                   testSet              = testSeqs.sc_all_rgb,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email                                                                                             simldmainrgb = RunSimSamsung(simName              = "ld_rgb",                                   encoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppEncoderStaticSCM1.0",                                   decoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppDecoderStaticSCM1.0",                                   cfgFileMain          = "../HM-14.0+RExt-7.0+SCM-1.0/cfg/encoder_lowdelay_main_scc.cfg",                                   testSet              = testSeqs.sc_all_rgb,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by emailsimimainrgb = RunSimSamsung(simName              = "i_rgb",                                   encoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppEncoderStaticSCM1.0",                                   decoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppDecoderStaticSCM1.0",                                   cfgFileMain          = "../HM-14.0+RExt-7.0+SCM-1.0/cfg/encoder_intra_main_scc.cfg",                                   testSet              = testSeqs.sc_all_rgb,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_intra",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email# lossless YUV and RGB# YUV  8bitsimramainll = RunSimSamsung(simName              = "ra_ll_yuv",                                   encoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppEncoderStaticSCM1.0",                                   decoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppDecoderStaticSCM1.0",                                   cfgFileMain          = "../HM-14.0+RExt-7.0+SCM-1.0/cfg/encoder_randomaccess_main_scc.cfg",                                   testSet              = testSeqs.sc_all_yuv,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[0]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email                                                                                             simldmainll = RunSimSamsung(simName              = "ld_ll_yuv",                                   encoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppEncoderStaticSCM1.0",                                   decoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppDecoderStaticSCM1.0",                                   cfgFileMain          = "../HM-14.0+RExt-7.0+SCM-1.0/cfg/encoder_lowdelay_main_scc.cfg",                                   testSet              = testSeqs.sc_all_yuv,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[0]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by emailsimimainll = RunSimSamsung(simName              = "i_ll_yuv",                                   encoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppEncoderStaticSCM1.0",                                   decoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppDecoderStaticSCM1.0",                                   cfgFileMain          = "../HM-14.0+RExt-7.0+SCM-1.0/cfg/encoder_intra_main_scc.cfg",                                   testSet              = testSeqs.sc_all_yuv,                                    maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[0]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_intra",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email# RGB  8bitsimramainrgbll = RunSimSamsung(simName              = "ra_ll_rgb",                                   encoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppEncoderStaticSCM1.0",                                   decoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppDecoderStaticSCM1.0",                                   cfgFileMain          = "../HM-14.0+RExt-7.0+SCM-1.0/cfg/encoder_randomaccess_main_scc.cfg",                                   testSet              = testSeqs.sc_all_rgb,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[0]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email                                                                                             simldmainrgbll = RunSimSamsung(simName              = "ld_ll_rgb",                                   encoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppEncoderStaticSCM1.0",                                   decoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppDecoderStaticSCM1.0",                                   cfgFileMain          = "../HM-14.0+RExt-7.0+SCM-1.0/cfg/encoder_lowdelay_main_scc.cfg",                                   testSet              = testSeqs.sc_all_rgb,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[0]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by emailsimimainrgbll = RunSimSamsung(simName              = "i_ll_rgb",                                   encoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppEncoderStaticSCM1.0",                                   decoderExe           = "../HM-14.0+RExt-7.0+SCM-1.0/bin/TAppDecoderStaticSCM1.0",                                   cfgFileMain          = "../HM-14.0+RExt-7.0+SCM-1.0/cfg/encoder_intra_main_scc.cfg",                                   testSet              = testSeqs.sc_all_rgb,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[0]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_intra",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email# optionally change config file parameters# NOTE: sequence properties (number of frames, frame sizes etc.)#       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)#sim.confMain['SearchMode'] = 1# command parameters can be modified by setting parameters to 'commandOpts'#sim.confMain['commandOpts'] = '-s 64'# start simulations#lossy# yuv scc simimain.confMain['InputChromaFormat'] = '444'simimain.confMain['SEIDecodedPictureHash'] = 1simimain.confMain['MSEBasedSequencePSNR'] = 1simimain.start()simramain.confMain['InputChromaFormat'] = '444'simramain.confMain['SEIDecodedPictureHash'] = 1simramain.confMain['MSEBasedSequencePSNR'] = 1simramain.start()simldmain.confMain['InputChromaFormat'] = '444'simldmain.confMain['SEIDecodedPictureHash'] = 1simldmain.confMain['MSEBasedSequencePSNR'] = 1simldmain.start()# rgb scc simramainrgb.confMain['InputChromaFormat'] = '444'simramainrgb.confMain['InputColourSpaceConvert'] = 'RGBtoGBR'simramainrgb.confMain['OutputColourSpaceConvert'] = 'GBRtoRGB'simramainrgb.confMain['SNRInternalColourSpace'] = 1simramainrgb.confMain['OutputInternalColourSpace'] = 0simramainrgb.confMain['SEIDecodedPictureHash'] = 1simramainrgb.confMain['MSEBasedSequencePSNR'] = 1simramainrgb.start()simldmainrgb.confMain['InputChromaFormat'] = '444'simldmainrgb.confMain['InputColourSpaceConvert'] = 'RGBtoGBR'simldmainrgb.confMain['OutputColourSpaceConvert'] = 'GBRtoRGB'simldmainrgb.confMain['SNRInternalColourSpace'] = 1simldmainrgb.confMain['OutputInternalColourSpace'] = 0simldmainrgb.confMain['SEIDecodedPictureHash'] = 1simldmainrgb.confMain['MSEBasedSequencePSNR'] = 1simldmainrgb.start()simimainrgb.confMain['InputChromaFormat'] = '444'simimainrgb.confMain['InputColourSpaceConvert'] = 'RGBtoGBR'simimainrgb.confMain['OutputColourSpaceConvert'] = 'GBRtoRGB'simimainrgb.confMain['SNRInternalColourSpace'] = 1simimainrgb.confMain['OutputInternalColourSpace'] = 0simimainrgb.confMain['SEIDecodedPictureHash'] = 1simimainrgb.confMain['MSEBasedSequencePSNR'] = 1simimainrgb.start()# lossless# yuv scc simramainll.confMain['InputChromaFormat'] = '444'simramainll.confMain['SEIDecodedPictureHash'] = 1simramainll.confMain['MSEBasedSequencePSNR'] = 1simramainll.confMain['CostMode'] = 'lossless'simramainll.confMain['IntraReferenceSmoothing'] = 0simramainll.confMain['CUTransquantBypassFlagForce'] = 1simramainll.confMain['TransquantBypassEnableFlag'] = 1simramainll.start()simldmainll.confMain['InputChromaFormat'] = '444'simldmainll.confMain['SEIDecodedPictureHash'] = 1simldmainll.confMain['MSEBasedSequencePSNR'] = 1simldmainll.confMain['CostMode'] = 'lossless'simldmainll.confMain['IntraReferenceSmoothing'] = 0simldmainll.confMain['CUTransquantBypassFlagForce'] = 1simldmainll.confMain['TransquantBypassEnableFlag'] = 1simldmainll.start()simimainll.confMain['InputChromaFormat'] = '444'simimainll.confMain['SEIDecodedPictureHash'] = 1simimainll.confMain['MSEBasedSequencePSNR'] = 1simimainll.confMain['CostMode'] = 'lossless'simimainll.confMain['IntraReferenceSmoothing'] = 0simimainll.confMain['CUTransquantBypassFlagForce'] = 1simimainll.confMain['TransquantBypassEnableFlag'] = 1simimainll.start()# rgb scc simramainrgbll.confMain['InputChromaFormat'] = '444'simramainrgbll.confMain['InputColourSpaceConvert'] = 'RGBtoGBR'simramainrgbll.confMain['OutputColourSpaceConvert'] = 'GBRtoRGB'simramainrgbll.confMain['SNRInternalColourSpace'] = 1simramainrgbll.confMain['OutputInternalColourSpace'] = 0simramainrgbll.confMain['SEIDecodedPictureHash'] = 1simramainrgbll.confMain['MSEBasedSequencePSNR'] = 1simramainrgbll.confMain['CostMode'] = 'lossless'simramainrgbll.confMain['IntraReferenceSmoothing'] = 0simramainrgbll.confMain['CUTransquantBypassFlagForce'] = 1simramainrgbll.confMain['TransquantBypassEnableFlag'] = 1simramainrgbll.start()simldmainrgbll.confMain['InputChromaFormat'] = '444'simldmainrgbll.confMain['InputColourSpaceConvert'] = 'RGBtoGBR'simldmainrgbll.confMain['OutputColourSpaceConvert'] = 'GBRtoRGB'simldmainrgbll.confMain['SNRInternalColourSpace'] = 1simldmainrgbll.confMain['OutputInternalColourSpace'] = 0simldmainrgbll.confMain['SEIDecodedPictureHash'] = 1simldmainrgbll.confMain['MSEBasedSequencePSNR'] = 1simldmainrgbll.confMain['CostMode'] = 'lossless'simldmainrgbll.confMain['IntraReferenceSmoothing'] = 0simldmainrgbll.confMain['CUTransquantBypassFlagForce'] = 1simldmainrgbll.confMain['TransquantBypassEnableFlag'] = 1simldmainrgbll.start()simimainrgbll.confMain['InputChromaFormat'] = '444'simimainrgbll.confMain['InputColourSpaceConvert'] = 'RGBtoGBR'simimainrgbll.confMain['OutputColourSpaceConvert'] = 'GBRtoRGB'simimainrgbll.confMain['SNRInternalColourSpace'] = 1simimainrgbll.confMain['OutputInternalColourSpace'] = 0simimainrgbll.confMain['SEIDecodedPictureHash'] = 1simimainrgbll.confMain['MSEBasedSequencePSNR'] = 1simimainrgbll.confMain['CostMode'] = 'lossless'simimainrgbll.confMain['IntraReferenceSmoothing'] = 0simimainrgbll.confMain['CUTransquantBypassFlagForce'] = 1simimainrgbll.confMain['TransquantBypassEnableFlag'] = 1simimainrgbll.start()