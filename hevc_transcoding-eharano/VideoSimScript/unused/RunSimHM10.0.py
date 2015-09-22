from RunSimDefinitions import RunSimSamsung


import testSeqs





# configure simulation


# main 8bit


simramain = RunSimSamsung(simName              = "ra_main",


                                   encoderExe           = "../HM-10.0/bin/TAppEncoderStatic",


                                   decoderExe           = "../HM-10.0/bin/TAppDecoderStatic",


                                   cfgFileMain          = "../HM-10.0/cfg/encoder_randomaccess_main.cfg",


                                   testSet              = testSeqs.JCTVC_RA_HM6_ALL,


                                   #testSet              = testSeqs.VCEGMPEG_ClassF,


                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N


                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding


                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration


                                   allowSeqOverride     = 1,


                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess",


                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg


                                   decode               = 1,


                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email








                                                                                             








simldmain = RunSimSamsung(simName              = "ld_main",


                                   encoderExe           = "../HM-10.0/bin/TAppEncoderStatic",


                                   decoderExe           = "../HM-10.0/bin/TAppDecoderStatic",


                                   cfgFileMain          = "../HM-10.0/cfg/encoder_lowdelay_main.cfg",


                                   testSet              = testSeqs.JCTVC_LD_HM6_ALL,


                                   #testSet              = testSeqs.VCEGMPEG_ClassF,


                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N


                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding


                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration


                                   allowSeqOverride     = 1,


                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",


                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg


                                   decode               = 1,


                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email








simldPmain = RunSimSamsung(simName              = "ld_P_main",


                                   encoderExe           = "../HM-10.0/bin/TAppEncoderStatic",


                                   decoderExe           = "../HM-10.0/bin/TAppDecoderStatic",


                                   cfgFileMain          = "../HM-10.0/cfg/encoder_lowdelay_P_main.cfg",


                                   testSet              = testSeqs.JCTVC_LD_HM6_ALL,


                                   #testSet              = testSeqs.VCEGMPEG_ClassF,


                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N


                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding


                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration


                                   allowSeqOverride     = 1,


                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",


                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg


                                   decode               = 1,


                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email











simimain = RunSimSamsung(simName              = "i_main",


                                   encoderExe           = "../HM-10.0/bin/TAppEncoderStatic",


                                   decoderExe           = "../HM-10.0/bin/TAppDecoderStatic",


                                   cfgFileMain          = "../HM-10.0/cfg/encoder_intra_main.cfg",


                                   testSet              = testSeqs.JCTVC_HM6_ALL,


                                   #testSet              = testSeqs.VCEGMPEG_ClassF,


                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N


                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding


                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration


                                   allowSeqOverride     = 1,


                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_intra",


                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg


                                   decode               = 1,


                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email




#10bit


simra10bit = RunSimSamsung(simName              = "ra_main10",


                                   encoderExe           = "../HM-10.0/bin/TAppEncoderStatic",


                                   decoderExe           = "../HM-10.0/bin/TAppDecoderStatic",


                                   cfgFileMain          = "../HM-10.0/cfg/encoder_randomaccess_main10.cfg",


                                   testSet              = testSeqs.JCTVC_RA_HM6_ALL,


                                   #testSet              = testSeqs.VCEGMPEG_ClassF, 


                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N


                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding


                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration


                                   allowSeqOverride     = 1,


                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_randomaccess",


                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg


                                   decode               = 1,


                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email




simld10bit = RunSimSamsung(simName              = "ld_main10",


                                   encoderExe           = "../HM-10.0/bin/TAppEncoderStatic",


                                   decoderExe           = "../HM-10.0/bin/TAppDecoderStatic",


                                   cfgFileMain          = "../HM-10.0/cfg/encoder_lowdelay_main10.cfg",


                                   testSet              = testSeqs.JCTVC_LD_HM6_ALL,

                                   #testSet              = testSeqs.VCEGMPEG_ClassF,


                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N


                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding


                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration


                                   allowSeqOverride     = 1,


                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",


                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg


                                   decode               = 1,


                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email




simldP10bit = RunSimSamsung(simName              = "ld_P_main10",


                                   encoderExe           = "../HM-10.0/bin/TAppEncoderStatic",


                                   decoderExe           = "../HM-10.0/bin/TAppDecoderStatic",


                                   cfgFileMain          = "../HM-10.0/cfg/encoder_lowdelay_P_main10.cfg",


                                   testSet              = testSeqs.JCTVC_LD_HM6_ALL,

                                   #testSet              = testSeqs.VCEGMPEG_ClassF,


                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N


                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding


                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration


                                   allowSeqOverride     = 1,


                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_lowdelay",


                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg


                                   decode               = 1,


                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email





simi10bit = RunSimSamsung(simName              = "i_main10",


                                   encoderExe           = "../HM-10.0/bin/TAppEncoderStatic",


                                   decoderExe           = "../HM-10.0/bin/TAppDecoderStatic",


                                   cfgFileMain          = "../HM-10.0/cfg/encoder_intra_main10.cfg",


                                   testSet              = testSeqs.JCTVC_HM6_ALL,

                                   #testSet              = testSeqs.VCEGMPEG_ClassF,


                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N


                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding


                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration


                                   allowSeqOverride     = 1,


                                   cfgSeqDirOverride    = "cfg_samsung/cfg_seq_intra",


                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg


                                   decode               = 1,


                                   removeEnc            = 0,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email





# optionally change config file parameters


# NOTE: sequence properties (number of frames, frame sizes etc.)


#       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)


#sim.confMain['SearchMode'] = 1


# command parameters can be modified by setting parameters to 'commandOpts'


#sim.confMain['commandOpts'] = '-s 64'



# start simulations

#NOTE MD5 is turned on here, to turn off set SEIpictureDigest to 0  
# .confMain['SEIpictureDigest'] = 0

# main 8 bit
#simramain.confMain['SEIpictureDigest'] = 1
simramain.start()
#simldmain.confMain['SEIpictureDigest'] = 1
simldmain.start()
#simimain.confMain['SEIpictureDigest'] = 1
simimain.start()


# main 10 bit
#simra10bit.confMain['SEIpictureDigest'] = 1
simra10bit.start()
#simld10bit.confMain['SEIpictureDigest'] = 1
simld10bit.start()
#simi10bit.confMain['SEIpictureDigest'] = 1
simi10bit.start()

# optional condition
#simldPmain.confMain['SEIpictureDigest'] = 1
simldPmain.start()
#simP10bit.confMain['SEIpictureDigest'] = 1
simldP10bit.start()









