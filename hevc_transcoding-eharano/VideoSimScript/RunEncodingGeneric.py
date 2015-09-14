import os, sys, subprocess, shutil
import cfgFile, testSeqs, psnr, stopwatch, calcMD5CheckSum


### read config ###
assert len(sys.argv) == 2, "expected one argument"
print "reading configuration '%s'\n" % sys.argv[1]
conf = cfgFile.cfgFile(inputFile=sys.argv[1])

workDir      = conf['workDir']
simId        = conf['simId']
binaries     = conf['binaries']

seqLayers = []
widths    = []
heights   = []
for attr in conf['seqLayers']:
    seqLayers.append(testSeqs.testSeq(*attr))
    widths.append(attr[2])
    heights.append(attr[3])

encOpts      = os.path.expandvars(conf['encOpts'])
decOpts      = [os.path.expandvars(d) for d in conf['decOpts']]
encDir       = os.path.expandvars(conf['encDir'])
recDir       = os.path.expandvars(conf['recDir'])
decDir       = os.path.expandvars(conf['decDir'])
transDir     = os.path.expandvars(conf['transDir'])

encFile      = os.path.expandvars(conf['encFile'])
recFile      = [os.path.expandvars(r) for r in conf['recFile']]
decFile      = [os.path.expandvars(d) for d in conf['decFile']]
transFile    = [os.path.expandvars(t) for t in conf['transFile']]
encode       = conf['encode']
decode       = conf['decode']
removeEnc    = conf['removeEnc']
removeRec    = conf['removeRec']
removeDec    = conf['removeDec']
preproc      = conf['preproc']
postproc     = conf['postproc']
calcMD5CheckSums = conf['calcMD5CheckSums']
if preproc:
    preprocScript  = conf['preprocScript']
    preprocOpts    = [os.path.expandvars(p) for p in conf['preprocOpts']]
    preprocFile    = [os.path.expandvars(p) for p in conf['preprocFile']]
    preprocFile2   = [os.path.expandvars(p) for p in conf['preprocFile2']]
    removePreproc  = conf['removePreproc']
if postproc:
    postprocScript = conf['postprocScript']
    postprocOpts   = [os.path.expandvars(p) for p in conf['postprocOpts']]
    postprocFile   = [os.path.expandvars(p) for p in conf['postprocFile']]
    postprocFile2  = [os.path.expandvars(p) for p in conf['postprocFile2']]
    postprocFile3  = [os.path.expandvars(p) for p in conf['postprocFile3']]
    removePostproc = conf['removePostproc']

# check if parameter exist in configfile
trnOutputLayer  = 0
if conf.countEntries('trnOutputLayer')>0:
  trnOutputLayer  = conf['trnOutputLayer']

numLayers    = len(seqLayers)

encoder      = binaries[0]
decoder      = binaries[1]
if ((numLayers > 1) & (len(binaries)>2)):
  extractor  = binaries[2]
  xtrOpts    = [os.path.expandvars(x) for x in conf['xtrOpts']]
  xtrFile    = os.path.expandvars(conf['xtrFile'])
else:
  extractor = None


### create directories if necessary ###
if not os.path.isdir(encDir):   os.makedirs(encDir)
if not os.path.isdir(recDir):   os.makedirs(recDir)
if not os.path.isdir(decDir):   os.makedirs(decDir)
if not os.path.isdir(transDir): os.makedirs(transDir)

### move to working dir ###
curDir = os.getcwd()
os.chdir(workDir)


### do some checks ###
assert os.path.isfile(encoder), "could not find encoder binary: %s" % encoder
assert os.path.isfile(decoder), "could not find decoder binary: %s" % decoder
#assert numLayers == 1 or os.path.isfile(extractor), "could not find extractor binary: %s" % extractor


### construct file names ###
logEncFile      = 'log_' + simId + '.txt'
encPsnrFile     = []
encRdFile       = []
decPsnrFile     = []
decRdFile       = []
transPsnrFile   = []
transRdFile     = []
logDecFile      = []
logTransFile    = []
logPreprocFile  = []
logPostprocFile = []

for layCnt in range(numLayers):
    encPsnrFile.append(      "%s/psnr_%s_L%d.txt"       % (conf['resultsDir'], simId, layCnt))
    encRdFile.append(        "%s/rd_%s_L%d.txt"         % (conf['resultsDir'], simId, layCnt))
    decPsnrFile.append(      "%s/dec-psnr_%s_L%d.txt"   % (conf['resultsDir'], simId, layCnt))
    decRdFile.append(        "%s/dec-rd_%s_L%d.txt"     % (conf['resultsDir'], simId, layCnt))
    transPsnrFile.append(    "%s/trans-psnr_%s_L%d.txt" % (conf['resultsDir'], simId, layCnt))
    transRdFile.append(      "%s/trans-rd_%s_L%d.txt"   % (conf['resultsDir'], simId, layCnt))

    logDecFile.append(       "log_dec_%s_L%d.txt"       % (simId, layCnt))
    logTransFile.append(     "log_trans_%s_L%d.txt"     % (simId, layCnt))
    logPreprocFile.append(   "log_preproc_%s_L%d.txt"   % (simId, layCnt))
    logPostprocFile.append(  "log_postproc_%s_L%d.txt"  % (simId, layCnt))

### figure out what to do ###
if decode:
    decode = False
    for layCnt in range(numLayers):
        if (not os.path.isfile(decRdFile[layCnt])) or (not os.path.isfile(decPsnrFile[layCnt])) or (not os.path.isfile(decFile[layCnt])):
            decode = True
            break
#encode = False
#if decode and (not os.path.isfile(encFile)):
#    encode = True
#else:
#    for layCnt in range(numLayers):
#        if not os.path.isfile(encRdFile[layCnt]):
#            encode = True
#            break
# Below two lines can be uncommented to force decoding without encoding when bitstreams are available
#encode = False
#decode = True
if encode and decode:
    print "will do encoding and decoding\n"
elif encode:
    print "will do encoding only\n"
elif decode:
    print "will do decoding only\n"
else:
    print "nothing to do\n"


# initialize stop watch
stopWatch = stopwatch.StopWatch()

### do preprocessing ###
for layCnt in range(numLayers):
    if preproc[layCnt]:

        ### run preprocessing ###
        logFile = open(logPreprocFile[layCnt], 'w')

        cmd = "python %s %s" % (preprocScript, preprocOpts[layCnt])
        print "running preprocessing: %s\n" % cmd
        stopWatch.start()
        p = subprocess.Popen(cmd, shell=True, stdout=logFile, stderr=logFile)
        print "preprocessing exit code = %d\n" % p.wait()
        preprocTime = stopWatch.stop()
        if calcMD5CheckSums:
            if preprocFile[layCnt].endswith('.exr'):
                calcMD5CheckSum.calcMD5Sum(preprocFile[layCnt].replace('%05d', '*').replace('%03d', '*').replace('%07d', '*'), workDir)
            else:
                calcMD5CheckSum.calcMD5Sum(preprocFile[layCnt], workDir)
            if preprocFile2[layCnt].endswith('.tif'):
                calcMD5CheckSum.calcMD5Sum(preprocFile2[layCnt].replace('%05d', '*').replace('%03d', '*').replace('%07d', '*'), workDir)
            else:
                calcMD5CheckSum.calcMD5Sum(preprocFile2[layCnt], workDir)

        logFile.close()
        print "Preprocessing time (user, system, all) = %f %f %f\n" % (preprocTime[0], preprocTime[1], preprocTime[2])


### do encoding ###
if encode:

    ### run encoder ###
    cmd = "%s %s" % (encoder, encOpts)
    print "running encoder: %s\n" % cmd
    logFile = open(logEncFile, 'w')
    stopWatch.start()
    p = subprocess.Popen(cmd, shell=True, stdout=logFile, stderr=logFile)
    print "encoder exit code = %d\n" % p.wait()
    encTime = stopWatch.stop()
    if calcMD5CheckSums:
        calcMD5CheckSum.calcMD5Sum(encFile, workDir)
        for f in recFile:
            calcMD5CheckSum.calcMD5Sum(f, workDir)
    logFile.close()

    ### create PSNR and RD files ###

    psnrY = {}
    psnrU = {}
    psnrV = {}
    numLayers2 = numLayers
    # use below if transcoding do not output a reconstruction (only main video is output)
    #if len(decOpts)==1:
    #    numLayers2 = 1        
    for layCnt in range(numLayers2):
    #for layCnt in range(numLayers):
        if removeRec ==4:
          psnrY[layCnt] = 0.0
          psnrU[layCnt] = 0.0
          psnrV[layCnt] = 0.0
          maxpsnrY = 0.0
          minpsnrY = 0.0
          maxpsnrU = 0.0
          minpsnrU = 0.0
          maxpsnrV = 0.0
          minpsnrV = 0.0
          bitrate = 0.0

        elif removeRec !=3:
          if preproc[layCnt]: # Not sure you always want to do this for preprocessing ...
            seqLayers[layCnt].chromaSubsampling = str(conf['InputChromaFormat'])
            seqLayers[layCnt].relPath = preprocFile[layCnt]
            seqLayers[layCnt].startFrame = 0

          print "conf['InternalBitDepth'] " + str(conf['InternalBitDepth'])

          psnrTool = psnr.snrcomp(seqLayers[layCnt], recFile[layCnt], encPsnrFile[layCnt],conf['InternalBitDepth'])
          (psnrY[layCnt], psnrU[layCnt], psnrV[layCnt]) = psnrTool.AvgPSNR()
          maxpsnrY = psnrTool.GetMaxPSNR(psnrTool.psnrY)
          minpsnrY = psnrTool.GetMinPSNR(psnrTool.psnrY)
          maxpsnrU = psnrTool.GetMaxPSNR(psnrTool.psnrU)
          minpsnrU = psnrTool.GetMinPSNR(psnrTool.psnrU)
          maxpsnrV = psnrTool.GetMaxPSNR(psnrTool.psnrV)
          minpsnrV = psnrTool.GetMinPSNR(psnrTool.psnrV)
          psnrTool.MakePSNRFile(psnrY[layCnt], psnrU[layCnt], psnrV[layCnt])

          #get bitrate bps
          if layCnt == numLayers-1:
            bitrate = os.path.getsize(encFile)*8.0*float(seqLayers[layCnt].fpsOut)/float(seqLayers[layCnt].framesOut)
          else:
            if extractor==None: # it will be the total bitrate but better than -1
              bitrate = os.path.getsize(encFile)*8.0*float(seqLayers[layCnt].fpsOut)/float(seqLayers[layCnt].framesOut)
            else:
              bitrate = -1.0
          print "layer %d: %f bps, %f dB\n" % (layCnt, bitrate, psnrY[layCnt])
        else:
          logFile = open(logEncFile, 'r')
          aline = logFile.readline()
          while aline:
           okfind = aline.rfind('SUMMARY')
           if okfind==0:
             aline = logFile.readline()
             aline = logFile.readline()
             astr = str(aline)
             alist = astr.split(None)
             bitrate = float(alist[2])
             psnrY[layCnt] = float(alist[3])
             psnrU[layCnt] = float(alist[4])
             psnrV[layCnt] = float(alist[5])
             # following measures are just dummies
             maxpsnrY = psnrY[layCnt]
             minpsnrY = psnrY[layCnt]
             maxpsnrU = psnrU[layCnt]
             minpsnrU = psnrU[layCnt]
             maxpsnrV = psnrV[layCnt]
             minpsnrV = psnrV[layCnt]
           else:
             aline = logFile.readline()
          logFile.close()

        print "Encoding time (user, system, all) = %f %f %f\n" % (encTime[0], encTime[1], encTime[2])

        #store info to text file
        afile = open(encRdFile[layCnt], 'w')
        afile.write("%s \nBitrate [bps] psnrY psnrU psnrV\n" % cmd)
        afile.write("%f %f %f %f\n" % (bitrate, psnrY[layCnt], psnrU[layCnt], psnrV[layCnt]))
        afile.write("max psnrY psnrU psnrV\n")
        afile.write("%f %f %f\n" % (maxpsnrY, maxpsnrU, maxpsnrV))
        afile.write("min psnrY psnrU psnrV\n")
        afile.write("%f %f %f\n" % (minpsnrY, minpsnrU, minpsnrV))
        afile.write("%f %f %f\n" % (encTime[0], encTime[1], encTime[2]))
        afile.close()

        #clean up reconstructed video
        if removeRec:
         if removeRec!=3:
            print "removing file %s\n" % recFile[layCnt]
            os.remove(recFile[layCnt])


# Files for guided transcoding
HOME_PATH               = "%s/.." % curDir
#HOME_PATH               = "C:\Users/eharano/hevc_transcoding-eharano"
guided_transcoder       = HOME_PATH + "/batch/guided_transcoding.bat"
guided_transcoder_linux = HOME_PATH + "/shell/guided_transcoding.sh"
transcoder_cfg          = HOME_PATH + "/HM-16.6/cfg/encoder_intra_main.cfg"

### do decoding ###
mismatch = False
if decode:
    numLayers2 = numLayers
    # after transcoding there is only a single layer bitstream
    if len(decOpts)==1:
        numLayers2 = 1
    for layCnt in range(numLayers2):
    #for layCnt in range(numLayers):
        logFile = open(logDecFile[layCnt], 'w')

        if layCnt == numLayers-1:
            tmpFile = encFile
        else:
            ### run extractor ###
            if extractor!=None:
             cmd = "%s %s" % (extractor, xtrOpts[layCnt])
             print "running extractor: %s\n" % cmd
             p = subprocess.Popen(cmd, shell=True, stdout=logFile, stderr=None)
             print "extractor exit code = %d\n" % p.wait()
             tmpFile = xtrFile
            else:
             tmpFile = encFile

        ### run decoder ###
        cmd = "%s %s" % (decoder, decOpts[layCnt])
        print "running decoder: %s\n" % cmd
        stopWatch.start()
        p = subprocess.Popen(cmd, shell=True, stdout=logFile, stderr=None)
        print "decoder exit code = %d\n" % p.wait()
        decTime = stopWatch.stop()
        if calcMD5CheckSums:
            for f in decFile:
                try:
                    calcMD5CheckSum.calcMD5Sum(f, workDir)
                except:
                    print "WARNING: could not calculate md5 checksum for decoded file %s" % f
        logFile.close()

        
        ### run transcoder ###

        #transFile_windows = os.path.abspath(transFile[layCnt])
        transFile_linux = transFile[layCnt]
        transCmd = "%s %s %s %s %s %s %s" % (guided_transcoder_linux, encFile, decFile[layCnt], widths[layCnt], heights[layCnt], transcoder_cfg, transFile_linux)
        logFile = open(logTransFile[layCnt], 'w')
        subprocess.Popen(transCmd, shell=True, stdout=logFile, stderr=logFile)
        logFile.close()
        

        ### create PSNR and RD files ###
        # Below four lines can be used when generating reconstruction (after post processing or after reconstruction) from bitstreams without need for
        # encoding or pre-processing. Some indentation needed after the else and before decoding time.
        # Set following parameters in RunSimX: encode=0 preproc=0 and removePostproc=0 or removeRec=0. Copy the bitstream directory to a directory with
        # another name. Change the name of the RunSimX correspondingly.
        #if 1:
        # print "no est of PSNR and RD files\n"
        #else:
        if removeDec ==4:
             psnrYdec = 0.0
             psnrUdec = 0.0
             psnrVdec = 0.0
             maxpsnrY = 0.0
             minpsnrY = 0.0
             maxpsnrU = 0.0
             minpsnrU = 0.0
             maxpsnrV = 0.0
             minpsnrV = 0.0
        elif removeDec ==3:
             # dummies
             psnrYdec = 0.0
             psnrUdec = 0.0
             psnrVdec = 0.0
             maxpsnrY = 0.0
             minpsnrY = 0.0
             maxpsnrU = 0.0
             minpsnrU = 0.0
             maxpsnrV = 0.0
             minpsnrV = 0.0
             logFile = open(logDecFile[layCnt], 'r')
             aline = logFile.readline()
             while aline:
               okfind = aline.rfind('POC')
               if okfind==0:
                 ismatch = aline.rfind('OK')
                 if ismatch==-1:
                   mismatch = True
                 aline = logFile.readline()
               else:
                aline = logFile.readline()
             logFile.close()
             if mismatch:
               print "************************** Encoder/decoder mismatch!! **************************\n"
        else:
          if numLayers2 == 1:
            psnrTool = psnr.snrcomp(seqLayers[trnOutputLayer], decFile[0], decPsnrFile[0], conf['InternalBitDepth'])
            
            #psnrTool_trans = psnr.snrcomp(seqLayers[trnOutputLayer], os.path.abspath(transFile[0]), transPsnrFile[0], conf['InternalBitDepth'])
            
            #tf = "C:/Users/eharano/hevc_transcoding-eharano/VideoSimScript/Results/trans/i_main_RunSimHM16.6/trans_RaceHorses_832x480_30qp37_L0.yuv"
            #psnrTool_trans = psnr.snrcomp(seqLayers[trnOutputLayer], os.path.abspath(tf), transPsnrFile[0], conf['InternalBitDepth'])
          else:
            psnrTool = psnr.snrcomp(seqLayers[layCnt], decFile[layCnt], decPsnrFile[layCnt], conf['InternalBitDepth'])
            #psnrTool_trans = psnr.snrcomp(seqLayers[layCnt], transFile[layCnt], transPsnrFile[layCnt], conf['InternalBitDepth'])
            #Skala ner originalvideon

          #psnrTool = psnr.snrcomp(seqLayers[layCnt], decFile[layCnt], decPsnrFile[layCnt], conf['InternalBitDepth'])
          (psnrYdec, psnrUdec, psnrVdec) = psnrTool.AvgPSNR()
          maxpsnrY = psnrTool.GetMaxPSNR(psnrTool.psnrY)
          minpsnrY = psnrTool.GetMinPSNR(psnrTool.psnrY)
          maxpsnrU = psnrTool.GetMaxPSNR(psnrTool.psnrU)
          minpsnrU = psnrTool.GetMinPSNR(psnrTool.psnrU)
          maxpsnrV = psnrTool.GetMaxPSNR(psnrTool.psnrV)
          minpsnrV = psnrTool.GetMinPSNR(psnrTool.psnrV)
          psnrTool.MakePSNRFile(psnrYdec, psnrUdec, psnrVdec)

          """
          (psnrYtrans, psnrUtrans, psnrVtrans) = psnrTool_trans.AvgPSNR()
          maxpsnrY_trans = psnrTool_trans.GetMaxPSNR(psnrTool_trans.psnrY)
          minpsnrY_trans = psnrTool_trans.GetMinPSNR(psnrTool_trans.psnrY)
          maxpsnrU_trans = psnrTool_trans.GetMaxPSNR(psnrTool_trans.psnrU)
          minpsnrU_trans = psnrTool_trans.GetMinPSNR(psnrTool_trans.psnrU)
          maxpsnrV_trans = psnrTool_trans.GetMaxPSNR(psnrTool_trans.psnrV)
          minpsnrV_trans = psnrTool_trans.GetMinPSNR(psnrTool_trans.psnrV)
          psnrTool_trans.MakePSNRFile(psnrYtrans, psnrUtrans, psnrVtrans)
          """

          if not encode:
            print "************************** Could not check encoder/decoder match **************************\n"
          else:
            if (psnrYdec!=psnrY[layCnt]) or (psnrUdec!=psnrU[layCnt]) or (psnrVdec!=psnrV[layCnt]):
                print "************************** Encoder/decoder mismatch!! **************************\n"
                mismatch = True

          #get bitrate bps
          print conf.countEntries('trnOutputLayer')
          if conf.countEntries('trnOutputLayer')>0:
              # get bitrate from transcoded bitstream instead of enc bitstream
              thelen = len(tmpFile)
              print thelen
              print tmpFile
              tmpFile2 = tmpFile[0:thelen-4]
              tmpFile2 = tmpFile2 + 'T.bin'
              print tmpFile2
              bitrate = os.path.getsize(tmpFile2)*8.0*float(seqLayers[layCnt].fpsOut)/float(seqLayers[layCnt].framesOut)
          else:      
              bitrate = os.path.getsize(tmpFile)*8.0*float(seqLayers[layCnt].fpsOut)/float(seqLayers[layCnt].framesOut)
          print "layer %d: %f bps, %f dB\n" % (layCnt, bitrate, psnrYdec)

        print "Decoding time (user, system, all) = %f %f %f\n" % (decTime[0], decTime[1], decTime[2])

        #store info to text file
        afile = open(decRdFile[layCnt], 'w')
        afile.write("%s \nBitrate [bps] psnrY psnrU psnrV\n" % cmd)
        afile.write("%f %f %f %f\n" % (bitrate, psnrYdec, psnrUdec, psnrVdec))
        afile.write("max psnrY psnrU psnrV\n")
        afile.write("%f %f %f\n" % (maxpsnrY, maxpsnrU, maxpsnrV))
        afile.write("min psnrY psnrU psnrV\n")
        afile.write("%f %f %f\n" % (minpsnrY, minpsnrU, minpsnrV))
        afile.write("%f %f %f\n" % (decTime[0], decTime[1], decTime[2]))
        afile.close()

        """
        bfile = open(transRdFile[layCnt], 'w')
        bfile.write("%s \nBitrate [bps] psnrY psnrU psnrV\n" % cmd)
        #bfile.write("%f %f %f %f\n" % (bitrate, psnrYdec, psnrUdec, psnrVdec))
        bfile.write("(bitrate) %f %f %f\n" % (psnrYtrans, psnrUtrans, psnrVtrans))
        bfile.write("max psnrY psnrU psnrV\n")
        bfile.write("%f %f %f\n" % (maxpsnrY_trans, maxpsnrU_trans, maxpsnrV_trans))
        bfile.write("min psnrY psnrU psnrV\n")
        bfile.write("%f %f %f\n" % (minpsnrY_trans, minpsnrU_trans, minpsnrV_trans))
        #bfile.write("%f %f %f\n" % (decTime[0], decTime[1], decTime[2]))
        bfile.close()
        """


        ### run postprocessing ###
        if postproc[layCnt]:
            logFile = open(logPostprocFile[layCnt], 'w')
            cmd = "python %s %s" % (postprocScript, postprocOpts[layCnt])
            print "running postprocessing: %s\n" % cmd
            stopWatch.start()
            p = subprocess.Popen(cmd, shell=True, stdout=logFile, stderr=logFile)
            print "postprocessing exit code = %d\n" % p.wait()
            postprocTime = stopWatch.stop()
            if calcMD5CheckSums:
                if postprocFile[layCnt].endswith('.exr'):
                    calcMD5CheckSum.calcMD5Sum(postprocFile[layCnt].replace('%05d', '*').replace('%03d', '*').replace('%07d', '*'), workDir)
                else:
                    calcMD5CheckSum.calcMD5Sum(postprocFile[layCnt], workDir)
                if postprocFile2[layCnt].endswith('.tif'):
                    calcMD5CheckSum.calcMD5Sum(postprocFile2[layCnt].replace('%05d', '*').replace('%03d', '*').replace('%07d', '*'), workDir)
                else:
                    calcMD5CheckSum.calcMD5Sum(postprocFile2[layCnt], workDir)
                calcMD5CheckSum.calcMD5Sum(postprocFile3[layCnt], workDir)
            logFile.close()

            print "Postprocessing time (user, system, all) = %f %f %f\n" % (postprocTime[0], postprocTime[1], postprocTime[2])

        #clean up
        if numLayers>1:
          if extractor!=None:
            if os.path.isfile(xtrFile):
                print "removing file %s\n" % xtrFile
                os.remove(xtrFile)
        if removeDec:
            if removeDec!=3:
              print "removing file %s\n" % decFile[layCnt]
              os.remove(decFile[layCnt])

if ((encode==0) & (decode==0)):
        ### run postprocessing ###
        layCnt = 0 # single layer for postprocessing
        if postproc[layCnt]:
            logFile = open(logPostprocFile[layCnt], 'w')
            cmd = "python %s %s" % (postprocScript, postprocOpts[layCnt])
            print "running postprocessing: %s\n" % cmd
            stopWatch.start()
            p = subprocess.Popen(cmd, shell=True, stdout=logFile, stderr=logFile)
            print "postprocessing exit code = %d\n" % p.wait()
            postprocTime = stopWatch.stop()
            if calcMD5CheckSums:
                if postprocFile[layCnt].endswith('.exr'):
                    calcMD5CheckSum.calcMD5Sum(postprocFile[layCnt].replace('%05d', '*').replace('%03d', '*').replace('%07d', '*'), workDir)
                else:
                    calcMD5CheckSum.calcMD5Sum(postprocFile[layCnt], workDir)
            logFile.close()

            print "Postprocessing time (user, system, all) = %f %f %f\n" % (postprocTime[0], postprocTime[1], postprocTime[2])

if removeEnc:
    print "removing file %s\n" % encFile
    os.remove(encFile)


for layCnt in range(numLayers):
    if preproc[layCnt] and removePreproc:
        for f in [preprocFile[layCnt], preprocFile2[layCnt]]:
            print "trying to remove " + f
            parentFolder = f.rsplit(os.sep,1)[0]
            if f.endswith('.exr') and os.path.isdir(parentFolder):
                print "removing exr files in dir %s\n" % parentFolder
                for afile in os.listdir(parentFolder):
                    if afile.endswith('.exr'):
                        os.remove(parentFolder+os.sep+afile)
            elif os.path.isfile(f):
                print "removing file %s\n" % f
                os.remove(f)
            elif os.path.isdir(f):
                print "removing files in dir %s\n" % f
                for afile in os.listdir(f):
                    os.remove(f+os.sep+afile)


    if postproc[layCnt] and removePostproc:
        print "trying to remove " + postprocFile[layCnt]
        parentFolder = postprocFile[layCnt].rsplit(os.sep,1)[0]
        if postprocFile[layCnt].endswith('.exr') and os.path.isdir(parentFolder):
            print "removing exr files in dir %s\n" % parentFolder
            for afile in os.listdir(parentFolder):
                if afile.endswith('.exr'):
                    os.remove(parentFolder+os.sep+afile)
        elif os.path.isfile(postprocFile[layCnt]):
            print "removing file %s\n" % postprocFile[layCnt]
            os.remove(postprocFile[layCnt])
        elif os.path.isdir(postprocFile[layCnt]):
            print "removing files in dir %s\n" % postprocFile[layCnt]
            for afile in os.listdir(postprocFile[layCnt]):
                    os.remove(postprocFile[layCnt]+os.sep+afile)


if mismatch:
  print "The encoder output does not match the decoder output.\nFor details check rd, psnr and str files for the decoder.\n"
