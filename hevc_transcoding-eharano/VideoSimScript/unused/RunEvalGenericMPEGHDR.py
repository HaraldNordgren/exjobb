import os, sys, math
import ExcelPlot, BDelta, cfgFile, testSeqs
########################################
##
## Output files
##
########################################
# Uses PSNR from encoder log file for excel plots
# Encoder decoder match still using PSNR computation from reconstructions
# This could be replaced by a diff of the reconstructions instead to possibly speed up simulation time

########################################
##
## Configuration
##
########################################

### read config ###
# python RunEvalGenericMPEGHDR.py Results\anchor Results\test [metric1, ..., metricN]

ANCHOR = 0
TEST   = 1
numarguments = len(sys.argv)
assert ((len(sys.argv) > 2)), "Syntax: %s Results_anchor Results_test testset [metric1, ... , metricN]. \
                               Testset is either HDR, HDRSDR ocr HDRSHVC. If no metrics are given the default \
                               MPEG anchor metrics are used. If no metrics or testset is given HDR is used as testset"

inputFile1 = sys.argv[1]
inputFile2 = sys.argv[2]
if(numarguments == 3):
    # Default testset
    testSet = 'HDR'
else:
    testSet = sys.argv[3].upper()


if numarguments < 5 or testSet == 'HDRSHVC':
    # Default metrics
    metrics = ['Bitrate', 'wtPSNR-X', 'wtPSNR-Y', 'wtPSNR-Z', 'wtPSNR-XYZ', 'psnrY', 'psnrU', 'psnrV', 'psnrYUV', 'wPSNR_DE0100', 'wmfPSNR0', 'EncTime', 'DecTime','PreprocTime','PostprocTime']
else:
    metrics = ['Bitrate'].extend(sys.argv[4:-1])

print "reading evaluation configuration '%s'\n" % inputFile1
conf1 = cfgFile.cfgFile(inputFile=inputFile1)
print "reading evaluation configuration '%s'\n" % inputFile2
conf2 = cfgFile.cfgFile(inputFile=inputFile2)

seqs1 = []
for attr in conf1['testSequences']:
    seqs1.append(testSeqs.testSeq(*attr))
seqs2 = []
for attr in conf2['testSequences']:
    seqs2.append(testSeqs.testSeq(*attr))
seqs = []
tmpSimIds1 = conf1['simIds']
decode = [conf1['decode'],
          conf2['decode']]
tmpSimIds2 = conf2['simIds']
simIds = [[],[]]
for seqCnt1 in range(len(seqs1)):
    for seqCnt2 in range(len(seqs2)):
        if seqs1[seqCnt1] == seqs2[seqCnt2]:
            for existingSeq in seqs:
                if seqs1[seqCnt1].name == existingSeq.name:
                    print "WARNING: duplicate sequence name found, check the following files:"
                    print inputFile1
                    print inputFile2
                    #exit(1) # Duplicate sequences could be allowed in case of SHVC
            seqs.append(seqs1[seqCnt1])
            simIds[ANCHOR].append(tmpSimIds1[seqCnt1])
            simIds[TEST].append(tmpSimIds2[seqCnt2])
            break
simSettings1 = conf1['simSettings']
simSettings2 = conf2['simSettings']
[resultsDir1, dummy] = os.path.split(inputFile1)
[resultsDir2, dummy] = os.path.split(inputFile2)
resultsSubDir = [resultsDir1+'/'+conf1['resultsSubDir'],
                 resultsDir2+'/'+conf2['resultsSubDir']]

ltmp=len(resultsDir1)
jobsDir1 = resultsDir1[0:ltmp-7]
tmp = conf1['resultsSubDir']
ltmp =len(tmp)
jobsDir1 = jobsDir1 + 'jobs/'+tmp[3:ltmp]

ltmp=len(resultsDir2)
jobsDir2 = resultsDir2[0:ltmp-7]
tmp = conf2['resultsSubDir']
ltmp =len(tmp)
jobsDir2 = jobsDir2 + 'jobs/'+tmp[3:ltmp]

jobsDirList = [jobsDir1,jobsDir2]


#    numQps = len(simIds[ANCHOR][0])
#### do some checks ###
#    for seqCnt in range(len(seqs)):
#        assert len(simIds[ANCHOR][seqCnt]) == numQps
#        assert len(simIds[TEST][seqCnt]) == numQps


########################################

results = [{},{}]
resultslog = [{},{}]
TimeData = {}
# this need to have same size as the number of metrics defined above
BDData = [{},{},{},{},{},{},{},{},{},{},{},{}, {}]
BDDataU = {}
BDDataV = {}
BDDataOverlap = [{},{},{},{},{},{},{},{},{},{},{},{}, {}]
BDDataUOverlap = {}
BDDataVOverlap = {}
BDPSNRData = [{},{},{},{},{},{},{},{},{},{},{},{}, {}]
BDPSNRDataU = {}
BDPSNRDataV = {}
BDPSNRDataOverlap = [{},{},{},{},{},{},{},{},{},{},{},{}, {}]
BDPSNRDataUOverlap = {}
BDPSNRDataVOverlap = {}
rangepsnrY2 = {}
rangepsnrU2 = {}
rangepsnrV2 = {}
rangepsnrY = {}
rangepsnrU = {}
rangepsnrV = {}
avgbitrate = {}
avgpsnr = {}
setavgBDY = [0, 0, 0]
setavgBDU = [0, 0, 0]
setavgBDV = [0, 0, 0]
setavgBDYOverlap = [0, 0, 0]
setavgBDUOverlap = [0, 0, 0]
setavgBDVOverlap = [0, 0, 0]
setavgBDPSNRY = [0, 0, 0]
setavgBDPSNRU = [0, 0, 0]
setavgBDPSNRV = [0, 0, 0]
setavgBDPSNRYOverlap = [0, 0, 0]
setavgBDPSNRUOverlap = [0, 0, 0]
setavgBDPSNRVOverlap = [0, 0, 0]
setavgbitrate = [0, 0]
setavgpsnr = [0, 0]
setavgtime = [[0,0,0], [0,0,0]]
seqOK = []
# run encodings
problem=0
mismatch = [0,0]
for seq in range(len(seqs)):
    for metric in metrics:
       #print "seq=%d index=%d\n" % (seq,metrics.index(metric))
       BDData[seq][metrics.index(metric)] = []
       BDDataOverlap[seq][metrics.index(metric)] = []
       BDPSNRData[seq][metrics.index(metric)] = []
       BDPSNRDataOverlap[seq][metrics.index(metric)] = []
    for sim in [ANCHOR, TEST]:
        results[sim][seq] = []#([], [], [], [], [], [])
        resultslog[sim][seq] = []#([], [], [], [], [], [])
    rangepsnrY2[seq] = []
    rangepsnrU2[seq] = []
    rangepsnrV2[seq] = []
    rangepsnrY[seq] = []
    rangepsnrU[seq] = []
    rangepsnrV[seq] = []
    avgbitrate[seq] = []
    avgbitrate[seq].append(0)
    avgbitrate[seq].append(0)
    avgpsnr[seq] = []
    avgpsnr[seq].append(0)
    avgpsnr[seq].append(0)

    theseqOK = 1

    index = 0

    for sim in [ANCHOR, TEST]:

       numQps = len(simIds[ANCHOR][seq])

       #### Check that the number of QPs are the same for anchor and test ###
       assert len(simIds[TEST][seq]) == numQps

       for qpCnt in range(numQps):
          for metric in metrics:
            if (qpCnt == 0):
                results[sim][seq].append([])
                resultslog[sim][seq].append([])

            if metric in ['Bitrate', 'psnrY', 'psnrU', 'psnrV', 'psnrYUV', 'EncTime', 'DecTime']:
              # Get bitrates and psnr to check match between enc and dec
              textEnc = resultsSubDir[sim] + '/rd_' + simIds[sim][seq][qpCnt] + ".txt"
              ok = os.path.isfile(textEnc)
              if(ok):
                afile = open(textEnc,'r')
                commandref = afile.readline()
                aline = afile.readline()
                aline = afile.readline()
                astr = str(aline)
                alist = astr.split(' ')
                bitrateEnc = float(alist[0])
                psnrYEnc = float(alist[1])
                psnrUEnc = float(alist[2])
                psnrVEnc = float(alist[3])
                aline = afile.readline()
                aline = afile.readline()
                aline = afile.readline()
                aline = afile.readline()
                aline = afile.readline()
                astr = str(aline)
                alist = astr.split(' ')
                encTime_usr = float(alist[0])
                encTime_sys = float(alist[1])
                encTime_all = float(alist[2])
              else:
                text = textEnc + 'NOT FOUND\n'
                print text
                theseqOK = 0
                bitrateEnc = 0.0
                psnrYEnc = 0.0
                psnrUEnc = 0.0
                psnrVEnc = 0.0
                encTime_usr = 0.0
                encTime_sys = 0.0
                encTime_all = 0.0

              # get PSNR from log file
              tmp=simIds[sim][seq][qpCnt]
              ltmp=len(tmp)
              textEnc = jobsDirList[sim] + '/' + tmp[0:ltmp-3] + '/log_' + tmp[0:ltmp-3] + ".txt"

              ok = os.path.isfile(textEnc)
              if(ok):
                afile = open(textEnc,'r')
                aline = afile.readline()
                while aline:
                 okfind = aline.rfind('SUMMARY')
                 if okfind==0:
                   if testSet == 'HDRSHVC':
                       aline = afile.readline() # Skip one extra line and only pick L1
                   aline = afile.readline()
                   aline = afile.readline()
                   astr = str(aline)
                   alist = astr.split(None)
                   if testSet == 'HDRSHVC':
                       bitrateEncLog = float(alist[3])
                       psnrYEncLog = float(alist[4])
                       psnrUEncLog = float(alist[5])
                       psnrVEncLog = float(alist[6])
                       psnrYUVEncLog = 0
                   else:
                       bitrateEncLog = float(alist[2])
                       psnrYEncLog = float(alist[3])
                       psnrUEncLog = float(alist[4])
                       psnrVEncLog = float(alist[5])
                       psnrYUVEncLog = float(alist[6])
                 else:
                   aline = afile.readline()
                afile.close()
              else:
                print "No encoder log file found \n"

              if decode[sim]:
                  text = resultsSubDir[sim] + '/dec-rd_' + simIds[sim][seq][qpCnt] + ".txt"
              else:
                  text = resultsSubDir[sim] + '/rd_' + simIds[sim][seq][qpCnt] + ".txt"

              ok = os.path.isfile(text)
              if(ok):
                afile = open(text,'r')
                commandtest = afile.readline()
                aline = afile.readline()
                aline = afile.readline()
                astr = str(aline)
                alist = astr.split(' ')
                bitrate = float(alist[0])
                psnrY = float(alist[1])
                psnrU = float(alist[2])
                psnrV = float(alist[3])
                aline = afile.readline()
                aline = afile.readline()
                astr = str(aline)
                alist = astr.split(' ')
                maxpsnrY = float(alist[0])
                maxpsnrU = float(alist[1])
                maxpsnrV = float(alist[2])
                aline = afile.readline()
                aline = afile.readline()
                astr = str(aline)
                alist = astr.split(' ')
                minpsnrY = float(alist[0])
                minpsnrU = float(alist[1])
                minpsnrV = float(alist[2])
                if decode[sim]:
                  aline = afile.readline()
                  astr = str(aline)
                  alist = astr.split(' ')
                  decTime_usr = float(alist[0])
                  decTime_sys = float(alist[1])
                  decTime_all = float(alist[2])
                else:
                  decTime_usr = 0.0
                  decTime_sys = 0.0
                  decTime_all = 0.0
                afile.close()

                if metric == 'Bitrate':
                  results[sim][seq][metrics.index(metric)].append(bitrateEncLog)
                  resultslog[sim][seq][metrics.index(metric)].append(bitrateEncLog)
                elif metric == 'psnrY':
                  results[sim][seq][metrics.index(metric)].append(psnrYEncLog)
                  resultslog[sim][seq][metrics.index(metric)].append(psnrYEncLog)
                elif metric == 'psnrU':
                  results[sim][seq][metrics.index(metric)].append(psnrUEncLog)
                  resultslog[sim][seq][metrics.index(metric)].append(psnrUEncLog)
                elif metric == 'psnrV':
                  results[sim][seq][metrics.index(metric)].append(psnrVEncLog)
                  resultslog[sim][seq][metrics.index(metric)].append(psnrVEncLog)
                elif metric == 'psnrYUV':
                  results[sim][seq][metrics.index(metric)].append(psnrYUVEncLog)
                  resultslog[sim][seq][metrics.index(metric)].append(psnrYUVEncLog)
                elif metric == 'EncTime':
                  results[sim][seq][metrics.index(metric)].append([encTime_usr, encTime_sys, encTime_all])
                  resultslog[sim][seq][metrics.index(metric)].append([encTime_usr, encTime_sys, encTime_all])
                elif metric == 'DecTime':
                  results[sim][seq][metrics.index(metric)].append([decTime_usr, decTime_sys, decTime_all])
                  resultslog[sim][seq][metrics.index(metric)].append([decTime_usr, decTime_sys, decTime_all])

                rangepsnrY2[seq].append(maxpsnrY-minpsnrY)
                rangepsnrU2[seq].append(maxpsnrU-minpsnrU)
                rangepsnrV2[seq].append(maxpsnrV-minpsnrV)
              else:
                if metric in ['EncTime', 'DecTime']:
                    results[sim][seq][metrics.index(metric)].append([0, 0, 0])
                    resultslog[sim][seq][metrics.index(metric)].append([0, 0, 0])
                else:
                    results[sim][seq][metrics.index(metric)].append(0)
                    resultslog[sim][seq][metrics.index(metric)].append(0)

                theseqOK = 0
                text = text + 'NOT FOUND\n'
                print text


            else:
              # Get Pre- and postproc times
              if metric in ['PreprocTime','PostprocTime']:
                  if metric == 'PreprocTime':
                      preprocLog = jobsDirList[sim] + '/'+ simIds[sim][seq][qpCnt][:-3]+'/log_preproc_' + simIds[sim][seq][qpCnt] + ".txt"

                      if os.path.isfile(preprocLog): # Check if file exists
                        f = open(preprocLog,'r')
                        lines = f.readlines()
                        f.close()
                        valuesSet = False
                        for line in lines:
                            if line.startswith('Total of'):
                                results[sim][seq][metrics.index(metric)].append(float(line.split()[6]))
                                resultslog[sim][seq][metrics.index(metric)].append(float(line.split()[6]))
                                valuesSet = True
                                break
                        if not valuesSet:
                            results[sim][seq][metrics.index(metric)].append(0)
                            resultslog[sim][seq][metrics.index(metric)].append(0)

                      else:
                        print "No preproc log file found \n"
                        results[sim][seq][metrics.index(metric)].append(0)
                        resultslog[sim][seq][metrics.index(metric)].append(0)
                        theseqOK = 0

                  elif metric == 'PostprocTime':
                      postprocLog = jobsDirList[sim] + '/'+ simIds[sim][seq][qpCnt][:-3]+'/log_postproc_' + simIds[sim][seq][qpCnt] + ".txt"

                      if os.path.isfile(postprocLog): # Check if file exists
                        f = open(postprocLog,'r')
                        lines = f.readlines()
                        f.close()
                        stateDetection = False
                        valuesSet = False
                        for line in lines:
                            if line.startswith('Postprocessing anchor for category 1 and 2'):
                                stateDetection = True
                            if line.startswith('Total of') and stateDetection:
                                results[sim][seq][metrics.index(metric)].append(float(line.split()[6]))
                                resultslog[sim][seq][metrics.index(metric)].append(float(line.split()[6]))
                                valuesSet = True
                                break
                        if not valuesSet:
                            results[sim][seq][metrics.index(metric)].append(0)
                            resultslog[sim][seq][metrics.index(metric)].append(0)
                      else:
                        print "No postproc log file found \n"
                        results[sim][seq][metrics.index(metric)].append(0)
                        resultslog[sim][seq][metrics.index(metric)].append(0)
                        theseqOK = 0

              else:
                  # Get results from HDR metrics
                  # Frame# can get the metric names
                  # D_Avg get the average values
                  textEnc = resultsSubDir[sim] + '/hdrmetrics_' + simIds[sim][seq][qpCnt] + ".txt"
                  thelen = len(textEnc)
                  textEnc = textEnc[0:thelen-7] + ".txt"

                  if os.path.isfile(textEnc): # Check if file exists
                    f = open(textEnc,'r')
                    lines = f.readlines()
                    f.close()
                    pos = -1
                    for line in lines:
                      if line.startswith('Frame#'):
                        try:
                          pos = {w:n for n,w in enumerate(line.split())}[metric]
                        except:
                            try:
                                if metric[0] == 'w':
                                    pos = {w:n for n,w in enumerate(line.split())}[metric[1:]]
                                else: raise Exception()
                            except:
                                print "Warning: %s not found in %s" % (metric,textEnc)
                      if 'D_Avg' in line:
                        if pos > -1:
                          results[sim][seq][metrics.index(metric)].append(float(line.split()[pos]))
                          resultslog[sim][seq][metrics.index(metric)].append(float(line.split()[pos]))
                        else:
                          results[sim][seq][metrics.index(metric)].append(0)
                          resultslog[sim][seq][metrics.index(metric)].append(0)
                    theseqOK = 1
                  else:
                    print "No hdrmetric encoder log file found \n"
                    results[sim][seq][metrics.index(metric)].append(0)
                    resultslog[sim][seq][metrics.index(metric)].append(0)
                    theseqOK = 0

          #check if enc and dec match
          if (decode[sim] and theseqOK):
              if((psnrY!=psnrYEnc) or (psnrU!=psnrUEnc) or (psnrV!=psnrVEnc) or (bitrate!=bitrateEnc)):
                  mismatch[sim]=1

    seqOK.append(theseqOK)

# get BD data
avgOverlap = 0
lowOverlap = 0
highOverlap = 0
seqnr = 0
for seq in range(len(seqs)):
    # BDelta is currently reusing the input variables thats why I duplicated the result variables
    # so that the Excel plotting can use the unmodified bitrates

  if(seqOK[seq]):
    for sim in [ANCHOR, TEST]:
        for i in range(len(resultslog[sim][seq][0])):
             if resultslog[sim][seq][metrics.index('Bitrate')][i] != 0:
                resultslog[sim][seq][metrics.index('Bitrate')][i] = math.log10(resultslog[sim][seq][metrics.index('Bitrate')][i])

        avgenctime1 = [0, 0, 0]
        for i in range(numQps):
            for j in range(3):
                avgenctime1[j] = avgenctime1[j] + results[ANCHOR][seq][metrics.index('EncTime')][i][j]
        avgenctime2 = [0, 0, 0]
        for i in range(numQps):
            for j in range(3):
                avgenctime2[j] = avgenctime2[j] + results[TEST][seq][metrics.index('EncTime')][i][j]
        avgdectime1 = [0, 0, 0]
        for i in range(numQps):
            for j in range(3):
                avgdectime1[j] = avgdectime1[j] + results[ANCHOR][seq][metrics.index('DecTime')][i][j]
        avgdectime2 = [0, 0, 0]
        for i in range(numQps):
            for j in range(3):
                avgdectime2[j] = avgdectime2[j] + results[TEST][seq][metrics.index('DecTime')][i][j]

    TimeData[seqnr] = {}
    for j in range(3):
        denctime = avgenctime2[j]-avgenctime1[j]
        if(avgenctime1[j]!=0):
          denctime = 100.0 * (float(denctime)/avgenctime1[j])
        else:
          denctime = 0.0

        ddectime = avgdectime2[j]-avgdectime1[j]
        if(avgdectime1[j]!=0):
          ddectime = 100.0 * (float(ddectime)/avgdectime1[j])
        else:
          ddectime = 0.0

        setavgtime[0][j] = setavgtime[0][j] + denctime
        setavgtime[1][j] = setavgtime[1][j] + ddectime

        TimeData[seqnr][j] = [denctime, ddectime]

    # TODO: Improve BD rate below
    if sim==1:
     for metric in metrics:
      if metric in ['wtPSNR-X', 'wtPSNR-Y', 'wtPSNR-Z', 'wtPSNR-XYZ', 'psnrY', 'psnrU', 'psnrV', 'psnrYUV', 'wPSNR_DE0100', 'wmfPSNR0'] and \
              resultslog[ANCHOR][seq][metrics.index('Bitrate')] and resultslog[ANCHOR][seq][metrics.index(metric)] and \
              resultslog[TEST][seq][metrics.index('Bitrate')] and resultslog[TEST][seq][metrics.index(metric)]:

        d = BDelta.NDiff(resultslog[ANCHOR][seq][metrics.index('Bitrate')], resultslog[ANCHOR][seq][metrics.index(metric)], resultslog[TEST][seq][metrics.index('Bitrate')], resultslog[TEST][seq][metrics.index(metric)])
        (average,avgOverlap) = d.bitrateDiff()
        (high,highOverlap) = d.bitrateDiffHigh()
        (low,lowOverlap) = d.bitrateDiffLow()
        #print "Anchor bitrate %f %f %f %f\n" % (results[ANCHOR][seq][metrics.index('Bitrate')][0],results[ANCHOR][seq][metrics.index('Bitrate')][1],results[ANCHOR][seq][metrics.index('Bitrate')][2],results[ANCHOR][seq][metrics.index('Bitrate')][3])
        #print "Anchor wtPSNR-XYZ %f %f %f %f\n" % (resultslog[ANCHOR][seq][metrics.index('wtPSNR-XYZ')][0],resultslog[ANCHOR][seq][metrics.index('wtPSNR-XYZ')][1],resultslog[ANCHOR][seq][metrics.index('wtPSNR-XYZ')][2],resultslog[ANCHOR][seq][metrics.index('wtPSNR-XYZ')][3])
        #print "Test bitrate %f %f %f %f\n" % (results[TEST][seq][metrics.index('Bitrate')][0],results[TEST][seq][metrics.index('Bitrate')][1],results[TEST][seq][metrics.index('Bitrate')][2],results[TEST][seq][metrics.index('Bitrate')][3])
        #print "Test wtPSNR-XYZ %f %f %f %f\n" % (resultslog[TEST][seq][metrics.index('wtPSNR-XYZ')][0],resultslog[TEST][seq][metrics.index('wtPSNR-XYZ')][1],resultslog[TEST][seq][metrics.index('wtPSNR-XYZ')][2],resultslog[TEST][seq][metrics.index('wtPSNR-XYZ')][3])
        print "%s BD metric%s rate=%f low=%f high=%f\n" % (seqs[seq].name, metric, average,low,high)
        BDData[seq][metrics.index(metric)].append(average)
        BDData[seq][metrics.index(metric)].append(high)
        BDData[seq][metrics.index(metric)].append(low)
        BDDataOverlap[seq][metrics.index(metric)].append(avgOverlap)
        BDDataOverlap[seq][metrics.index(metric)].append(highOverlap)
        BDDataOverlap[seq][metrics.index(metric)].append(lowOverlap)
        #
        (average,avgOverlap) = d.psnrDiff()
        (high,highOverlap) = d.psnrDiffHigh()
        (low,lowOverlap) = d.psnrDiffLow()
        BDPSNRData[seq][metrics.index(metric)].append(average)
        BDPSNRData[seq][metrics.index(metric)].append(high)
        BDPSNRData[seq][metrics.index(metric)].append(low)
        BDPSNRDataOverlap[seq][metrics.index(metric)].append(avgOverlap)
        BDPSNRDataOverlap[seq][metrics.index(metric)].append(highOverlap)
        BDPSNRDataOverlap[seq][metrics.index(metric)].append(lowOverlap)
      else:
        # just dummy stuff for things in metric that dont apply to BD rate calculatios
        BDData[seq][metrics.index(metric)].append(0.0)
        BDData[seq][metrics.index(metric)].append(0.0)
        BDData[seq][metrics.index(metric)].append(0.0)
        BDDataOverlap[seq][metrics.index(metric)].append(0.0)
        BDDataOverlap[seq][metrics.index(metric)].append(0.0)
        BDDataOverlap[seq][metrics.index(metric)].append(0.0)
        BDPSNRData[seq][metrics.index(metric)].append(0.0)
        BDPSNRData[seq][metrics.index(metric)].append(0.0)
        BDPSNRData[seq][metrics.index(metric)].append(0.0)
        BDPSNRDataOverlap[seq][metrics.index(metric)].append(0.0)
        BDPSNRDataOverlap[seq][metrics.index(metric)].append(0.0)
        BDPSNRDataOverlap[seq][metrics.index(metric)].append(0.0)

    #
    # for i in range(3):
    #   setavgBDY[i] = setavgBDY[i] + BDData[seqnr][i]
    #   setavgBDYOverlap[i] = setavgBDYOverlap[i] + BDDataOverlap[seqnr][i]
    #   setavgBDPSNRY[i] = setavgBDPSNRY[i] + BDPSNRData[seqnr][i]
    #   setavgBDPSNRYOverlap[i] = setavgBDPSNRYOverlap[i] + BDPSNRDataOverlap[seqnr][i]
    #

# TODO: end improve bd rate (only BD data is used)

    for sim in [ANCHOR, TEST]:
        avgbitrate[seqnr][sim] = sum(results[sim][seq][metrics.index('Bitrate')])/float(len(results[sim][seq][metrics.index('Bitrate')]))
        setavgbitrate[sim] = setavgbitrate[sim] + avgbitrate[seqnr][sim]

        avgpsnr[seqnr][sim] = sum(results[sim][seq][metrics.index('psnrY')])/float(len(results[sim][seq][metrics.index('psnrY')]))
        setavgpsnr[1] = setavgpsnr[1] + avgpsnr[seqnr][1]

        avgpsnrU = sum(results[sim][seq][metrics.index('psnrU')])/float(len(results[sim][seq][metrics.index('psnrU')]))
        avgpsnrV = sum(results[sim][seq][metrics.index('psnrV')])/float(len(results[sim][seq][metrics.index('psnrV')]))

        # avgpsnrU = sum(results[seq][2])/float(len(results[seq][2]))
        # avgpsnrV = sum(results[seq][3])/float(len(results[seq][3]))
        #text = "%f %f %f %f\n" % (avgbitrate[seqnr][0], avgpsnr[seqnr][0], avgpsnrU, avgpsnrV)
        #print text
        #afile.write(text)

        #text = "Test Average bitrate [kbps] and psnrY U V\n"
        #print text
        #afile.write(text)

    seqnr = seqnr + 1
  else:
    text = "Results for all QPs do not exist for %s" % seqs[seq].name
    print text
    # set data to soemthing to be able to do plot of available data
    for sim in [ANCHOR, TEST]:
        for i in range(len(resultslog[sim][seq][0])):
             if(resultslog[sim][seq][0][i]!=0):
                resultslog[sim][seq][0][i] = math.log10(resultslog[sim][seq][0][i])
    TimeData[seqnr] = {}
    for j in range(3):
        TimeData[seqnr][j] = [0, 0]
    BDData[seqnr] = [0, 0, 0]
    BDDataOverlap[seqnr] = [0, 0, 0]
    BDPSNRData[seqnr] = [0, 0, 0]
    BDPSNRDataOverlap[seqnr] = [0, 0, 0]
    BDDataU[seqnr] = [0, 0, 0]
    BDDataUOverlap[seqnr] = [0, 0, 0]
    BDPSNRDataU[seqnr] = [0, 0, 0]
    BDPSNRDataUOverlap[seqnr] = [0, 0, 0]
    BDDataV[seqnr] = [0, 0, 0]
    BDDataVOverlap[seqnr] = [0, 0, 0]
    BDPSNRDataV[seqnr] = [0, 0, 0]
    BDPSNRDataVOverlap[seqnr] = [0, 0, 0]
    seqOK[seq] = 1
    problem=1
    seqnr = seqnr + 1

# print results into excel
outFileExcel = "%s/result_JCTVC_%s_%s.xls" % (resultsDir2, simSettings1, simSettings2)

if(seqnr!=0):
 for i in range(3):
  setavgBDY[i] = setavgBDY[i]/seqnr
  setavgBDU[i] = setavgBDU[i]/seqnr
  setavgBDV[i] = setavgBDV[i]/seqnr
  setavgBDYOverlap[i] = setavgBDYOverlap[i]/seqnr
  setavgBDUOverlap[i] = setavgBDUOverlap[i]/seqnr
  setavgBDVOverlap[i] = setavgBDVOverlap[i]/seqnr
  setavgBDPSNRY[i] = setavgBDPSNRY[i]/seqnr
  setavgBDPSNRU[i] = setavgBDPSNRU[i]/seqnr
  setavgBDPSNRV[i] = setavgBDPSNRV[i]/seqnr
  setavgBDPSNRYOverlap[i] = setavgBDPSNRYOverlap[i]/seqnr
  setavgBDPSNRUOverlap[i] = setavgBDPSNRUOverlap[i]/seqnr
  setavgBDPSNRVOverlap[i] = setavgBDPSNRVOverlap[i]/seqnr

 for i in range(2):
    setavgbitrate [i] = setavgbitrate[i]/seqnr
    setavgpsnr [i] = setavgpsnr[i]/seqnr
    for j in range(3):
        setavgtime[i][j] = setavgtime[i][j]/seqnr

#if not loop:
plotter = ExcelPlot.ExcelPlotter(outFileExcel)
numdecimals=4

if testSet == 'HDR':
    plotter.plotMPEGHDRPage(seqOK,"CT.2020", seqs, list(range(0,9)),results[ANCHOR],results[TEST],simIds[ANCHOR],simIds[TEST], BDData, BDDataU, BDDataV,setavgBDY, setavgBDU, setavgBDV,setavgtime,type,numdecimals) #,lc)
    plotter.plotMPEGHDRPage(seqOK,"class A_prime", seqs, list(range(9,12)),results[ANCHOR],results[TEST],simIds[ANCHOR],simIds[TEST], BDData, BDDataU, BDDataV,setavgBDY, setavgBDU, setavgBDV,setavgtime,type,numdecimals) #,lc)
elif testSet == 'HDRSHVC':
    plotter.plotMPEGHDRSHVCPage(seqOK,"SHVC", seqs, list(range(0,8)),results[ANCHOR],results[TEST],simIds[ANCHOR],simIds[TEST], BDData, BDDataU, BDDataV,setavgBDY, setavgBDU, setavgBDV,setavgtime,type,numdecimals) #,lc)
elif testSet == 'HDRSDR':
    plotter.plotMPEGHDRSDRPage(seqOK,"anchors SDR-HDR", seqs, list(range(0,6)),results[ANCHOR],results[TEST],simIds[ANCHOR],simIds[TEST], BDData, BDDataU, BDDataV,setavgBDY, setavgBDU, setavgBDV,setavgtime,type,numdecimals) #,lc)
elif testSet == 'HDR_709_PROBLEM_SEQS':
    plotter.plotMPEGHDRPage(seqOK,"709 problem sequences", seqs, list(range(0,4)),results[ANCHOR],results[TEST],simIds[ANCHOR],simIds[TEST], BDData, BDDataU, BDDataV,setavgBDY, setavgBDU, setavgBDV,setavgtime,type,numdecimals) #,lc))

#plotter.plotSummary("Summary", seqOK, setavgBDY, setavgBDU, setavgBDV, setavgBDYOverlap, setavgBDUOverlap, setavgBDVOverlap, setavgBDPSNRY, setavgBDPSNRU, setavgBDPSNRV, setavgBDPSNRYOverlap, setavgBDPSNRUOverlap, setavgBDPSNRVOverlap, setavgbitrate, setavgpsnr, seqs, BDData, BDDataU, BDDataV, BDDataOverlap, BDDataUOverlap, BDDataVOverlap, BDPSNRData, BDPSNRDataU, BDPSNRDataV, BDPSNRDataOverlap, BDPSNRDataUOverlap, BDPSNRDataVOverlap, avgbitrate, avgpsnr, TimeData, setavgtime)

#seqnr = 0
#for seq in range(len(seqs)):
#  if(seqOK[seq]):
#    plotter.plot(seqs[seq].name, results[seq][0], results[seq][1], results[seq][2], results[seq][3], results[seq][4], results[seq][5], results2[seq][0], results2[seq][1], results2[seq][2], results2[seq][3], results2[seq][4], results2[seq][5], BDData[seqnr], BDDataU[seqnr], BDDataV[seqnr], BDDataOverlap[seqnr], BDDataUOverlap[seqnr], BDDataVOverlap[seqnr], BDPSNRData[seqnr], BDPSNRDataU[seqnr], BDPSNRDataV[seqnr], BDPSNRDataOverlap[seqnr], BDPSNRDataUOverlap[seqnr], BDPSNRDataVOverlap[seqnr], avgbitrate[seqnr], avgpsnr[seqnr])
#    seqnr = seqnr + 1
plotter.save()

if(mismatch[ANCHOR]):
  errmsg = "Encoder/Decoder mismatch in anchor: %s\n" % simSettings1
  print errmsg
if(mismatch[TEST]):
  errmsg = "Encoder/Decoder mismatch in test: %s\n" % simSettings2
  print errmsg

sumseqOK = 0;
for seq in range(len(seqs)):
  sumseqOK = sumseqOK + seqOK[seq]

if(sumseqOK != len(seqs)):
  for seq in range(len(seqs)):
    if(seqOK[seq]==0):
      print "Results could not be determined for %s\n" % seqs[seq].name

if(problem):
  print "Some sequences could not be determined. This could be ok if the sequences are in the uncompressed set.\n"

