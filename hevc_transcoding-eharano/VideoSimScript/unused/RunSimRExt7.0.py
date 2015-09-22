from RunSimDefinitions import RunSimSamsungimport testSeqs# configure simulation# YUV  8bitsimramain = RunSimSamsung(simName              = "ra_main_rext",                                   encoderExe           = "../HM-14.0+RExt-7.0/bin/TAppEncoderStatic",                                   decoderExe           = "../HM-14.0+RExt-7.0/bin/TAppDecoderStatic",                                   cfgFileMain          = "../HM-14.0+RExt-7.0/cfg/encoder_randomaccess_main_rext.cfg",                                   testSet              = testSeqs.sc_all_yuv,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email                                                                                             simldmain = RunSimSamsung(simName              = "ld_main_rext",                                   encoderExe           = "../HM-14.0+RExt-7.0/bin/TAppEncoderStatic",                                   decoderExe           = "../HM-14.0+RExt-7.0/bin/TAppDecoderStatic",                                   cfgFileMain          = "../HM-14.0+RExt-7.0/cfg/encoder_lowdelay_main_rext.cfg",                                   testSet              = testSeqs.sc_all_yuv,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by emailsimimain = RunSimSamsung(simName              = "i_main_rext",                                   encoderExe           = "../HM-14.0+RExt-7.0/bin/TAppEncoderStatic",                                   decoderExe           = "../HM-14.0+RExt-7.0/bin/TAppDecoderStatic",                                   cfgFileMain          = "../HM-14.0+RExt-7.0/cfg/encoder_intra_main_rext.cfg",                                   testSet              = testSeqs.sc_all_yuv,                                    maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_intra",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email# RGB  8bitsimramainrgb = RunSimSamsung(simName              = "ra_rgb_rext",                                   encoderExe           = "../HM-14.0+RExt-7.0/bin/TAppEncoderStatic",                                   decoderExe           = "../HM-14.0+RExt-7.0/bin/TAppDecoderStatic",                                   cfgFileMain          = "../HM-14.0+RExt-7.0/cfg/encoder_randomaccess_main_rext.cfg",                                   testSet              = testSeqs.sc_all_rgb,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email                                                                                             simldmainrgb = RunSimSamsung(simName              = "ld_rgb_rext",                                   encoderExe           = "../HM-14.0+RExt-7.0/bin/TAppEncoderStatic",                                   decoderExe           = "../HM-14.0+RExt-7.0/bin/TAppDecoderStatic",                                   cfgFileMain          = "../HM-14.0+RExt-7.0/cfg/encoder_lowdelay_main_rext.cfg",                                   testSet              = testSeqs.sc_all_rgb,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by emailsimimainrgb = RunSimSamsung(simName              = "i_rgb_rext",                                   encoderExe           = "../HM-14.0+RExt-7.0/bin/TAppEncoderStatic",                                   decoderExe           = "../HM-14.0+RExt-7.0/bin/TAppDecoderStatic",                                   cfgFileMain          = "../HM-14.0+RExt-7.0/cfg/encoder_intra_main_rext.cfg",                                   testSet              = testSeqs.sc_all_rgb,                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration                                   allowSeqOverride     = 1,                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_intra",                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg                                   decode               = 1,                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email# optionally change config file parameters# NOTE: sequence properties (number of frames, frame sizes etc.)#       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)#sim.confMain['SearchMode'] = 1# command parameters can be modified by setting parameters to 'commandOpts'#sim.confMain['commandOpts'] = '-s 64'# start simulations# yuv rext simramain.confMain['InputChromaFormat'] = '444'simramain.confMain['SEIDecodedPictureHash'] = 1simramain.confMain['MSEBasedSequencePSNR'] = 1simramain.start()simldmain.confMain['InputChromaFormat'] = '444'simldmain.confMain['SEIDecodedPictureHash'] = 1simldmain.confMain['MSEBasedSequencePSNR'] = 1simldmain.start()simimain.confMain['InputChromaFormat'] = '444'simimain.confMain['SEIDecodedPictureHash'] = 1simimain.confMain['MSEBasedSequencePSNR'] = 1simimain.start()# rgb rext simramainrgb.confMain['InputChromaFormat'] = '444'simramainrgb.confMain['InputColourSpaceConvert'] = 'RGBtoGBR'simramainrgb.confMain['OutputColourSpaceConvert'] = 'GBRtoRGB'simramainrgb.confMain['SNRInternalColourSpace'] = 1simramainrgb.confMain['OutputInternalColourSpace'] = 0simramainrgb.confMain['SEIDecodedPictureHash'] = 1simramainrgb.confMain['MSEBasedSequencePSNR'] = 1simramainrgb.start()simldmainrgb.confMain['InputChromaFormat'] = '444'simldmainrgb.confMain['InputColourSpaceConvert'] = 'RGBtoGBR'simldmainrgb.confMain['OutputColourSpaceConvert'] = 'GBRtoRGB'simldmainrgb.confMain['SNRInternalColourSpace'] = 1simldmainrgb.confMain['OutputInternalColourSpace'] = 0simldmainrgb.confMain['SEIDecodedPictureHash'] = 1simldmainrgb.confMain['MSEBasedSequencePSNR'] = 1simldmainrgb.start()simimainrgb.confMain['InputChromaFormat'] = '444'simimainrgb.confMain['InputColourSpaceConvert'] = 'RGBtoGBR'simimainrgb.confMain['OutputColourSpaceConvert'] = 'GBRtoRGB'simimainrgb.confMain['SNRInternalColourSpace'] = 1simimainrgb.confMain['OutputInternalColourSpace'] = 0simimainrgb.confMain['SEIDecodedPictureHash'] = 1simimainrgb.confMain['MSEBasedSequencePSNR'] = 1simimainrgb.start()