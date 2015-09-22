from RunSimDefinitions import RunSimC65


import testSeqs





# configure simulation



simldPmainperf = RunSimC65(simName              = "ld_P_main_perf",


                                   encoderExe           = "../c65/bin/c65_app",


                                   decoderExe           = "../HM-9.0/bin/TAppDecoderStatic",


                                   cfgFileMain          = "../c65/cfg/example.cfg",


                                   testSet              = testSeqs.c65_perf,



                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N


                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding


                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration


                                   allowSeqOverride     = 1,


                                   cfgSeqDirOverride    = None, #"cfg_samsung/cfg_seq_lowdelay", #sequence specific things can be added here


                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg


                                   decode               = 1,


                                   removeEnc            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeRec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeDec            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email



simldPmainspeed = RunSimC65(simName              = "ld_P_main_speed",


                                   encoderExe           = "../c65/bin/c65_app",


                                   decoderExe           = "../HM-9.0/bin/TAppDecoderStatic",


                                   cfgFileMain          = "../c65/cfg/example.cfg",


                                   testSet              = testSeqs.c65_speed,



                                   maxNumCodedFrames    = None,                                      # None: do nothing, N: restrict number of coded frames to N


                                   temporalSubsampling  = None,                                   # None: do nothing, N: temporally subsample by N before coding


                                   qpSet                = [[37], [32], [27], [22]],               # may be overriden by sequence specific configuration


                                   allowSeqOverride     = 1,


                                   cfgSeqDirOverride    = None, #"cfg_samsung/cfg_seq_lowdelay", #sequence specific things can be added here


                                   qpNum                = -1,                                     # -1: as in qpSet or seq cfg, 0/1/2/3: select one qp out of qpSet or seq cfg


                                   decode               = 0,


                                   removeEnc            = 2,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeRec            = 4,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   removeDec            = 4,                                      # 0: keep, 1: use project disk and remove when done, 2: use /tmp


                                   jobParamsLsfOverride = ['intel', 'sim', False])                 # only for LSF cluster: platform, queue, send results by email



# optionally change config file parameters


# NOTE: sequence properties (number of frames, frame sizes etc.)


#       as well as file names, qp settings etc. should not be modified here (these will be overwritten in the script)


#sim.confMain['SearchMode'] = 1


# command parameters can be modified by setting parameters to 'commandOpts'


#sim.confMain['commandOpts'] = '-s 64'



# start simulations

simldPmainperf.start()
simldPmainspeed.start()








