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
assert len(sys.argv) == 4, "expected three arguments, the third argument indicates i:intra, l: low-delay, r:random access"
inputFile1 = sys.argv[1]
inputFile2 = sys.argv[2]
type = sys.argv[3]
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
decode1 = conf1['decode']
tmpSimIds2 = conf2['simIds']
decode2 = conf2['decode']
simIds1 = []
simIds2 = []
for seqCnt1 in range(len(seqs1)):
    for seqCnt2 in range(len(seqs2)):
        if seqs1[seqCnt1] == seqs2[seqCnt2]:
            for existingSeq in seqs:
                if seqs1[seqCnt1].name == existingSeq.name:
                    print "ERROR: duplicate sequence name found, check the following files:"
                    print inputFile1
                    print inputFile2
                    exit(1)
            seqs.append(seqs1[seqCnt1])
            simIds1.append(tmpSimIds1[seqCnt1])
            simIds2.append(tmpSimIds2[seqCnt2])
            break
simSettings1 = conf1['simSettings']
simSettings2 = conf2['simSettings']
[resultsDir1, dummy] = os.path.split(inputFile1)
[resultsDir2, dummy] = os.path.split(inputFile2) 
resultsSubDir1 = resultsDir1+'/'+conf1['resultsSubDir']
resultsSubDir2 = resultsDir2+'/'+conf2['resultsSubDir']

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

numQps = len(simIds1[0])
#numQps = 3
#### do some checks ###
for seqCnt in range(len(seqs)):
    assert len(simIds1[seqCnt]) == numQps
    assert len(simIds2[seqCnt]) == numQps


########################################

results = {}
results2 = {}
resultslog = {}
resultslog2 = {}
TimeData = {}
BDData = {}
BDDataU = {}
BDDataV = {}
BDDataOverlap = {}
BDDataUOverlap = {}
BDDataVOverlap = {}
BDPSNRData = {}
BDPSNRDataU = {}
BDPSNRDataV = {}
BDPSNRDataOverlap = {}
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
mismatch1 = 0
mismatch2 = 0
for seq in range(len(seqs)):
    results[seq] = ([], [], [], [], [], [], [])
    results2[seq] = ([], [], [], [], [], [],[])
    resultslog[seq] = ([], [], [], [], [], [], [])
    resultslog2[seq] = ([], [], [], [], [], [], [])
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
   
    index=0
    # first test
    for qpCnt in range(numQps):

        #get bitrates and psnr to check match between enc and dec 
        textEnc = resultsSubDir1 + '/rd_' + simIds1[seq][qpCnt] + ".txt"
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
          encTime1_usr = float(alist[0])
          encTime1_sys = float(alist[1])
          encTime1_all = float(alist[2])
              
        else:
          theseqOK = 0
          text = textEnc + 'NOT FOUND\n'
          print text
          bitrateEnc = 0.0
          psnrYEnc = 0.0
          psnrUEnc = 0.0
          psnrVEnc = 0.0
          encTime1_usr = 0.0
          encTime1_sys = 0.0
          encTime1_all = 0.0

        # get PSNR from log file
        tmp=simIds1[seq][qpCnt]
        ltmp=len(tmp)
        textEnc = jobsDir1 + '/' + tmp[0:ltmp-3] + '/log_' + tmp[0:ltmp-3] + ".txt"
        ok = os.path.isfile(textEnc)
        if(ok):
          afile = open(textEnc,'r')
          aline = afile.readline()
          while aline:
           okfind = aline.rfind('SUMMARY')
           if okfind==0:
             aline = afile.readline()
             aline = afile.readline()
             astr = str(aline)
             alist = astr.split(None)
             bitrateEnc2 = float(alist[2])
             psnrYEnc2 = float(alist[3])
             psnrUEnc2 = float(alist[4])
             psnrVEnc2 = float(alist[5])
           else:
             aline = afile.readline()
             okfind = aline.rfind('Total number of dQP bits')
             if okfind==0:
               astr = str(aline)
               alist = astr.split(None)
               tmp = alist[6]
               thelen = len(tmp)
               bitrateEncdQP2 = float(tmp[1:thelen])
          afile.close()
        else:
          print "No encoder log file found \n"

        if decode1:
            text = resultsSubDir1 + '/dec-rd_' + simIds1[seq][qpCnt] + ".txt"
        else:
            text = resultsSubDir1 + '/rd_' + simIds1[seq][qpCnt] + ".txt"
        #text2 = "Reading anchor info %s" % text
        #print text2

        ok = os.path.isfile(text)
        if(ok):
          afile = open(text,'r')
          commandref = afile.readline()
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
          if decode1:
            aline = afile.readline()
            astr = str(aline)
            alist = astr.split(' ')
            decTime1_usr = float(alist[0])
            decTime1_sys = float(alist[1])
            decTime1_all = float(alist[2])
          else:
            decTime1_usr = 0.0
            decTime1_sys = 0.0
            decTime1_all = 0.0

          afile.close()
          #print bitrate
          #print psnrY

          results[seq][0].append(bitrateEnc2)
          results[seq][1].append(psnrYEnc2)
          results[seq][2].append(psnrUEnc2)
          results[seq][3].append(psnrVEnc2)
          results[seq][4].append([encTime1_usr, encTime1_sys, encTime1_all])
          results[seq][5].append([decTime1_usr, decTime1_sys, decTime1_all])
          results[seq][6].append(bitrateEncdQP2)
          resultslog[seq][0].append(bitrateEnc2)
          resultslog[seq][1].append(psnrYEnc2)
          resultslog[seq][2].append(psnrUEnc2)
          resultslog[seq][3].append(psnrVEnc2)
          resultslog[seq][4].append([encTime1_usr, encTime1_sys, encTime1_all])
          resultslog[seq][5].append([decTime1_usr, decTime1_sys, decTime1_all])
          resultslog[seq][6].append(bitrateEncdQP2)

          rangepsnrY[seq].append(maxpsnrY-minpsnrY)
          rangepsnrU[seq].append(maxpsnrU-minpsnrU)
          rangepsnrV[seq].append(maxpsnrV-minpsnrV)
        else:
          results[seq][0].append(0)
          results[seq][1].append(0)
          results[seq][2].append(0)
          results[seq][3].append(0)
          results[seq][4].append([0, 0, 0])
          results[seq][5].append([0, 0, 0])
          results[seq][6].append(0)
          resultslog[seq][0].append(0)
          resultslog[seq][1].append(0)
          resultslog[seq][2].append(0)
          resultslog[seq][3].append(0)
          resultslog[seq][4].append([0, 0, 0])
          resultslog[seq][5].append([0, 0, 0])
          resultslog[seq][6].append(0)

          theseqOK = 0
          text = text + 'NOT FOUND\n'
          print text

        #check if enc and dec match
        if (decode1 and theseqOK):
          if((psnrY!=psnrYEnc) or (psnrU!=psnrUEnc) or (psnrV!=psnrVEnc) or (bitrate!=bitrateEnc)):
            mismatch1=1

    # second test
    for qpCnt in range(numQps):
        #get bitrates and psnr to check match between enc and dec 
        textEnc = resultsSubDir2 + '/rd_' + simIds2[seq][qpCnt] + ".txt"
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
          encTime2_usr = float(alist[0])
          encTime2_sys = float(alist[1])
          encTime2_all = float(alist[2])
        else:
          text = textEnc + 'NOT FOUND\n'
          print text
          theseqOK = 0
          bitrateEnc = 0.0
          psnrYEnc = 0.0
          psnrUEnc = 0.0
          psnrVEnc = 0.0
          encTime2_usr = 0.0
          encTime2_sys = 0.0
          encTime2_all = 0.0

        # get PSNR from log file
        tmp=simIds2[seq][qpCnt]
        ltmp=len(tmp)
        textEnc = jobsDir2 + '/' + tmp[0:ltmp-3] + '/log_' + tmp[0:ltmp-3] + ".txt"
        ok = os.path.isfile(textEnc)
        if(ok):
          afile = open(textEnc,'r')
          aline = afile.readline()
          while aline:
           okfind = aline.rfind('SUMMARY')
           if okfind==0:
             aline = afile.readline()
             aline = afile.readline()
             astr = str(aline)
             alist = astr.split(None)
             bitrateEnc2 = float(alist[2])
             psnrYEnc2 = float(alist[3])
             psnrUEnc2 = float(alist[4])
             psnrVEnc2 = float(alist[5])
           else:
             aline = afile.readline()
             okfind = aline.rfind('Total number of dQP bits')
             if okfind==0:
               astr = str(aline)
               alist = astr.split(None)
               tmp = alist[6]
               thelen = len(tmp)
               bitrateEncdQP2 = float(tmp[1:thelen])
          afile.close()
        else:
          print "No encoder log file found \n"

        if decode2:
            text = resultsSubDir2 + '/dec-rd_' + simIds2[seq][qpCnt] + ".txt"
        else:
            text = resultsSubDir2 + '/rd_' + simIds2[seq][qpCnt] + ".txt"
        #text2 = "Reading test info %s" % text
        #print text2
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
          if decode2:
            aline = afile.readline()
            astr = str(aline)
            alist = astr.split(' ')
            decTime2_usr = float(alist[0])
            decTime2_sys = float(alist[1])
            decTime2_all = float(alist[2])
          else:
            decTime2_usr = 0.0
            decTime2_sys = 0.0
            decTime2_all = 0.0
          afile.close()
          #print bitrate
          #print psnrY

          results2[seq][0].append(bitrateEnc2)
          results2[seq][1].append(psnrYEnc2)
          results2[seq][2].append(psnrUEnc2)
          results2[seq][3].append(psnrVEnc2)
          results2[seq][4].append([encTime2_usr, encTime2_sys, encTime2_all])
          results2[seq][5].append([decTime2_usr, decTime2_sys, decTime2_all])
          results2[seq][6].append(bitrateEncdQP2)
          resultslog2[seq][0].append(bitrateEnc2)
          resultslog2[seq][1].append(psnrYEnc2)
          resultslog2[seq][2].append(psnrUEnc2)
          resultslog2[seq][3].append(psnrVEnc2)
          resultslog2[seq][4].append([encTime2_usr, encTime2_sys, encTime2_all])
          resultslog2[seq][5].append([decTime2_usr, decTime2_sys, decTime2_all])
          resultslog2[seq][6].append(bitrateEncdQP2)

          rangepsnrY2[seq].append(maxpsnrY-minpsnrY)
          rangepsnrU2[seq].append(maxpsnrU-minpsnrU)
          rangepsnrV2[seq].append(maxpsnrV-minpsnrV)
        else:
          results2[seq][0].append(0)
          results2[seq][1].append(0)
          results2[seq][2].append(0)
          results2[seq][3].append(0)
          results2[seq][4].append([0, 0, 0])
          results2[seq][5].append([0, 0, 0])
          results2[seq][6].append(0)
          resultslog2[seq][0].append(0)
          resultslog2[seq][1].append(0)
          resultslog2[seq][2].append(0)
          resultslog2[seq][3].append(0)
          resultslog2[seq][4].append([0, 0, 0])
          resultslog2[seq][5].append([0, 0, 0])
          resultslog2[seq][6].append(0)
          theseqOK = 0
          text = text + 'NOT FOUND\n'
          print text

        #check if enc and dec match
        if (decode2 and theseqOK):
          if((psnrY!=psnrYEnc) or (psnrU!=psnrUEnc) or (psnrV!=psnrVEnc) or (bitrate!=bitrateEnc)):
            mismatch2=1

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
    for i in range(len(resultslog[seq][0])):
            resultslog[seq][0][i] = math.log10(resultslog[seq][0][i])
            resultslog2[seq][0][i] = math.log10(resultslog2[seq][0][i])

    avgenctime1 = [0, 0, 0]
    for i in range(numQps):
        for j in range(3):
            avgenctime1[j] = avgenctime1[j] + results[seq][4][i][j]
    avgenctime2 = [0, 0, 0]
    for i in range(numQps):
        for j in range(3):
            avgenctime2[j] = avgenctime2[j] + results2[seq][4][i][j]
    avgdectime1 = [0, 0, 0]
    for i in range(numQps):
        for j in range(3):
            avgdectime1[j] = avgdectime1[j] + results[seq][5][i][j]
    avgdectime2 = [0, 0, 0]
    for i in range(numQps):
        for j in range(3):
            avgdectime2[j] = avgdectime2[j] + results2[seq][5][i][j]

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

    d = BDelta.NDiff(resultslog[seq][0], resultslog[seq][1], resultslog2[seq][0], resultslog2[seq][1])
    (average,avgOverlap) = d.bitrateDiff()     
    (high,highOverlap) = d.bitrateDiffHigh()
    (low,lowOverlap) = d.bitrateDiffLow()
    BDData[seqnr] = [average, high, low]
    BDDataOverlap[seqnr] = [avgOverlap, highOverlap, lowOverlap]

    (average,avgOverlap) = d.psnrDiff()     
    (high,highOverlap) = d.psnrDiffHigh()
    (low,lowOverlap) = d.psnrDiffLow()
    BDPSNRData[seqnr] = [average, high, low]
    BDPSNRDataOverlap[seqnr] = [avgOverlap, highOverlap, lowOverlap]
   
    for i in range(3):
      setavgBDY[i] = setavgBDY[i] + BDData[seqnr][i]
      setavgBDYOverlap[i] = setavgBDYOverlap[i] + BDDataOverlap[seqnr][i]
      setavgBDPSNRY[i] = setavgBDPSNRY[i] + BDPSNRData[seqnr][i]
      setavgBDPSNRYOverlap[i] = setavgBDPSNRYOverlap[i] + BDPSNRDataOverlap[seqnr][i]

    dU = BDelta.NDiff(resultslog[seq][0], resultslog[seq][2], resultslog2[seq][0], resultslog2[seq][2])
    (averageU,avgOverlap) = dU.bitrateDiff()
    (highU,highOverlap) = dU.bitrateDiffHigh()
    (lowU,lowOverlap) = dU.bitrateDiffLow()
    BDDataU[seqnr] = [averageU, highU, lowU]
    BDDataUOverlap[seqnr] = [avgOverlap, highOverlap, lowOverlap]

    (average,avgOverlap) = dU.psnrDiff()     
    (high,highOverlap) = dU.psnrDiffHigh()
    (low,lowOverlap) = dU.psnrDiffLow()
    BDPSNRDataU[seqnr] = [average, high, low]
    BDPSNRDataUOverlap[seqnr] = [avgOverlap, highOverlap, lowOverlap]

    for i in range(3):
      setavgBDU[i] = setavgBDU[i] + BDDataU[seqnr][i]
      setavgBDUOverlap[i] = setavgBDUOverlap[i] + BDDataUOverlap[seqnr][i]
      setavgBDPSNRU[i] = setavgBDPSNRU[i] + BDPSNRDataU[seqnr][i]
      setavgBDPSNRUOverlap[i] = setavgBDPSNRUOverlap[i] + BDPSNRDataUOverlap[seqnr][i]

    dV = BDelta.NDiff(resultslog[seq][0], resultslog[seq][3], resultslog2[seq][0], resultslog2[seq][3])
    (averageV,avgOverlap) = dV.bitrateDiff()
    (highV,highOverlap) = dV.bitrateDiffHigh()
    (lowV,lowOverlap) = dV.bitrateDiffLow()
    BDDataV[seqnr] = [averageV, highV, lowV]
    BDDataVOverlap[seqnr] = [avgOverlap, highOverlap, lowOverlap]

    (average,avgOverlap) = dV.psnrDiff()     
    (high,highOverlap) = dV.psnrDiffHigh()
    (low,lowOverlap) = dV.psnrDiffLow()
    BDPSNRDataV[seqnr] = [average, high, low]
    BDPSNRDataVOverlap[seqnr] = [avgOverlap, highOverlap, lowOverlap]

    for i in range(3):
      setavgBDV[i] = setavgBDV[i] + BDDataV[seqnr][i]
      setavgBDVOverlap[i] = setavgBDVOverlap[i] + BDDataVOverlap[seqnr][i]
      setavgBDPSNRV[i] = setavgBDPSNRV[i] + BDPSNRDataV[seqnr][i]
      setavgBDPSNRVOverlap[i] = setavgBDPSNRVOverlap[i] + BDPSNRDataVOverlap[seqnr][i]

    #text = "%s/BD_%s_%s.txt" % (resultsSubDir1, simSettings1, simSettings2)
    #afile = open(text,'w')
    #text = "%s %s \nLuma: Average BDrate BDlow BDhigh\n" % (simSettings1, simSettings2)
    #print text
    #afile.write(text) 
    #text = "%f %f %f\n" % (average, low, high) 
    #print text
    #afile.write(text) 
    #text = "Chroma U: Average BDrate BDlow BDhigh\n"
    #print text
    #afile.write(text) 
    #text = "%f %f %f\n" % (averageU, lowU, highU) 
    #print text
    #afile.write(text) 

    #text = "Chroma V: Average BDrate BDlow BDhigh\n"
    #print text
    #afile.write(text) 
    #text = "%f %f %f\n" % (averageV, lowV, highV) 
    #print text
    #afile.write(text) 

    #text = "Anchor Average bitrate [kbps] and psnrY\n"
    #print text
    #afile.write(text) 
    avgbitrate[seqnr][0] = sum(results[seq][0])/float(len(results[seq][0]))
    setavgbitrate[0] = setavgbitrate[0] + avgbitrate[seqnr][0]

    avgpsnr[seqnr][0] = sum(results[seq][1])/float(len(results[seq][1]))
    setavgpsnr[0] = setavgpsnr[0] + avgpsnr[seqnr][0]

    avgpsnrU = sum(results[seq][2])/float(len(results[seq][2]))
    avgpsnrV = sum(results[seq][3])/float(len(results[seq][3]))
    #text = "%f %f %f %f\n" % (avgbitrate[seqnr][0], avgpsnr[seqnr][0], avgpsnrU, avgpsnrV) 
    #print text
    #afile.write(text) 

    #text = "Test Average bitrate [kbps] and psnrY U V\n"
    #print text
    #afile.write(text) 
    avgbitrate[seqnr][1] = sum(results2[seq][0])/float(len(results2[seq][0]))
    setavgbitrate[1] = setavgbitrate[1] + avgbitrate[seqnr][1]

    avgpsnr[seqnr][1] = sum(results2[seq][1])/float(len(results2[seq][1]))
    setavgpsnr[1] = setavgpsnr[1] + avgpsnr[seqnr][1]

    avgpsnrU = sum(results2[seq][2])/float(len(results2[seq][2]))
    avgpsnrV = sum(results2[seq][3])/float(len(results2[seq][3]))
    #text = "%f %f %f %f\n" % (avgbitrate[seqnr][1], avgpsnr[seqnr][1], avgpsnrU, avgpsnrV) 
    #print text
    #afile.write(text) 

    #for i in range(numQps):
    #    text = "%s: PSNR range Y U V\n" % simIds1[seq][i]
    #    afile.write(text) 
    #    print text
    #    text = "%f %f %f\n" % (rangepsnrY[seq][i], rangepsnrU[seq][i], rangepsnrV[seq][i]) 
    #    afile.write(text) 
    #    print text
    #for i in range(numQps):
    #    text = "%s: PSNR range Y U V\n" % simIds2[seq][i]
    #    afile.write(text)
    #    print text
    #    text = "%f %f %f\n" % (rangepsnrY2[seq][i], rangepsnrU2[seq][i], rangepsnrV2[seq][i]) 
    #    afile.write(text) 
    #    print text

    #afile.close()
    seqnr = seqnr + 1
  else:
    text = "Results for all QPs do not exist for %s" % seqs[seq].name
    print text
    # set data to soemthing to be able to do plot of available data
    for i in range(len(resultslog[seq][0])):
         if(resultslog[seq][0][i]!=0):
            resultslog[seq][0][i] = math.log10(resultslog[seq][0][i])
         if(resultslog2[seq][0][i]!=0):
            resultslog2[seq][0][i] = math.log10(resultslog2[seq][0][i])
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
 
plotter = ExcelPlot.ExcelPlotter(outFileExcel)
numdecimals=4
plotter.plotJCTVCPagedQP(seqOK,seqs,results,results2,simIds1,simIds2, BDData, BDDataU, BDDataV,setavgBDY, setavgBDU, setavgBDV,setavgtime,type,numdecimals) #,lc)
#plotter.plotSummary("Summary", seqOK, setavgBDY, setavgBDU, setavgBDV, setavgBDYOverlap, setavgBDUOverlap, setavgBDVOverlap, setavgBDPSNRY, setavgBDPSNRU, setavgBDPSNRV, setavgBDPSNRYOverlap, setavgBDPSNRUOverlap, setavgBDPSNRVOverlap, setavgbitrate, setavgpsnr, seqs, BDData, BDDataU, BDDataV, BDDataOverlap, BDDataUOverlap, BDDataVOverlap, BDPSNRData, BDPSNRDataU, BDPSNRDataV, BDPSNRDataOverlap, BDPSNRDataUOverlap, BDPSNRDataVOverlap, avgbitrate, avgpsnr, TimeData, setavgtime)
seqnr = 0
for seq in range(len(seqs)):   
  if(seqOK[seq]):    
    plotter.plot(seqs[seq].name, results[seq][0], results[seq][1], results[seq][2], results[seq][3], results[seq][4], results[seq][5], results2[seq][0], results2[seq][1], results2[seq][2], results2[seq][3], results2[seq][4], results2[seq][5], BDData[seqnr], BDDataU[seqnr], BDDataV[seqnr], BDDataOverlap[seqnr], BDDataUOverlap[seqnr], BDDataVOverlap[seqnr], BDPSNRData[seqnr], BDPSNRDataU[seqnr], BDPSNRDataV[seqnr], BDPSNRDataOverlap[seqnr], BDPSNRDataUOverlap[seqnr], BDPSNRDataVOverlap[seqnr], avgbitrate[seqnr], avgpsnr[seqnr])
    seqnr = seqnr + 1 
plotter.save()

if(mismatch1):
  errmsg = "Encoder/Decoder mismatch: %s\n" % simSettings1
  print errmsg
if(mismatch2):
  errmsg = "Encoder/Decoder mismatch: %s\n" % simSettings2
  print errmsg
  
sumseqOK = 0;
for seq in range(len(seqs)):
  sumseqOK = sumseqOK + seqOK[seq]

if(sumseqOK != len(seqs)):
  for seq in range(len(seqs)):
    if(seqOK[seq]==0):   
      print "Results could not be determined for %s\n" % seqs[seq].name

if(problem):
  print "Some sequences could not be determined\n"

