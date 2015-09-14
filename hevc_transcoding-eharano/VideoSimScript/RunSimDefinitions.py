import os, sys, platform, math, shutil, re
import jobCtrl, cfgFile, testSeqs


class RunSimException(Exception):
    pass


class RunSim:
    """ abstract base class for RunSim functionality """

    def __init__(self, simName, encoderExe, decoderExe,
                 cfgFileMain, testSet, qpSet, allowSeqOverride, qpNum,
                 encode = 1, decode = 1, preproc = 0, postproc = 0, calcMD5CheckSums = 0,
                 removeEnc = 2, removeRec = 2, removeDec = 2, removePreproc = 2, removePostproc = 2,
                 simPostfixOverride = None,  preprocScript = None, postprocScript = None, extractorExe = None,
                 cfgFileLayers = None, cfgSeqDirOverride = None,
                 temporalSubsampling = None, maxNumCodedFrames = None,
                 resultsDirOverride = None, resultsSubDirOverride = None,
                 encDirOverride = None, recDirOverride = None, decDirOverride = None,
                 jobTypeOverride = None, jobParamsLsfOverride = None, jobSimDirOverride = None,
                 cfgFileMainTrn = None):

        # store parameters
        self._simName = simName
        if simPostfixOverride:
            self._simPostfix = simPostfixOverride
        else:
            self._simPostfix = os.path.basename(sys.argv[0]).rsplit('.',1)[0]
        self._simSettings = "%s_%s" % (self._simName, self._simPostfix)
        self._encoderExe = encoderExe
        self._decoderExe = decoderExe
        self._extractorExe = extractorExe
        self._preprocScript = preprocScript
        self._postprocScript = postprocScript
        self._cfgFileMain = cfgFileMain
        self._cfgDir = os.path.dirname(self._cfgFileMain)
        self._cfgFileMainBase = os.path.basename(self._cfgFileMain)

        self._cfgFileMainTrn = cfgFileMainTrn
        if self._cfgFileMainTrn!= None:
          self._cfgDirTrn = os.path.dirname(self._cfgFileMainTrn)
          self._cfgFileMainBaseTrn = os.path.basename(self._cfgFileMainTrn)

        self._cfgFileLayers = cfgFileLayers
        if self._cfgFileLayers is None:
            self._numLayers = 1
            self._cfgFileLayersBase = None
        else:
            self._numLayers = len(self._cfgFileLayers)
            self._cfgFileLayersBase = []
            for cfgFileLayer in self._cfgFileLayers:
                self._cfgFileLayersBase.append(os.path.basename(cfgFileLayer))
        self._testSet = testSet
        self._temporalSubsampling = temporalSubsampling
        self._maxNumCodedFrames = maxNumCodedFrames
        self._qpSet = qpSet
        self._allowSeqOverride = allowSeqOverride
        self._qpNum = qpNum
        self._encode = encode
        self._decode = decode
        self._preproc= [preproc]
        self._postproc = [postproc]
        self._calcMD5CheckSums = calcMD5CheckSums
        self._removeEnc = removeEnc
        self._removeRec = removeRec
        self._removeDec = removeDec
        self._removePreproc = removePreproc
        self._removePostproc = removePostproc


	self._current_changeQpFrameSetFlag = None
	self._current_changeQPISetFlag = None
	self._current_changeQPPSetFlag = None
	self._current_changeQPBSetFlag = None

        if cfgSeqDirOverride:
            self._cfgSeqDir = cfgSeqDirOverride
        else:
            self._cfgSeqDir = "%s/%s" % (self._cfgDir, "cfg_seq")
        if resultsDirOverride:
            self._resultsDir = resultsDirOverride
        else:
            self._resultsDir = "Results"
        if resultsSubDirOverride:
            self._resultsSubDir = resultsSubDirOverride
        else:
            self._resultsSubDir = "%s/%s" % ("rd", self._simSettings)
        if encDirOverride:
            self._encDir = encDirOverride
        else:
            self._encDir = "%s/%s/%s" % (self._resultsDir, "enc", self._simSettings)
        if recDirOverride:
            self._recDir = recDirOverride
        else:
            self._recDir = "%s/%s/%s" % (self._resultsDir, "rec", self._simSettings)
        if decDirOverride:
            self._decDir = decDirOverride
        else:
            self._decDir = "%s/%s/%s" % (self._resultsDir, "dec", self._simSettings)

        #transcoding1
        self._transDir = "%s/%s/%s" % (self._resultsDir, "trans", self._simSettings)

        if self._preproc:
            self._preprocDir = "%s/%s/%s" % (self._resultsDir, "preproc", self._simSettings)
        if self._postproc:
            self._postprocDir = "%s/%s/%s" % (self._resultsDir, "postproc", self._simSettings)
        if jobTypeOverride:
            self._jobType = jobTypeOverride
        else:
            if platform.system() == 'Linux':
                self._jobType = jobCtrl.lsfJob
            else:
                self._jobType = jobCtrl.simpleJob
        if self._jobType == jobCtrl.lsfJob or self._jobType == jobCtrl.remoteLsfJob:
            if jobParamsLsfOverride:
                self._jobParams = jobParamsLsfOverride
            else:
                self._jobParams = ['linux', 'sim', True]
        else:
            self._jobParams = []
        if jobSimDirOverride:
            self._jobSimDir = jobSimDirOverride
        else:
            self._jobSimDir = "%s/%s" % ('jobs', self._simSettings)


        # do some checks
        #print "layers=%d\n" % (self._numLayers)
        #print "length test=%d\n" % (len(self._testSet))
        #print "seq name0 =%s\n" % ( self._testSet[0])
        #print "seq name1 =%s\n" % ( self._testSet[1])

        for seqCnt in range(len(self._testSet)):
            self._checkParameter('len(self._testSet[%d])' % seqCnt, '== self._numLayers')
#        self._checkParameter('len(self._qpSet)', '== 4')
        for qpCnt in range(len(self._qpSet)):
            self._checkParameter('len(self._qpSet[%d])' % qpCnt, '== self._numLayers')
        self._checkParameter('self._allowSeqOverride', 'in [0, 1]')
        self._checkParameter('self._qpNum', 'in range(-1, len(self._qpSet))')
        self._checkParameter('self._encode', 'in [0, 1]')
        self._checkParameter('self._decode', 'in [0, 1]')
        self._checkParameter('self._preproc[0]', 'in [0, 1]')
        self._checkParameter('self._postproc[0]', 'in [0, 1]')
        self._checkParameter('self._removeEnc', 'in [0, 1, 2]')
        self._checkParameter('self._removeRec', 'in [0, 1, 2, 3, 4]')
        self._checkParameter('self._removeDec', 'in [0, 1, 2, 3, 4]')
        self._checkParameter('self._removePreproc', 'in [0, 1, 2, 3, 4]')
        self._checkParameter('self._removePostproc', 'in [0, 1, 2, 3, 4]')

        # open config files
        self._openCfgFiles()


    def _openCfgFiles(self):
        """ open encoder config files """
        self.confMain = cfgFile.cfgFile(self._cfgFileMain, self._cfgFileKeyValDelim, self._cfgFileCommDelim)
        if self._cfgFileLayers is None:
            self.confLayers = None
        else:
            self.confLayers = []
            for cfgFileLayer in self._cfgFileLayers:
                self.confLayers.append(cfgFile.cfgFile(cfgFileLayer, self._cfgFileKeyValDelim, self._cfgFileCommDelim))
        if self._cfgFileMainTrn!= None:
          self.confMainTrn = cfgFile.cfgFile(self._cfgFileMainTrn, self._cfgFileKeyValDelim, self._cfgFileCommDelim)


    def _checkParameter(self, variable, test):
        """ evaluate expression and throw exception if evaluation fails """
        if not eval(variable + ' ' + test):
            raise RunSimException("parameter check ('%s %s') failed (%s = %s)" % (variable, test, variable, eval(variable)))


    def _createDir(self, dir):
        """ create dir if it does not exist """
        if not os.path.isdir(dir):
            os.makedirs(dir)


    def _doSeqOverride(self, subGopSize):
        """ access sequences specific parameters and override qpSet and intra period if specified,
            given the gopSize and the key for intra period parameter in the main config file """
        # sequence specific overriding
        if self._allowSeqOverride:
            # Check if this an all intra test defined in the main configuration file
            # In such case it is not allowed to override the '-intra_period' parameter
            noIntra = True
            allIntra = False

            if self._current_confMain.countEntries(self._intraPeriodKey) > 0:
                intraPeriod = self._current_confMain[self._intraPeriodKey]
                if intraPeriod >= 1:
                    noIntra = False
                    if intraPeriod == 1:
                        allIntra = True

            # change confMain with definitions in confSeq
            # if qpSet is defined in cfgSeq it is used here
            cfgSeq = "%s/%s%s" % (self._cfgSeqDir, self._current_seqLayers[0].name, ".cfg")
            print "cfg seq=%s\n" % (cfgSeq)
            self._current_confMain.update(cfgSeq)
            #print "intra =%s\n" % (self._current_confMain['IntraPeriod'])

            if self._current_confMain.countEntries(self._qpSetKey) > 0:
                self._current_qpSet = self._current_confMain[self._qpSetKey]
                self._current_confMain.removeEntries(self._qpSetKey)

            if self._current_confMain.countEntries('ChangeQPFrameSet') > 0:
                self._current_changeQpFrameSet = self._current_confMain['ChangeQPFrameSet']
                self._current_confMain.removeEntries('ChangeQPFrameSet')
	        self._current_changeQpFrameSetFlag = 1

            if self._current_confMain.countEntries('ChangeQPISet') > 0:
              self._current_changeQPISet = self._current_confMain['ChangeQPISet']
              self._current_confMain.removeEntries('ChangeQPISet')
 	      self._current_changeQPISetFlag = 1

            if self._current_confMain.countEntries('ChangeQPPSet') > 0:
              self._current_changeQPPSet = self._current_confMain['ChangeQPPSet']
              self._current_confMain.removeEntries('ChangeQPPSet')
	      self._current_changeQPPSetFlag = 1

            if self._current_confMain.countEntries('ChangeQPBSet') > 0:
                self._current_changeQPBSet = self._current_confMain['ChangeQPBSet']
                self._current_confMain.removeEntries('ChangeQPBSet')
	        self._current_changeQPBSetFlag = 1

            #if noIntra:                                                    # only one I frame in original cfg file -> keep
            #    print "keep entry\n" #self._current_confMain.removeEntries(self._intraPeriodKey)
            #elif allIntra:                                                 # all intra coding in original cfg file -> keep
            #    self._current_confMain[self._intraPeriodKey] = 1
            #elif self._current_confMain.countEntries(self._intraPeriodKey) > 0:  # some other intra period in original cfg file -> adjust
            #    intraPeriod = self._current_confMain[self._intraPeriodKey]
            #    if self._keyFramesOnlyCount:
            #        intraPeriod = int(math.floor(float(intraPeriod)/float(subGopSize)))
            #    else:
            #        intraPeriod = subGopSize*(int(math.floor(float(intraPeriod)/float(subGopSize))))
            #    self._current_confMain[self._intraPeriodKey] = intraPeriod

        # adjust number of frames to be encoded
        if self._adjustFramesOut:
            frames = int(math.floor(float(self._current_seqLayers[-1].framesOut-1)/float(subGopSize) + 1.0))
            actualFrames = (frames-1)*subGopSize+1
            for layCnt in range(self._numLayers-1):
                actualFramesLayer = int(round(float(self._current_seqLayers[layCnt].framesOut) / float(self._current_seqLayers[-1].framesOut) * actualFrames))
                self._current_seqLayers[layCnt].setFramesOut(actualFramesLayer)
            self._current_seqLayers[-1].setFramesOut(actualFrames)


    def start(self):
        """ start simulations """

        # simulation configuration file basics
        self._current_conf = cfgFile.cfgFile()
        self._current_conf['encode'] = self._encode
        self._current_conf['decode'] = self._decode
        self._current_conf['preproc'] = self._preproc
        self._current_conf['postproc'] = self._postproc
        self._current_conf['preprocScript'] = self._preprocScript
        self._current_conf['postprocScript'] = self._postprocScript
        self._current_conf['calcMD5CheckSums'] = self._calcMD5CheckSums
        self._current_conf['removeEnc'] = self._removeEnc
        self._current_conf['removeRec'] = self._removeRec
        self._current_conf['removeDec'] = self._removeDec
        self._current_conf['removePreproc'] = self._removePreproc
        self._current_conf['removePostproc'] = self._removePostproc

        # create directories if necessary
        self._createDir(self._encDir)
        self._createDir(self._recDir)
        self._createDir(self._decDir)

        #transcoding 2
        self._createDir(self._transDir)

        self._createDir(self._resultsDir)
        self._createDir("%s/%s" % (self._resultsDir, self._resultsSubDir))
        if self._preproc:
            self._createDir(self._preprocDir)
        if self._postproc:
            self._createDir(self._postprocDir)

        #hej (Boolean here sets verbose mode)
        # get job ctrl and loop over sequences
        jc = jobCtrl.jobCtrl(self._jobType, self._jobSimDir, False, *self._jobParams)


        simId = []
        self._qpSetSeq = []
        for seqCnt in range(len(self._testSet)):

            # initialize one original video per layer
            self._current_seqLayers = []
            refFramesOut = None
            for layCnt in range(self._numLayers-1,-1,-1):
                seq = testSeqs.testSeq(*self._testSet[seqCnt][layCnt])
                if self._temporalSubsampling is not None:
                    seq.setFpsOut(float(seq.fpsOut)/float(self._temporalSubsampling))
                    print "applying temporal subsampling (%f), number of coded frames: %d" % (self._temporalSubsampling, seq.framesOut)
                if self._maxNumCodedFrames is not None:
                    if refFramesOut is None:
                        refFramesOut = seq.framesOut
                    maxNumCodedFrames = int(math.ceil(float(seq.framesOut) / float(refFramesOut) * self._maxNumCodedFrames))
                    if seq.framesOut > maxNumCodedFrames:
                        print "adjusting number of coded frames: %d -> %d" % (seq.framesOut, maxNumCodedFrames)
                        seq.setFramesOut(maxNumCodedFrames)
                self._current_seqLayers.insert(0, seq)

            # assign encoder config file
            self._current_confMain = self.confMain
            if self.confLayers is None:
                self._current_confLayers = None
            else:
                self._current_confLayers = []
                for confLayer in self.confLayers:
                    self._current_confLayers.append(confLayer)

            #trn config
            if self._cfgFileMainTrn != None:
              self._current_confMainTrn = self.confMainTrn
            else:
              self._current_confMainTrn = None

            # codec specific adjustment of cfg parameters (sequence specific configuration)
            self._current_qpSet = self._qpSet
            self._adjustSeqSpecCfg()

            # set qp if single qp requested
            if self._qpNum != -1:
                self._current_qpSet = [self._current_qpSet[self._qpNum]]

            # update sequence attributes
            seqLayers = []
            for layCnt in range(self._numLayers):
                seqLayers.append(self._current_seqLayers[layCnt].getAttributes())
            self._current_conf['seqLayers'] = seqLayers

            # loop over qps
            self._qpSetSeq.append(self._current_qpSet)
            simId.append([])
            for qpCnt in range(len(self._qpSetSeq[seqCnt])):

                # update sim id
                tmpId = "%sqp%d" % (self._current_seqLayers[0].name, self._qpSetSeq[seqCnt][qpCnt][0])
                for layCnt in range(1, self._numLayers):
                    tmpId = tmpId + "_%sqp%d" % (self._current_seqLayers[layCnt].name, self._qpSetSeq[seqCnt][qpCnt][layCnt])
                simId[seqCnt].append(tmpId)

                # get job instance
                job = jc.getNewJob(simId[seqCnt][qpCnt])
                dstCwd = job.getDestinationCwd()
                dstTmp = job.getDestinationTmpDir()
                jobDir = jc.getJobRootRelative()+'/'+job.getJobDirRelative()    # make sure it works for remoteLsfJobs

				# Determine whether to expand the binary paths
                if os.path.isabs(self._encoderExe):	      encExe = self._encoderExe
                else: 							          encExe = dstCwd+'/'+self._encoderExe 
                if os.path.isabs(self._decoderExe):       decExe = self._decoderExe  
                else: 							          decExe = dstCwd+'/'+self._decoderExe     
                if self._extractorExe: 
					if os.path.isabs(self._extractorExe): extExe = self._extractorExe    
					else: 							      extExe = dstCwd+'/'+self._extractorExe
                preprocScr = self._preprocScript
                if self._preprocScript != None:
                  if os.path.isabs(self._preprocScript):    preprocScr = self._preprocScript
                  else:                                     preprocScr = dstCwd+'/'+self._preprocScript  
                postprocScr = self._postprocScript
                if self._postprocScript != None:
                  if os.path.isabs(self._postprocScript):   postprocScr = self._postprocScript
                  else:                                     postprocScr = dstCwd+'/'+self._postprocScript
                        
                # set general job configuration
                if self._extractorExe:
                    self._current_conf['binaries'] = [encExe, decExe, extExe]
                else:
                    self._current_conf['binaries'] = [encExe, decExe]
                if self._removeEnc > 1:
                    self._current_conf['encDir'] = dstTmp+'/'+self._encDir
                else:
                    self._current_conf['encDir'] = dstCwd+'/'+self._encDir
                if self._removeRec > 1:
                    self._current_conf['recDir'] = dstTmp+'/'+self._recDir
                else:
                    self._current_conf['recDir'] = dstCwd+'/'+self._recDir
                if self._removeDec > 1:
                    self._current_conf['decDir'] = dstTmp+'/'+self._decDir
                else:
                    self._current_conf['decDir'] = dstCwd+'/'+self._decDir

                #transcoding3
                self._current_conf['transDir'] = dstCwd+'/'+self._transDir

                if self._preproc:
                    self._current_conf['preprocScript'] = preprocScr
                    if self._removePreproc > 1:
                        self._current_conf['preprocDir'] = dstTmp+'/'+self._preprocDir
                    else:
                        self._current_conf['preprocDir'] = dstCwd+'/'+self._preprocDir
                if self._postproc:
                    self._current_conf['postprocScript'] = postprocScr
                    if self._removePostproc > 1:
                        self._current_conf['postprocDir'] = dstTmp+'/'+self._postprocDir
                    else:
                        self._current_conf['postprocDir'] = dstCwd+'/'+self._postprocDir
                self._current_conf['resultsDir'] = dstCwd+'/'+self._resultsDir+'/'+self._resultsSubDir
                self._current_conf['simId'] = simId[seqCnt][qpCnt]
                self._current_conf['workDir'] = dstCwd+'/'+jobDir
                encFile = "%s/%s%s" % (self._current_conf['encDir'], self._current_conf['simId'], self._bitStreamFileExtension)
                self._current_conf['encFile'] = encFile
                if self._numLayers > 1:
                    self._current_conf['xtrFile'] = "%s/%s-xtr%s" % (self._current_conf['encDir'], self._current_conf['simId'], self._bitStreamFileExtension)
                recFile   = []
                decFile   = []
                transFile = []
                for layCnt in range(self._numLayers):
                    recFile.append("%s/rec_%s_L%d.yuv" % (self._current_conf['recDir'], self._current_conf['simId'], layCnt))
                    decFile.append("%s/dec_%s_L%d.yuv" % (self._current_conf['decDir'], self._current_conf['simId'], layCnt))
                    transFile.append("%s/trans_%s_L%d.yuv" % (self._current_conf['transDir'], self._current_conf['simId'], layCnt))
                self._current_conf['recFile'] = recFile
                self._current_conf['decFile'] = decFile
                self._current_conf['transFile'] = transFile

                if self._preproc:
                    preprocFile=[]
                    for layCnt in range(self._numLayers):
                        preprocFile.append("%s/pre_%s_L%d.yuv" % (self._current_conf['preprocDir'], self._current_conf['simId'], layCnt))
                    self._current_conf['preprocFile'] = preprocFile
                if self._postproc:
                    postprocFile=[]
                    for layCnt in range(self._numLayers):
                        postprocFile.append("%s/pst_%s_L%d.yuv" % (self._current_conf['postprocDir'], self._current_conf['simId'], layCnt))
                    self._current_conf['postprocFile'] = postprocFile

                # job specific adjustment of cfg parameters
                self._adjustJobSpecCfg(job.getDestinationPlatform(), self._qpSetSeq[seqCnt][qpCnt], qpCnt)

                # set to InternalBitDepth
                aconf = cfgFile.cfgFile(self._cfgFileMain, self._cfgFileKeyValDelim, self._cfgFileCommDelim)
                if aconf.countEntries('InternalBitDepth')>0:
                  self._current_conf['InternalBitDepth']=aconf['InternalBitDepth']
                else:
                  self._current_conf['InternalBitDepth']=8

                # set to InputBitDepth
                aconf = cfgFile.cfgFile(self._cfgFileMain, self._cfgFileKeyValDelim, self._cfgFileCommDelim)
                if aconf.countEntries('InputBitDepth')>0:
                  self._current_conf['InputBitDepth']=aconf['InputBitDepth']
                else:
                  self._current_conf['InputBitDepth']=8

                # write the config files
                confFileName = jobDir+"/config.txt"
                self._current_conf.writeCfgFile(confFileName)
                self._current_confMain.writeCfgFile(jobDir+'/'+self._cfgFileMainBase)
                if self._current_confLayers is not None:
                    for layCnt in range(self._numLayers):
                        self._current_confLayers[layCnt].writeCfgFile("%s/L%d_%s" % (jobDir, layCnt, self._cfgFileLayersBase[layCnt]))

                # config file for the transcoder
                if self._cfgFileMainTrn!= None:
                  self._current_confMainTrn.writeCfgFile(jobDir+'/'+self._cfgFileMainBaseTrn)

                # estimate megabytes required for temporary storage
                tmpMegaBytes = 0
                if self._removeEnc > 1:
                    tmpMegaBytes += self._current_seqLayers[self._numLayers-1].fileSizeOut / 1000000.0        # bit stream will never be bigger than uncompressed
                if (self._removeRec > 1) or (self._decode and (self._removeDec > 1)):
                    tmpMegaBytes += self._current_seqLayers[self._numLayers-1].fileSizeOut * 1.1 / 1000000.0  # add 10% to be safe

                # execute
                job.submit("python RunEncodingGeneric.py " + confFileName, tmpMegaBytes)

        # write configurations for RunEval
        for layCnt in range(self._numLayers):
            evalConfFileName =  "%s/%s-L%d.txt" % (self._resultsDir, self._simSettings, layCnt)
            if os.path.isfile(evalConfFileName):
                evalConf = cfgFile.cfgFile(evalConfFileName)
                if evalConf['resultsSubDir'] != self._resultsSubDir:
                    raise RunSimException("resultsSubDir not matching in %s (%s, %s)" % (evalConfFileName, evalConf['resultsSubDir'], self._resultsSubDir))
                if evalConf['simSettings'] != "%s-L%d" % (self._simSettings, layCnt):
                    raise RunSimException("simSettings not matching in %s (%s, %s)" % (evalConfFileName, evalConf['simSettings'], "%s-L%d" % (self._simSettings, layCnt)))
                seqOut = evalConf['testSequences']
                simIdOut = evalConf['simIds']
            else:
                evalConf = cfgFile.cfgFile()
                evalConf['resultsSubDir'] = self._resultsSubDir
                evalConf['simSettings'] = "%s-L%d" % (self._simSettings, layCnt)
                seqOut = []
                simIdOut = []
            for seqCnt in range(len(self._testSet)):
                if self._testSet[seqCnt][layCnt] in seqOut:
                    simIdCnt = seqOut.index(self._testSet[seqCnt][layCnt])
                else:
                    seqOut.append(self._testSet[seqCnt][layCnt])
                    simIdCnt = len(simIdOut)
                    simIdOut.append([])
                for qpCnt in range(len(self._qpSetSeq[seqCnt])):
                    simIdOut[simIdCnt].append(simId[seqCnt][qpCnt] + "_L%d" % layCnt)
            evalConf['testSequences'] = seqOut
            evalConf['simIds'] = simIdOut
            evalConf['decode'] = self._current_conf['decode']
            evalConf['cfgfile'] = self._cfgFileMainBase
            evalConf.writeCfgFile(evalConfFileName)
            print "evaluation configuration written to %s" % evalConfFileName

class RunSimSamsung(RunSim,object):

    _cfgFileKeyValDelim = ':'
    _cfgFileCommDelim = '#'
    _bitStreamFileExtension = '.bin'

    _adjustFramesOut = False         # encodes any number of frames ?
    _keyFramesOnlyCount = False      # counts all frames for intra period ?
    _intraPeriodKey = 'IntraPeriod'  # intra period key in cfg file
    _qpSetKey = 'qpSet'

    _qpOffsetIP = 1
    _qpOffsetIB = 2

    def _adjustSeqSpecCfg(self):
        """ sequence specific adjustment of cfg parameters including sequence override """
        # sequence specific overriding
        self._doSeqOverride(self._current_confMain['GOPSize'])
        # perhaps a GOP (b gop size) override here

        # sequences specific settings
        seq = self._current_seqLayers[0]
        self._current_confMain['SourceWidth'] = seq.width
        self._current_confMain['SourceHeight'] = seq.height
        self._current_confMain['FramesToBeEncoded'] = seq.framesOut
        self._current_confMain['FrameSkip'] = seq.startFrame
        self._current_confMain['FrameRate'] = seq.fpsIn
        self._current_confMain['InputBitDepth'] = seq.bitdepth

    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):
        """ job specific adjustment of cfg parameters """
        # update confMain with the actual encoding parameters
        self._current_confMain['InputFile'] = self._current_seqLayers[0].getPath(destinationPlatform)
        if self._removeRec==3:
          count = self._current_confMain.countEntries('ReconFile')
          if count:
            self._current_confMain.removeEntries('ReconFile')
        else:
          self._current_confMain['ReconFile'] = self._current_conf['recFile'][0]
        self._current_confMain['BitstreamFile'] = self._current_conf['encFile']
        self._current_confMain['QP'] = qpLayers[0]

        #hej1
        self._current_conf['preprocOpts'] = []
        self._current_conf['preprocFile2'] = []
        self._current_conf['postprocOpts'] = []
        self._current_conf['postprocFile2'] = []
        self._current_conf['postprocFile3'] = []


        #if self._current_changeQpFrameSetFlag is not None:
        #    self._current_confMain['ChangeQPFrame'] = self._current_changeQpFrameSet[qpChange][0]
        #if self._current_changeQPISetFlag is not None:
        #    self._current_confMain['ChangeQPI'] = self._current_changeQPISet[qpChange][0]
        #if self._current_changeQPPSetFlag is not None:
        #    self._current_confMain['ChangeQPP'] = self._current_changeQPPSet[qpChange][0]
        #if self._current_changeQPBSetFlag is not None:
        #    self._current_confMain['ChangeQPB'] = self._current_changeQPBSet[qpChange][0]

        if self._current_confMain.countEntries('commandOpts') > 0:
          command =     self._current_confMain['commandOpts']
        else:
          command = ''

        # encoder/decoder options
        if self._removeRec==3:
          encOpts = "-c %s -i %s -b %s -q %f %s" % (self._cfgFileMainBase, self._current_confMain['InputFile'], self._current_confMain['BitstreamFile'],self._current_confMain['QP'], command)
        else:
          encOpts = "-c %s -i %s -o %s -b %s -q %f %s" % (self._cfgFileMainBase, self._current_confMain['InputFile'], self._current_confMain['ReconFile'], self._current_confMain['BitstreamFile'],self._current_confMain['QP'], command)
        self._current_conf['encOpts'] = encOpts

        # this one is only valid for RExt and RGB content coding
        if self._current_confMain.countEntries('OutputColourSpaceConvert') > 0:
            addParam = "--OutputColourSpaceConvert=GBRtoRGB"
            self._current_conf['decFile'][0] = self._current_conf['decFile'][0].replace('.yuv','.rgb')
        else:
            addParam = ""

        # This is only valid when using the SCM CU visualization binary
        if self._current_confMain.countEntries('CUVisualization') > 0:
            cuVisFile = self._current_conf['decFile'][0].replace('dec','cuvis')
            addParam += " -v %s" % cuVisFile
            self._createDir(os.path.dirname(cuVisFile))

        if self._removeDec==3:
          decOpts = ["-b %s %s" % (self._current_conf['encFile'],addParam)]
        else:
          decOpts = ["-b %s -o %s %s" % (self._current_conf['encFile'], self._current_conf['decFile'][0], addParam)]

        self._current_conf['decOpts'] = decOpts


'''
class RunSimTNTM(RunSim):

    _cfgFileKeyValDelim = None
    _cfgFileCommDelim = ';'
    _bitStreamFileExtension = '.bin'

    #_adjustFramesOut = True            # frame number needs to be aligned with sub-gop size
    _adjustFramesOut = False            # frame number needs to be aligned with sub-gop size
    _keyFramesOnlyCount = False        # TNTM counts all frames for intra period
    _intraPeriodKey = '-intra_period'  # intra period key in TNTM cfg file
    _qpSetKey = 'qpSet'

    def _adjustSeqSpecCfg(self):
        """ sequence specific adjustment of cfg parameters including sequence override """
        # sequence specific overriding
        self._doSeqOverride(self._current_confMain['-nb'] + 1)

        # sequences specific settings
        self._current_confMain['-rows'] = self._current_seqLayers[0].height
        self._current_confMain['-cols'] = self._current_seqLayers[0].width
        self._current_confMain['-n'] = self._current_seqLayers[0].framesOut
        self._current_confMain['-ifreq'] = self._current_seqLayers[0].fpsIn
        if self._current_confMain.countEntries('-ofreq')!=0:         # Check needed to be compliant with limited version
            self._current_confMain['-ofreq'] = self._current_seqLayers[0].fpsOut
        self._current_confMain['-first_frames_skip'] = self._current_seqLayers[0].startFrame


    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):
        """ job specific adjustment of cfg parameters """
        # update confMain with the actual encoding parameters
        self._current_confMain['-if'] = self._current_seqLayers[0].getPath(destinationPlatform)
        self._current_confMain['-rf'] = self._current_conf['recFile'][0]
        self._current_confMain['-of'] = self._current_conf['encFile']
        self._current_confMain['-qp'] = qpLayers[0]

        if self._current_changeQpFrameSetFlag is not None:
            self._current_confMain['-qp2start'] = self._current_changeQpFrameSet[qpChange][0]
        if self._current_changeQPPSetFlag is not None:
            self._current_confMain['-qp2'] = qpLayers[0]+self._current_changeQPPSet[qpChange][0]

        # encoder/decoder options
        encOpts = "-cf %s -if %s -cols %d -rows %d -n %d -qp %d -rf %s -of %s -ifreq %f -first_frames_skip %d" % (self._cfgFileMainBase, self._current_confMain['-if'], self._current_confMain['-cols'], self._current_confMain['-rows'], self._current_confMain['-n'], self._current_confMain['-qp'], self._current_confMain['-rf'], self._current_confMain['-of'], self._current_confMain['-ifreq'], self._current_confMain['-first_frames_skip'])
        self._current_conf['encOpts'] = encOpts

        decOpts = ["%s %s %s" % (self._current_conf['encFile'], self._current_conf['decFile'][0], self._current_confMain['-if'])]
        self._current_conf['decOpts'] = decOpts



class RunSimJSVM(RunSim):

    _cfgFileKeyValDelim = None
    _cfgFileCommDelim = '#'
    _bitStreamFileExtension = '.264'

    _adjustFramesOut = False         # JSVM works with any number of frames
    _keyFramesOnlyCount = False      # JSVM counts all frames for intra period
    _intraPeriodKey = 'IntraPeriod'  # intra period key in JSVM cfg file
    _qpSetKey = 'qpSet'

    def _adjustSeqSpecCfg(self):
        """ sequence specific adjustment of cfg parameters including sequence override """
        # sequence specific overriding
        self._doSeqOverride(self._current_confMain['GOPSize'])

        # sequences specific settings
        maxFpsIn = 0
        maxNumFrames = 0
        # JSVM does not support fpsIn!=fpsOut in highest layer
        self._checkParameter('self._current_seqLayers[%d].fpsIn - self._current_seqLayers[%d].fpsOut' % (self._numLayers-1, self._numLayers-1), '== 0')
        for layCnt in range(self._numLayers):
            self._checkParameter('self._current_seqLayers[%d].startFrame' % layCnt, '== 0')  # JSVM does not support startFrame!=0
            seq = self._current_seqLayers[layCnt]
            self._current_confLayers[layCnt]['SourceWidth'] = seq.width
            self._current_confLayers[layCnt]['SourceHeight'] = seq.height
            self._current_confLayers[layCnt]['FrameRateIn'] = seq.fpsIn
            self._current_confLayers[layCnt]['FrameRateOut'] = seq.fpsOut
            if seq.fpsIn > maxFpsIn:
                maxFpsIn = seq.fpsIn
                maxNumFrames = seq.framesIn
        self._current_confMain['FrameRate'] = maxFpsIn
        self._current_confMain['FramesToBeEncoded'] = maxNumFrames
        self._current_confMain['NumLayers'] = self._numLayers


    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):
        """ job specific adjustment of cfg parameters """

        # update confMain with the actual encoding parameters
        self._current_confMain['OutputFile'] = self._current_conf['encFile']
        self._current_confMain.removeEntries('LayerCfg')
        for layCnt in range(self._numLayers):
            self._checkParameter('self._current_seqLayers[%d].startFrame' % layCnt, '== 0')  # JSVM does not support startFrame!=0
            seq = self._current_seqLayers[layCnt]
            self._current_confLayers[layCnt]['InputFile'] = seq.getPath(destinationPlatform)
            self._current_confLayers[layCnt]['ReconFile'] = self._current_conf['recFile'][layCnt]
            self._current_confMain.appendEntry('LayerCfg', "L%d_%s" % (layCnt, self._cfgFileLayersBase[layCnt]))

        # encoder/decoder options
        encOpts = "-pf %s -bf %s" % (self._cfgFileMainBase, self._current_conf['encFile'])
        for layCnt in range(self._numLayers):
            encOpts = encOpts + " -lqp %d %d -rec %d %s" % (layCnt, qpLayers[layCnt], layCnt, self._current_confLayers[layCnt]['ReconFile'])
        self._current_conf['encOpts'] = encOpts
        xtrOpts = []
        decOpts = []
        for layCnt in range(self._numLayers-1):
            xtrOpts.append("%s %s -l %d" % (self._current_conf['encFile'], self._current_conf['xtrFile'], layCnt))
            decOpts.append("%s %s" % (self._current_conf['xtrFile'], self._current_conf['decFile'][layCnt]))
        decOpts.append("%s %s" % (self._current_conf['encFile'], self._current_conf['decFile'][self._numLayers-1]))
        self._current_conf['xtrOpts'] = xtrOpts
        self._current_conf['decOpts'] = decOpts



class RunSimJM(RunSim):

    _cfgFileKeyValDelim = '='
    _cfgFileCommDelim = '#'
    _bitStreamFileExtension = '.264'

    _adjustFramesOut = False         # JM encodes any number of frames
    _keyFramesOnlyCount = False      # JM counts all frames for intra period
    _intraPeriodKey = 'IntraPeriod'  # intra period key in JM cfg file
    _qpSetKey = 'qpSet'

    _qpOffsetIP = 1
    _qpOffsetIB = 2

    def _adjustSeqSpecCfg(self):
        """ sequence specific adjustment of cfg parameters including sequence override """
        # sequence specific overriding
        self._doSeqOverride(self._current_confMain['NumberBFrames'] + 1)

        # sequences specific settings
        seq = self._current_seqLayers[0]
        self._current_confMain['SourceWidth'] = seq.width
        self._current_confMain['SourceHeight'] = seq.height
        self._current_confMain['FramesToBeEncoded'] = seq.framesOut
        self._current_confMain['FrameSkip'] = seq.framesDropped
        self._current_confMain['FrameRate'] = seq.fpsIn
        self._current_confMain['StartFrame'] = seq.startFrame


    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):
        """ job specific adjustment of cfg parameters """
        # update confMain with the actual encoding parameters
        self._current_confMain['InputFile'] = self._current_seqLayers[0].getPath(destinationPlatform)
        self._current_confMain['ReconFile'] = self._current_conf['recFile'][0]
        self._current_confMain['OutputFile'] = self._current_conf['encFile']
        self._current_confMain['QPISlice'] = qpLayers[0]
        self._current_confMain['QPPSlice'] = qpLayers[0] + self._qpOffsetIP
        self._current_confMain['QPBSlice'] = qpLayers[0] + self._qpOffsetIB

        if self._current_changeQpFrameSetFlag is not None:
            self._current_confMain['ChangeQPFrame'] = self._current_changeQpFrameSet[qpChange][0]
        if self._current_changeQPISetFlag is not None:
            self._current_confMain['ChangeQPI'] = self._current_changeQPISet[qpChange][0]
        if self._current_changeQPPSetFlag is not None:
            self._current_confMain['ChangeQPP'] = self._current_changeQPPSet[qpChange][0]
        if self._current_changeQPBSetFlag is not None:
            self._current_confMain['ChangeQPB'] = self._current_changeQPBSet[qpChange][0]

        # encoder/decoder options
        encOpts = "-d %s -p InputFile=%s -p SourceWidth=%d -p SourceHeight=%d -p FramesToBeEncoded=%d -p QPISlice=%d -p QPPSlice=%d -p QPBSlice=%d -p ReconFile=%s -p OutputFile=%s -p FrameRate=%f -p StartFrame=%d" % (self._cfgFileMainBase, self._current_confMain['InputFile'], self._current_confMain['SourceWidth'], self._current_confMain['SourceHeight'], self._current_confMain['FramesToBeEncoded'], self._current_confMain['QPISlice'], self._current_confMain['QPPSlice'], self._current_confMain['QPBSlice'], self._current_confMain['ReconFile'], self._current_confMain['OutputFile'], self._current_confMain['FrameRate'], self._current_confMain['StartFrame'])
        self._current_conf['encOpts'] = encOpts

        decOpts = ["-i %s -o %s -r %s" % (self._current_conf['encFile'], self._current_conf['decFile'][0], self._current_confMain['InputFile'])]
        self._current_conf['decOpts'] = decOpts



class RunSimKTA(RunSim):

    _cfgFileKeyValDelim = '='
    _cfgFileCommDelim = '#'
    _bitStreamFileExtension = '.264'

    _adjustFramesOut = True          # KTA needs the number of frames adjusted with the sub-gop size
    _keyFramesOnlyCount = True       # KTA counts only non-B frames for intra period (old JM behaviour)
    _intraPeriodKey = 'IntraPeriod'  # intra period key in KTA cfg file
    _qpSetKey = 'qpSet'

    _qpOffsetIP = 1
    _qpOffsetIB = 2

    def _adjustSeqSpecCfg(self):
        """ sequence specific adjustment of cfg parameters including sequence override """
        # sequence specific overriding
        numberBFrames = self._current_confMain['NumberBFrames']
        self._doSeqOverride(numberBFrames + 1)

        # sequences specific settings
        seq = self._current_seqLayers[0]
        self._current_confMain['SourceWidth'] = seq.width
        self._current_confMain['SourceHeight'] = seq.height
        assert (seq.framesOut - 1) % (numberBFrames + 1) == 0  # because of _doSeqOverride
        self._current_confMain['FramesToBeEncoded'] = (seq.framesOut - 1) / (numberBFrames + 1) + 1
        self._current_confMain['FrameSkip'] = numberBFrames + (numberBFrames + 1) * seq.framesDropped
        self._current_confMain['FrameRate'] = seq.fpsIn
        self._current_confMain['StartFrame'] = seq.startFrame


    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):
        """ job specific adjustment of cfg parameters """
        # update confMain with the actual encoding parameters
        self._current_confMain['InputFile'] = self._current_seqLayers[0].getPath(destinationPlatform)
        self._current_confMain['ReconFile'] = self._current_conf['recFile'][0]
        self._current_confMain['OutputFile'] = self._current_conf['encFile']
        self._current_confMain['QPISlice'] = qpLayers[0]
        self._current_confMain['QPPSlice'] = qpLayers[0] + self._qpOffsetIP
        self._current_confMain['QPBSlice'] = qpLayers[0] + self._qpOffsetIB

        # encoder/decoder options
        encOpts = "-d %s -p InputFile=%s -p SourceWidth=%d -p SourceHeight=%d -p FramesToBeEncoded=%d -p QPISlice=%d -p QPPSlice=%d -p QPBSlice=%d -p ReconFile=%s -p OutputFile=%s -p FrameRate=%f -p StartFrame=%d" % (self._cfgFileMainBase, self._current_confMain['InputFile'], self._current_confMain['SourceWidth'], self._current_confMain['SourceHeight'], self._current_confMain['FramesToBeEncoded'], self._current_confMain['QPISlice'], self._current_confMain['QPPSlice'], self._current_confMain['QPBSlice'], self._current_confMain['ReconFile'], self._current_confMain['OutputFile'], self._current_confMain['FrameRate'], self._current_confMain['StartFrame'])
        self._current_conf['encOpts'] = encOpts

        decOpts = ["-i %s -o %s -r %s" % (self._current_conf['encFile'], self._current_conf['decFile'][0], self._current_confMain['InputFile'])]
        self._current_conf['decOpts'] = decOpts

class RunSimSHVC(RunSim):

    _cfgFileKeyValDelim = ':'
    _cfgFileCommDelim = '#'
    _bitStreamFileExtension = '.bin'

    _adjustFramesOut = False         # encodes any number of frames ?
    _keyFramesOnlyCount = False      # counts all frames for intra period ?
    _intraPeriodKey = 'IntraPeriod'  # intra period key in cfg file
    _qpSetKey = 'qpSet'

    def _adjustSeqSpecCfg(self):
        """ sequence specific adjustment of cfg parameters including sequence override """
        # sequence specific overriding

        self._doSeqOverride(self._current_confMain['GOPSize'])

        for layCnt in range(self._numLayers):
            IntraPeriod = 'IntraPeriod' + str(layCnt);
            if self._current_confMain.hasEntries(IntraPeriod):
              self._current_confMain[IntraPeriod] = self._current_confMain['IntraPeriod']
            else:
              self._current_confMain.appendEntry(IntraPeriod,self._current_confMain['IntraPeriod'])

        self._current_confMain['NumLayers'] = self._numLayers

        # sequences specific settings
        seq = self._current_seqLayers[0]
        self._current_confMain['FramesToBeEncoded'] = seq.framesOut


    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):
        """ job specific adjustment of cfg parameters """

        # update confMain with the actual encoding parameters
        self._current_confMain['BitstreamFile'] = self._current_conf['encFile']

        self._current_confMain.removeEntries('LayerCfg')
        for layCnt in range(self._numLayers):
            seq = self._current_seqLayers[layCnt]
            InputFile = 'InputFile' + str(layCnt);
            if self._current_confMain.hasEntries(InputFile):
              self._current_confMain[InputFile] = seq.getPath(destinationPlatform)
            else:
              self._current_confMain.appendEntry(InputFile,seq.getPath(destinationPlatform))

            ReconFile = 'ReconFile' + str(layCnt);
            if self._current_confMain.hasEntries(ReconFile):
              self._current_confMain[ReconFile] = self._current_conf['recFile'][layCnt]
            else:
              self._current_confMain.appendEntry(ReconFile,self._current_conf['recFile'][layCnt])

            LayerCfg = 'LayerConfig' + str(layCnt)
            if self._current_confMain.hasEntries(LayerCfg):
              self._current_confMain[LayerCfg] = self._cfgFileLayersBase[layCnt]
            else:
              self._current_confMain.appendEntry(LayerCfg, "%s" % (self._cfgFileLayersBase[layCnt]))

            FrameRate = 'FrameRate' + str(layCnt);
            if self._current_confMain.hasEntries(FrameRate):
              self._current_confMain[FrameRate] = seq.fpsIn
            else:
              self._current_confMain.appendEntry(FrameRate,seq.fpsIn)

            SourceWidth = 'SourceWidth' + str(layCnt);
            if self._current_confMain.hasEntries(SourceWidth):
              self._current_confMain[SourceWidth] = seq.width
            else:
              self._current_confMain.appendEntry(SourceWidth,seq.width)

            SourceHeight = 'SourceHeight' + str(layCnt);
            if self._current_confMain.hasEntries(SourceHeight):
              self._current_confMain[SourceHeight] = seq.height
            else:
              self._current_confMain.appendEntry(SourceHeight,seq.height)

            QP = 'QP' + str(layCnt);
            if self._current_confMain.hasEntries(QP):
              self._current_confMain[QP] = qpLayers[layCnt]
            else:
              self._current_confMain.appendEntry(QP,qpLayers[layCnt])



        # encoder/decoder options

        encOpts = "-c %s -b %s" % (self._cfgFileMainBase,self._current_conf['encFile'])
        for layCnt in range(self._numLayers):
          encOpts = encOpts + " -lc%d %s -o%d %s" % (layCnt,self._cfgFileLayers[layCnt],layCnt,self._current_conf['recFile'][layCnt])
        self._current_conf['encOpts'] = encOpts

        decOpts = []
        for layCnt in range(self._numLayers):
            decOpts.append(" -b %s -o%d %s -ls %d" % (self._current_conf['encFile'],layCnt,self._current_conf['decFile'][layCnt],layCnt+1))
            #decOpts.append(" -b %s -o%d %s -ls %d" % (self._current_conf['encFile'],layCnt,self._current_conf['decFile'][layCnt],self._numLayers))
        self._current_conf['decOpts'] = decOpts
'''

class RunSimSHVC_TRN(RunSim):

    _cfgFileKeyValDelim = ':'
    _cfgFileCommDelim = '#'
    _bitStreamFileExtension = '.bin'

    _adjustFramesOut = False         # encodes any number of frames ?
    _keyFramesOnlyCount = False      # counts all frames for intra period ?
    _intraPeriodKey = 'IntraPeriod'  # intra period key in cfg file
    _qpSetKey = 'qpSet'

    def _adjustSeqSpecCfg(self):
        """ sequence specific adjustment of cfg parameters including sequence override """
        # sequence specific overriding

        self._doSeqOverride(self._current_confMain['GOPSize'])

        for layCnt in range(self._numLayers):
            IntraPeriod = 'IntraPeriod' + str(layCnt);
            if self._current_confMain.hasEntries(IntraPeriod):
              self._current_confMain[IntraPeriod] = self._current_confMain['IntraPeriod']
            else:
              self._current_confMain.appendEntry(IntraPeriod,self._current_confMain['IntraPeriod'])

        self._current_confMain['NumLayers'] = self._numLayers

        # sequences specific settings
        seq = self._current_seqLayers[0]
        self._current_confMain['FramesToBeEncoded'] = seq.framesOut


    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):
        """ job specific adjustment of cfg parameters """

        # update confMain with the actual encoding parameters
        self._current_confMain['BitstreamFile'] = self._current_conf['encFile']

        self._current_confMain.removeEntries('LayerCfg')
        for layCnt in range(self._numLayers):
            seq = self._current_seqLayers[layCnt]
            InputFile = 'InputFile' + str(layCnt);
            if self._current_confMain.hasEntries(InputFile):
              self._current_confMain[InputFile] = seq.getPath(destinationPlatform)
            else:
              self._current_confMain.appendEntry(InputFile,seq.getPath(destinationPlatform))

            ReconFile = 'ReconFile' + str(layCnt);
            if self._current_confMain.hasEntries(ReconFile):
              self._current_confMain[ReconFile] = self._current_conf['recFile'][layCnt]
            else:
              self._current_confMain.appendEntry(ReconFile,self._current_conf['recFile'][layCnt])

            LayerCfg = 'LayerConfig' + str(layCnt)
            if self._current_confMain.hasEntries(LayerCfg):
              self._current_confMain[LayerCfg] = self._cfgFileLayersBase[layCnt]
            else:
              self._current_confMain.appendEntry(LayerCfg, "%s" % (self._cfgFileLayersBase[layCnt]))

            FrameRate = 'FrameRate' + str(layCnt);
            if self._current_confMain.hasEntries(FrameRate):
              self._current_confMain[FrameRate] = seq.fpsIn
            else:
              self._current_confMain.appendEntry(FrameRate,seq.fpsIn)

            SourceWidth = 'SourceWidth' + str(layCnt);
            if self._current_confMain.hasEntries(SourceWidth):
              self._current_confMain[SourceWidth] = seq.width
            else:
              self._current_confMain.appendEntry(SourceWidth,seq.width)

            SourceHeight = 'SourceHeight' + str(layCnt);
            if self._current_confMain.hasEntries(SourceHeight):
              self._current_confMain[SourceHeight] = seq.height
            else:
              self._current_confMain.appendEntry(SourceHeight,seq.height)

            QP = 'QP' + str(layCnt);
            if self._current_confMain.hasEntries(QP):
              self._current_confMain[QP] = qpLayers[layCnt]
            else:
              self._current_confMain.appendEntry(QP,qpLayers[layCnt])



        # modify layer0 params with params from layer1

        num = self._current_confMain.getSize()

        for i in range(num):
            key = self._current_confMain.getKey(i)
            if key!='':
              self._current_confMainTrn[key]=self._current_confMain[key]

        text = 'SourceHeight'
        text = text + str(self._current_confMainTrn['TranscodedStreamLayer'])
        self._current_confMainTrn['SourceHeight0'] = self._current_confMainTrn[text]
        text = 'SourceWidth'
        text = text + str(self._current_confMainTrn['TranscodedStreamLayer'])
        self._current_confMainTrn['SourceWidth0'] = self._current_confMainTrn[text]
        self._current_confMainTrn['NumLayers'] = self._current_confMainTrn['NumLayers']
        text = 'RDOQ'
        text = text + str(self._current_confMainTrn['TranscodedStreamLayer'])
        self._current_confMainTrn['RDOQ0'] = self._current_confMainTrn[text]
        text = 'RDOQTS'
        text = text + str(self._current_confMainTrn['TranscodedStreamLayer'])
        self._current_confMainTrn['RDOQTS0'] = self._current_confMainTrn[text]
        text = 'SignHideFlag'
        text = text + str(self._current_confMainTrn['TranscodedStreamLayer'])
        self._current_confMainTrn['SignHideFlag0'] = self._current_confMainTrn[text]
        text = 'QP'
        text = text + str(self._current_confMainTrn['TranscodedStreamLayer'])
        self._current_confMainTrn['QP0'] = self._current_confMainTrn[text]
        #self._current_confMainTrn['SourceHeight0'] = self._current_confMainTrn['SourceHeight1']
        #self._current_confMainTrn['SourceWidth0'] = self._current_confMainTrn['SourceWidth1']
        #self._current_confMainTrn['NumLayers'] = self._current_confMainTrn['NumLayers']
        #self._current_confMainTrn['RDOQ0'] = self._current_confMainTrn['RDOQ1']
        #self._current_confMainTrn['RDOQTS0'] = self._current_confMainTrn['RDOQTS1']
        #self._current_confMainTrn['SignHideFlag0'] = self._current_confMainTrn['SignHideFlag1']
        #self._current_confMainTrn['QP0'] = self._current_confMainTrn['QP1']

        bitstreamfile = self._current_conf['encFile']
        slen = len(bitstreamfile)
        outputbitstream = bitstreamfile[0:slen-4]
        # this is a bit sloppy, should define a encTrnFile similar to encFile
        outputbitstream = outputbitstream + 'T' + '.bin'
        self._current_confMainTrn['OutputBitstreamFile'] = outputbitstream


        # encoder/decoder options

        encOpts = "-c %s -b %s -ob %s -tr %d -ls %d" % (self._cfgFileMainBaseTrn,self._current_conf['encFile'],self._current_confMainTrn['OutputBitstreamFile'], self._current_confMainTrn['TranscodedStreamLayer'],self._current_confMainTrn['NumLayers'])
        for layCnt in range(self._numLayers):
          encOpts = encOpts + " -lc%d %s -o%d %s " % (layCnt,self._cfgFileLayers[layCnt],layCnt,self._current_conf['recFile'][layCnt])
        self._current_conf['encOpts'] = encOpts


        decOpts = []
        #for layCnt in range(self._numLayers):
        #decOpts = " -c %s -b %s -o%d %s -ls %d -ob %s" % (self._cfgFileMainBaseTrn, self._current_conf['encFile'], 1, self._current_conf['decFile'][0],2, self._current_confMainTrn['encTrnFile'])
            #decOpts.append(" -c %s -b %s -o%d %s -ls %d" % (self._cfgFileMainBaseTrn, self._current_conf['encFile'],layCnt,self._current_conf['decFile'][layCnt],layCnt+1))
            #decOpts.append(" -c %s -b %s -o%d %s -ls %d" % ('transcoding.cfg', self._current_conf['encFile'],layCnt,self._current_conf['decFile'][layCnt],layCnt+1))
        #SHM decoding single layer
        #decOpts.append("-b %s -o%d %s -ls %d" % (outputbitstream,0,self._current_conf['decFile'][0],1))
        #HM decoding
        decOpts.append("-b %s -o %s" % (outputbitstream,self._current_conf['decFile'][0]))
        #print decOpts
        self._current_conf['decOpts'] = decOpts
        self._current_conf['trnOutputLayer'] = self._current_confMainTrn['TranscodedStreamLayer']

'''
class RunSimRSM_HEVC(RunSim):

    _cfgFileKeyValDelim = ':'
    _cfgFileCommDelim = '#'
    _bitStreamFileExtension = '.bin'

    _adjustFramesOut = False         # encodes any number of frames ?
    _keyFramesOnlyCount = False      # counts all frames for intra period ?
    _intraPeriodKey = 'GOPLength'  # intra period key in cfg file
    _qpSetKey = 'qpSet'

    _qpOffsetIP = 0
    _qpOffsetIB = 0

    def _adjustSeqSpecCfg(self):
        """ sequence specific adjustment of cfg parameters including sequence override """
        # sequence specific overriding
        self._doSeqOverride(self._current_confMain['GOPLength'])
        # perhaps a GOP (b gop size) override here

        # sequences specific settings
        seq = self._current_seqLayers[0]
        self._current_confMain['YUVWidth'] = seq.width
        self._current_confMain['YUVHeight'] = seq.height
        self._current_confMain['PictureWidth'] = seq.width
        self._current_confMain['PictureHeight'] = seq.height
        self._current_confMain['FramesToEncode'] = seq.framesOut

        if seq.fpsIn == 23.9:
          self._current_confMain['FrameRate'] = "Rate_23_9"
        elif seq.fpsIn == 24:
          self._current_confMain['FrameRate'] = "Rate_24"
        elif seq.fpsIn == 25:
          self._current_confMain['FrameRate'] = "Rate_25"
        elif seq.fpsIn == 29.9:
          self._current_confMain['FrameRate'] = "Rate_29_9"
        elif seq.fpsIn == 30:
          self._current_confMain['FrameRate'] = "Rate_30"
        elif seq.fpsIn == 50:
          self._current_confMain['FrameRate'] = "Rate_50"
        elif seq.fpsIn == 59.9:
          self._current_confMain['FrameRate'] = "Rate_59.9"
        elif seq.fpsIn == 60:
          self._current_confMain['FrameRate'] = "Rate_60"
        else:
          print "Warning frame rate not recognized, assuming 25 fps"
          self._current_confMain['FrameRate'] = "Rate_25"

        # always start on first frame ?


    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):
        """ job specific adjustment of cfg parameters """
        # update confMain with the actual encoding parameters


        self._current_confMain['SourceVideoPath'] = self._current_seqLayers[0].getPath(destinationPlatform)
        self._current_confMain['InputAviFile'] = self._current_seqLayers[0].getPath(destinationPlatform)

        p = re.compile(r'.*/');
        self._current_confMain['InputAviFile'] = p.sub('', self._current_confMain['InputAviFile'])

        p = re.compile(r'/[^/]*$');
        self._current_confMain['SourceVideoPath'] = p.sub('', self._current_confMain['SourceVideoPath'])

        self._current_confMain['ReconOutputFile'] = self._current_conf['recFile'][0]
        self._current_confMain['OutputBitStream'] = self._current_conf['encFile']

        ## Force decode to be 0
        #self._checkParameter('self._decode', 'in [0]')

        ## Disallow 2 since RSM currently requires relative paths
        self._checkParameter('self._removeEnc', 'in [0, 1]')

        ## Writing to the temp directory does not work with RSM since RSM only accepts relative direcories.
        ## For removeRec equal to 2, we use absolute directories anyway. RSM will encode but not write any reconstructed output
        ## For removeRec equal to 0 and 1, we convert the file to use a realtive path
        if self._removeRec == 2:
          print "WARNING - no reconstruction will be written, no PSNR numbers will be available. Encoding will be OK but the script will exit returning a 1"
        else:
          p = re.compile(r'.*/Results/');
          self._current_confMain['ReconOutputFile'] = p.sub('../../../Results/', self._current_confMain['ReconOutputFile'])

        p = re.compile(r'.*/Results/');
        self._current_confMain['OutputBitStream'] = p.sub('../../../Results/', self._current_confMain['OutputBitStream'])

        if self._current_confMain['RateControlType'] == "RateControl_Fixed":
          self._current_confMain['RateControl_Fixed_init_qp'] = qpLayers[0]
        else:
          self._current_confMain['RateControl_BitRate'] = qpLayers[0]

        if self._current_confMain.countEntries('commandOpts') > 0:
          command =     self._current_confMain['commandOpts']
        else:
          command = ''

        # encoder/decoder options

        encOpts = "%s" % (self._cfgFileMainBase)
        self._current_conf['encOpts'] = encOpts

        #decOpts = ["-i %s -o %s " % (self._current_conf['encFile'], self._current_conf['decFile'][0])]
        decOpts = ["-b %s -o %s " % (self._current_conf['encFile'], self._current_conf['decFile'][0])]
        self._current_conf['decOpts'] = decOpts

class RunSimRSM_ICE6(RunSim):

    _cfgFileKeyValDelim = ':'
    _cfgFileCommDelim = '#'
    _bitStreamFileExtension = '.bin'

    _adjustFramesOut = False         # encodes any number of frames ?
    _keyFramesOnlyCount = False      # counts all frames for intra period ?
    _intraPeriodKey = 'GOPLength'  # intra period key in cfg file
    _qpSetKey = 'qpSet'

    _qpOffsetIP = 0
    _qpOffsetIB = 0

    def _adjustSeqSpecCfg(self):
        """ sequence specific adjustment of cfg parameters including sequence override """
        # sequence specific overriding
        self._doSeqOverride(self._current_confMain['GOPLength'])
        # perhaps a GOP (b gop size) override here

        # sequences specific settings
        seq = self._current_seqLayers[0]
        self._current_confMain['YUVWidth'] = seq.width
        self._current_confMain['YUVHeight'] = seq.height
        self._current_confMain['PictureWidth'] = seq.width
        self._current_confMain['PictureHeight'] = seq.height
        self._current_confMain['FramesToEncode'] = seq.framesOut

        if seq.fpsIn == 23.9:
          self._current_confMain['FrameRate'] = "Rate_23_9"
        elif seq.fpsIn == 24:
          self._current_confMain['FrameRate'] = "Rate_24"
        elif seq.fpsIn == 25:
          self._current_confMain['FrameRate'] = "Rate_25"
        elif seq.fpsIn == 29.9:
          self._current_confMain['FrameRate'] = "Rate_29_9"
        elif seq.fpsIn == 30:
          self._current_confMain['FrameRate'] = "Rate_30"
        elif seq.fpsIn == 50:
          self._current_confMain['FrameRate'] = "Rate_50"
        elif seq.fpsIn == 59.9:
          self._current_confMain['FrameRate'] = "Rate_59.9"
        elif seq.fpsIn == 60:
          self._current_confMain['FrameRate'] = "Rate_60"
        else:
          print "Warning frame rate not recognized, assuming 25 fps"
          self._current_confMain['FrameRate'] = "Rate_25"

        # always start on first frame ?


    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):
        """ job specific adjustment of cfg parameters """
        # update confMain with the actual encoding parameters


        self._current_confMain['SourceVideoPath'] = self._current_seqLayers[0].getPath(destinationPlatform)
        self._current_confMain['InputAviFile'] = self._current_seqLayers[0].getPath(destinationPlatform)

        p = re.compile(r'.*/');
        self._current_confMain['InputAviFile'] = p.sub('', self._current_confMain['InputAviFile'])

        p = re.compile(r'/[^/]*$');
        self._current_confMain['SourceVideoPath'] = p.sub('', self._current_confMain['SourceVideoPath'])


        self._current_confMain['ReconOutputFile'] = self._current_conf['recFile'][0]
        self._current_confMain['OutputBitStream'] = self._current_conf['encFile']


        ## Force decode to be 0
        #self._checkParameter('self._decode', 'in [0]')

        ## Disallow 2 since RSM currently requires relative paths
        self._checkParameter('self._removeEnc', 'in [0, 1]')

        ## Writing to the temp directory does not work with RSM since RSM only accepts relative direcories.
        ## For removeRec equal to 2, we use absolute directories anyway. RSM will encode but not write any reconstructed output
        ## For removeRec equal to 0 and 1, we convert the file to use a realtive path
        if self._removeRec == 2:
          print "WARNING - no reconstruction will be written, no PSNR numbers will be available. Encoding will be OK but the script will exit returning a 1"
        else:
          p = re.compile(r'.*/Results/');
          self._current_confMain['ReconOutputFile'] = p.sub('../../../Results/', self._current_confMain['ReconOutputFile'])

        p = re.compile(r'.*/Results/');
        self._current_confMain['OutputBitStream'] = p.sub('../../../Results/', self._current_confMain['OutputBitStream'])


        if self._current_confMain['RateControlType'] == "RateControl_Fixed":
          self._current_confMain['RateControl_Fixed_init_qp'] = qpLayers[0]
        else:
          self._current_confMain['RateControl_BitRate'] = qpLayers[0]


        if self._current_confMain.countEntries('commandOpts') > 0:
          command =     self._current_confMain['commandOpts']
        else:
          command = ''

        # encoder/decoder options

        encOpts = "%s" % (self._cfgFileMainBase)
        self._current_conf['encOpts'] = encOpts

        #decOpts = ["-i %s -o %s " % (self._current_conf['encFile'], self._current_conf['decFile'][0])]
        decOpts = ["-i %s -o %s -r %s" % (self._current_conf['encFile'], self._current_conf['decFile'][0], self._current_confMain['InputAviFile'])]
        self._current_conf['decOpts'] = decOpts

class RunSimC64(RunSim):

    _cfgFileKeyValDelim = None
    _cfgFileCommDelim = '#'
    _bitStreamFileExtension = '.264'

    _adjustFramesOut = False          # encodes any number of frames ?
    _keyFramesOnlyCount = False       # ?
    _intraPeriodKey = 'IntraPeriod'   # ?
    _qpSetKey = 'qpSet'

    _qpOffsetIP = 0                   # not used?
    _qpOffsetIB = 0                   # not used?

    def _adjustSeqSpecCfg(self):
        """ sequence specific adjustment of cfg parameters including sequence override """
        # sequences specific settings
        seq = self._current_seqLayers[0]
        self._current_confMain['SourceWidth'] = seq.width
        self._current_confMain['SourceHeight'] = seq.height
        self._current_confMain['FrameRate'] = seq.fpsIn
        self._current_confMain['FrameSkip'] = seq.framesDropped + 1
        self._current_confMain['StartFrame'] = seq.startFrame
        self._current_confMain['StopFrame'] = seq.framesOut - seq.startFrame - 1

        if self._current_confMain.countEntries('RefFrames') == 0:
            self._current_confMain['RefFrames'] = 1

    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):
        """ job specific adjustment of cfg parameters """
        # update confMain with the actual encoding parameters
        self._current_confMain['InputFile'] = self._current_seqLayers[0].getPath(destinationPlatform)
        self._current_confMain['ReconFile'] = self._current_conf['recFile'][0]
        self._current_confMain['OutputFile'] = self._current_conf['encFile']
        self._current_confMain['QP'] = qpLayers[0]

        # encoder options
        encOpts = "-i %s -x %d %d -qp %d -o %s -B %s -fps %f -r %d -a %d -b %d -k %d" % (self._current_confMain['InputFile'], self._current_confMain['SourceWidth'], self._current_confMain['SourceHeight'], self._current_confMain['QP'], self._current_confMain['ReconFile'], self._current_confMain['OutputFile'], self._current_confMain['FrameRate'], self._current_confMain['RefFrames'], self._current_confMain['StartFrame'], self._current_confMain['StopFrame'], self._current_confMain['FrameSkip'])
        self._current_conf['encOpts'] = encOpts

        if self._current_confMain.hasEntries('Speed'):
            self._current_conf['encOpts'] += " -speed %d" % (self._current_confMain['Speed'])
        if self._current_confMain.hasEntries('Cabac'):
            self._current_conf['encOpts'] += " -CABAC %d" % (self._current_confMain['Cabac'])
        if self._current_confMain.hasEntries('RCType'):
            self._current_conf['encOpts'] += " -RC %d %d" % (self._current_confMain['RCType'], self._current_confMain['RCBitrate'])
        if self._current_confMain.hasEntries('SliceByteThreshold'):
            self._current_conf['encOpts'] += " -slice %d" % (self._current_confMain['SliceByteThreshold'])

        # decoder options
        decOpts = ["-i %s -o %s" % (self._current_conf['encFile'], self._current_conf['decFile'][0])]
        self._current_conf['decOpts'] = decOpts


class RunSimC65(RunSim):

    _cfgFileKeyValDelim = ':'
    _cfgFileCommDelim = '#'
    _bitStreamFileExtension = '.bin'

    _adjustFramesOut = False         # encodes any number of frames ?
    _keyFramesOnlyCount = False      # counts all frames for intra period ?
    _intraPeriodKey = 'IntraPeriod'  # intra period key in cfg file
    _qpSetKey = 'qpSet'

    _qpOffsetIP = 1
    _qpOffsetIB = 2

    def _adjustSeqSpecCfg(self):
        """ sequence specific adjustment of cfg parameters including sequence override """
        # sequence specific overriding
        #self._doSeqOverride(self._current_confMain['GOPSize'])
        # perhaps a GOP (b gop size) override here

        # sequences specific settings
        seq = self._current_seqLayers[0]
        self._current_confMain['SourceWidth'] = seq.width
        self._current_confMain['SourceHeight'] = seq.height
        self._current_confMain['NumberOfFrames'] = seq.framesOut
        #self._current_confMain['FrameSkip'] = seq.framesDropped
        self._current_confMain['SourceFrameRate'] = seq.fpsIn
        #self._current_confMain['InputBitDepth'] = seq.bitdepth
        #self._current_confMain['StartFrame'] = seq.startFrame
        # always start on first frame ?


    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):
        """ job specific adjustment of cfg parameters """
        # update confMain with the actual encoding parameters
        self._current_confMain['InputFileName'] = self._current_seqLayers[0].getPath(destinationPlatform)
        if self._removeRec==3:
          count = self._current_confMain.countEntries('ResonstructedFileName')
          if count:
            self._current_confMain.removeEntries('ResonstructedFileName')
        else:
          self._current_confMain['ResonstructedFileName'] = self._current_conf['recFile'][0]
        self._current_confMain['OutputBitstreamName'] = self._current_conf['encFile']
        self._current_confMain['QP'] = qpLayers[0]

        if self._current_confMain.countEntries('commandOpts') > 0:
          command =     self._current_confMain['commandOpts']
        else:
          command = ''

        # encoder/decoder options
        if self._removeRec==3:
         encOpts = "-c %s -i %s -o %s %s" % (self._cfgFileMainBase,  self._current_confMain['InputFileName'], self._current_confMain['OutputBitstreamName'], command)
        else:
         encOpts = "-c %s  -i %s -o %s -r %s %s" % (self._cfgFileMainBase, self._current_confMain['InputFileName'], self._current_confMain['OutputBitstreamName'], self._current_confMain['ResonstructedFileName'], command)

        self._current_conf['encOpts'] = encOpts

        if self._removeDec==3:
          decOpts = ["-b %s " % (self._current_conf['encFile'])]
        else:
          decOpts = ["-b %s -o %s " % (self._current_conf['encFile'], self._current_conf['decFile'][0])]

        self._current_conf['decOpts'] = decOpts


class RunSimF265(RunSim):

    _cfgFileKeyValDelim = '='
    _cfgFileCommDelim = '#'
    _bitStreamFileExtension = '.265'

    _adjustFramesOut = False         # encodes any number of frames ?
    _keyFramesOnlyCount = True      # counts all frames for intra period ?
    _intraPeriodKey = 'key-frame-spacing'  # intra period key in cfg file
    _qpSetKey = 'qpSet'

    _qpOffsetIP = 0 # Not used?
    _qpOffsetIB = 0 # Not used

    def _adjustSeqSpecCfg(self):
        """ sequence specific adjustment of cfg parameters including sequence override """
        # sequence specific overriding
        #self._doSeqOverride(self._current_confMain['GOPSize'])
        # perhaps a GOP (b gop size) override here

        # sequences specific settings
        seq = self._current_seqLayers[0]
        #self._current_confMain['SourceWidth'] = seq.width
        #self._current_confMain['SourceHeight'] = seq.height
        #self._current_confMain['NumberOfFrames'] = seq.framesOut
        #self._current_confMain['FrameSkip'] = seq.framesDropped
        self._current_confMain['fps'] = "%i,1" % seq.fpsIn
        #self._current_confMain['InputBitDepth'] = seq.bitdepth
        #self._current_confMain['StartFrame'] = seq.startFrame
        # always start on first frame ?


    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):
        """ job specific adjustment of cfg parameters """
        # update confMain with the actual encoding parameters
        seq = self._current_seqLayers[0]
        #self._current_confMain['InputFileName'] = self._current_seqLayers[0].getPath(destinationPlatform)
        inputFile = self._current_seqLayers[0].getPath(destinationPlatform)
        #if self._removeRec==3:
        #  count = self._current_confMain.countEntries('ResonstructedFileName')
        #  if count:
        #    self._current_confMain.removeEntries('ResonstructedFileName')
        #else:
        #  self._current_confMain['ResonstructedFileName'] = self._current_conf['recFile'][0]
        #self._current_confMain['OutputBitstreamName'] = self._current_conf['encFile']
        bitstreamFile = self._current_conf['encFile']
        self._current_confMain['yuv-dump'] = self._current_conf['recFile'][0]
        self._current_confMain['qp'] = qpLayers[0]

        # encoder options

        #paramValuePairs = "".join(['%s:%s;' % (param,value) for param, value in self._current_confMain.items()])
        paramValuePairs = "".join(['%s:%s;' % (param,self._current_confMain[param]) for param in \
                                ['chroma-format', 'bit-depth', 'cb-range', 'tb-range', 'tb-depth', 'ref',
                                 'bframes', 'wpp', 'deblock', 'fpel', 'hpel', 'qpel', 'rdoq', 'sign-hiding',
                                 'smooth-intra', 'amp', 'tmv', 'nb-merge', 'pml', 'chroma-me', 'rdo',
                                 'hm-me', 'all-intra', 'nullify-inter-tb', 'qp', 'key-frame-spacing',
                                 'key-frame-type', 'yuv-dump', #'hm-gop',
                                 'rc', 'bitrate', 'bitrate-range', 'qp-bounds',
                                 'fps', 'lt-conv-min', 'lt-conv-exp']])
        if self._current_confMain['hm-gop'] != 'none':
            paramValuePairs += "hm-gop:%s;" % self._current_confMain['hm-gop']


        encOpts = '-v -c %i -p"%s" -w%ix%i %s %s' %\
            (seq.framesOut,
             paramValuePairs,
             seq.width,
             seq.height,
             inputFile,
             bitstreamFile)

        self._current_conf['encOpts'] = encOpts

        # decoder options
        if self._removeDec==3:
          decOpts = ["-b %s " % (self._current_conf['encFile'])]
        else:
          decOpts = ["-b %s -o %s " % (self._current_conf['encFile'], self._current_conf['decFile'][0])]

        self._current_conf['decOpts'] = decOpts

class RunSimHDR(RunSimSamsung, object):

    def _adjustSeqSpecCfgHDR(self):
        """ Necessary for multiple inheritence of HDRSHVC """
        self._adjustSeqSpecCfg()

    def _adjustSeqSpecCfg(self):
        """ sequence specific adjustment of cfg parameters including sequence override """

        self._doSeqOverride(self._current_confMain['GOPSize'])
        # perhaps a GOP (b gop size) override here

        # sequences specific settings
        seq = self._current_seqLayers[0]
        self._current_confMain['SourceWidth'] = seq.width
        self._current_confMain['SourceHeight'] = seq.height
        self._current_confMain['FramesToBeEncoded'] = seq.framesOut
        self._current_confMain['FrameStartProc'] = seq.startFrame
        self._current_confMain['FrameRate'] = seq.fpsIn
        #self._current_confMain['InputBitDepth'] = seq.bitdepth

        self._current_conf['InputChromaFormat'] = self._current_confMain['InputChromaFormat']


    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):
        """ job specific adjustment of cfg parameters """
        super(RunSimHDR,self)._adjustJobSpecCfg(destinationPlatform, qpLayers, qpChange)

        # update confMain with the preprocessing parameters
        preprocInputFile  = self._current_confMain['InputFile']
        # Only used when input is tiff or tif
        preprocBt2020Ref = "%s/bt2020ref_%s/b2020ref_%s_%s.exr" % \
          (self._current_conf['preprocDir'],
           self._current_conf['simId'],
           self._current_conf['simId'],
           os.path.splitext(preprocInputFile)[0][-4:]) # Frame number

        self._current_conf['preprocFile2'] = [preprocBt2020Ref]

        if self._current_confMain.hasEntries('preProcessExe'):
            preProcessExe = self._current_confMain['preProcessExe']
        else:
            preProcessExe = ''

        preprocOpts = ' '.join([str(x) for x in \
            ['-pre',
             preprocInputFile,
             self._current_conf['preprocFile'][0], # preprocOutputFile
             preprocBt2020Ref,
             self._preprocDir,
             self._removePreproc,
             self._current_confMain['SourceWidth'],
             self._current_confMain['SourceHeight'],
             self._current_confMain['FrameRate'],
             self._current_confMain['InputBitDepth'],
             self._current_confMain['InputChromaFormat'],
             self._current_confMain['FrameStartProc'],
             self._current_confMain['FramesToBeEncoded'],
             preProcessExe]])

        self._current_conf['preprocOpts'] = [preprocOpts]

        # update confMain with the postprocessing parameters
        # if no encoding and decoding do post-processing of pre-processing outputfile
        if ((self._encode==0) & (self._decode==0)):
          postprocInputFile = self._current_conf['preprocFile'][0]
        else:        
          postprocInputFile = self._current_conf['decFile'][0]
        if os.path.splitext(preprocInputFile)[1] in ['.exr', '.tiff', '.tif']:
            self._current_conf['postprocFile'] = ["%s/pst_%s/pst_%s_%s.exr" % \
              (self._current_conf['postprocDir'],
               self._current_conf['simId'],
               self._current_conf['simId'],
               os.path.splitext(preprocInputFile)[0][-4:])] # Frame number



        # Update metrics
        calculateMetrics = 1
        if self._current_confMain.hasEntries('MetricSettings'):
            metricSettings = str(self._current_confMain['MetricSettings']).replace(' ', '').replace(',', '\\,').replace("'", '\\"')
        else:
            metricSettings = '""'
        metricLogFile = "%s/hdrmetrics_%s.txt" % (self._current_conf['resultsDir'],self._current_conf['simId'])

        if os.path.splitext(preprocInputFile)[1] in ['.tiff', '.tif']:
            metricRefFile = preprocBt2020Ref
        else:
            metricRefFile = preprocInputFile

        postprocOutputFile = self._current_conf['postprocFile'][0]
        postProcTiffOutputFile = "%s/tif_%s/tif_%s_%s.tif" % \
          (self._current_conf['postprocDir'],
           self._current_conf['simId'],
           self._current_conf['simId'],
           os.path.splitext(postprocOutputFile)[0][-4:]) # Frame number

        postProcSim2OutputFile = "%s/sim2_%s.avi" % \
          (self._current_conf['postprocDir'],
           self._current_conf['simId'])


        self._current_conf['postprocFile2'] = [postProcTiffOutputFile]
        self._current_conf['postprocFile3'] = [postProcSim2OutputFile]

        if self._current_confMain.hasEntries('postProcessExe'):
            postProcessExe = self._current_confMain['postProcessExe']
        else:
            postProcessExe = ''

        if self._current_confMain.hasEntries('createTiffOutput'):
            createTiffOutput = self._current_confMain['createTiffOutput']
        else:
            createTiffOutput = 0

        if self._current_confMain.hasEntries('createSim2Output'):
            createSim2Output = self._current_confMain['createSim2Output']
        else:
            createSim2Output = 0


        postprocOpts = ' '.join ([str(x) for x in \
            ['-post',
             postprocInputFile,
             self._current_conf['postprocFile'][0], # postprocOutputFile
             postProcTiffOutputFile,
             postProcSim2OutputFile,
             self._postprocDir,
             self._removePostproc,
             self._current_confMain['SourceWidth'],
             self._current_confMain['SourceHeight'],
             self._current_confMain['FrameRate'],
             self._current_confMain['InputBitDepth'],
             self._current_confMain['InputChromaFormat'],
             self._current_confMain['FramesToBeEncoded'],
             calculateMetrics,
             metricSettings,
             metricRefFile,
             metricLogFile,
             createTiffOutput,
             createSim2Output,
             postProcessExe]])

        self._current_conf['postprocOpts'] = [postprocOpts]

        # Override encoder settings
        if self._current_conf['preproc'][0]:
            self._current_confMain['InputFile'] = self._current_conf['preprocFile'][0]

        if self._current_confMain.countEntries('commandOpts') > 0:
          command =     self._current_confMain['commandOpts']
        else:
          command = ''

        # encoder options
        if self._removeRec==3:
          encOpts = "-c %s -i %s -b %s -q %f %s" % (self._cfgFileMainBase, self._current_confMain['InputFile'], self._current_confMain['BitstreamFile'],self._current_confMain['QP'], command)
        else:
          encOpts = "-c %s -i %s -o %s -b %s -q %f %s" % (self._cfgFileMainBase, self._current_confMain['InputFile'], self._current_confMain['ReconFile'], self._current_confMain['BitstreamFile'],self._current_confMain['QP'], command)
        self._current_conf['encOpts'] = encOpts

class RunSimHDRSDR(RunSimHDR, object):

    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):

        seq = self._current_seqLayers[0]

        if seq.getPath(destinationPlatform).lower().endswith('.yuv'):
            self._current_conf['preproc'] = 0
            self._current_conf['postproc'] = 0
        else:
            self._current_conf['preproc'] = 1
            self._current_conf['postproc'] = 1

        """ job specific adjustment of cfg parameters """
        super(RunSimHDRSDR,self)._adjustJobSpecCfg(destinationPlatform, qpLayers, qpChange)



class RunSimHDRSHVC(RunSimSHVC, RunSimHDR, object):

    def _adjustSeqSpecCfg(self):
        """ sequence specific adjustment of cfg parameters including sequence override """
        #super(RunSimHDRSHVC,self)._adjustSeqSpecCfgHDR()
        RunSimHDR._adjustSeqSpecCfg(self)
        super(RunSimHDRSHVC,self)._adjustSeqSpecCfg()

    def _adjustJobSpecCfg(self, destinationPlatform, qpLayers, qpChange):
        """ job specific adjustment of cfg parameters """
        super(RunSimHDRSHVC,self)._adjustJobSpecCfg(destinationPlatform, qpLayers, qpChange)

        # Override preproc and postproc settings depending on file name
        self._current_conf['preproc'] = []
        self._current_conf['postproc'] = []
        for layCnt in range(self._numLayers):
            if self._current_confMain['InputFile'+str(layCnt)].endswith('.yuv'):
                self._current_conf['preproc'].append(0)
                self._current_conf['postproc'].append(0)
            else:
                self._current_conf['preproc'].append(1)
                self._current_conf['postproc'].append(1)

        # update confMain with the preprocessing parameters
        preprocInputFile=[]
        preprocFile2=[]
        preprocOpts=[]
        for layCnt in range(self._numLayers):
            preprocInputFile.append(self._current_confMain['InputFile'+str(layCnt)])

            # Only used when input is tiff or tif
            preprocBt2020Ref = "%s/bt2020ref_%s/b2020ref_%s_L%d_%s.exr" % \
              (self._current_conf['preprocDir'],
               self._current_conf['simId'],
               self._current_conf['simId'],
               layCnt,
               os.path.splitext(preprocInputFile[layCnt])[0][-4:]) # Frame number

            preprocFile2.append(preprocBt2020Ref)

            preprocOpts.append(' '.join([str(x) for x in \
                ['-pre',
                 preprocInputFile[layCnt],
                 self._current_conf['preprocFile'][layCnt], # preprocOutputFile
                 preprocBt2020Ref,
                 self._preprocDir,
                 self._removePreproc,
                 self._current_confMain['SourceWidth'+str(layCnt)],
                 self._current_confMain['SourceHeight'+str(layCnt)],
                 self._current_confMain['FrameRate'+str(layCnt)],
                 self._current_confMain['InputBitDepth'+str(layCnt)],
                 self._current_confMain['InputChromaFormat'],
                 self._current_confMain['FrameStartProc'],
                 self._current_confMain['FramesToBeEncoded']]]))

        self._current_conf['preprocFile2'] = preprocFile2

        self._current_conf['preprocOpts'] = preprocOpts

        # update confMain with the postprocessing parameters
        # if no encoding and decoding do post-processing of pre-processing outputfile
        postprocInputFile=[]
        for layCnt in range(self._numLayers):
            if ((self._encode==0) & (self._decode==0)):
              postprocInputFile.append(self._current_conf['preprocFile'][layCnt])
            else:
              postprocInputFile.append(self._current_conf['decFile'][layCnt])
            if os.path.splitext(preprocInputFile[layCnt])[1] in ['.exr', '.tiff', '.tif']:
                self._current_conf['postprocFile'][layCnt] = "%s/pst_%s/pst_%s_L%d_%s.exr" % \
                  (self._current_conf['postprocDir'],
                   self._current_conf['simId'],
                   self._current_conf['simId'],
                   layCnt,
                   os.path.splitext(preprocInputFile[layCnt])[0][-4:]) # Frame number

        # Update metrics
        postprocOpts=[]
        for layCnt in range(self._numLayers):

            calculateMetrics = self._current_conf['preproc'][layCnt]
            if self._current_confMain.hasEntries('MetricSettings'):
                metricSettings = str(self._current_confMain['MetricSettings']).replace(' ', '').replace(',', '\\,').replace("'", '\\"')
            else:
                metricSettings = '""'
            metricLogFile = "%s/hdrmetrics_%s.txt" % (self._current_conf['resultsDir'],self._current_conf['simId'])

            if os.path.splitext(preprocInputFile[layCnt])[1] in ['.tiff', '.tif']:
                metricRefFile = preprocBt2020Ref
            else:
                metricRefFile = preprocInputFile[layCnt]

            postprocOpts.append(' '.join ([str(x) for x in \
                ['-post',
                 postprocInputFile[layCnt],
                 self._current_conf['postprocFile'][layCnt], # postprocOutputFile
                 self._postprocDir,
                 self._removePostproc,
                 self._current_confMain['SourceWidth'+str(layCnt)],
                 self._current_confMain['SourceHeight'+str(layCnt)],
                 self._current_confMain['FrameRate'+str(layCnt)],
                 self._current_confMain['InputBitDepth'+str(layCnt)],
                 self._current_confMain['InputChromaFormat'],
                 self._current_confMain['FramesToBeEncoded'],
                 calculateMetrics,
                 metricSettings,
                 metricRefFile,
                 metricLogFile]]))

        self._current_conf['postprocOpts'] = postprocOpts

        # Override encoder settings
        for layCnt in range(self._numLayers):
            if self._current_conf['preproc'][layCnt]:
                self._current_confMain['InputFile'+str(layCnt)] = self._current_conf['preprocFile'][layCnt]

        if self._current_confMain.countEntries('commandOpts') > 0:
          command =     self._current_confMain['commandOpts']
        else:
          command = ''

        # encoder options
        encOpts = "-c %s -b %s" % (self._cfgFileMainBase, self._current_confMain['BitstreamFile'])
        for layCnt in range(self._numLayers):
          encOpts = encOpts + " -lc%d %s" % (layCnt,self._cfgFileLayers[layCnt])
          if not self._removeRec==3:
            encOpts = encOpts + " -o%d %s" % (layCnt,self._current_conf['recFile'][layCnt])
        self._current_conf['encOpts'] = encOpts

        decOpts = []
        for layCnt in range(self._numLayers):
            decOpts.append(" -b %s -o%d %s -ls %d" % (self._current_conf['encFile'],layCnt,self._current_conf['decFile'][layCnt],layCnt+1))
        self._current_conf['decOpts'] = decOpts
'''