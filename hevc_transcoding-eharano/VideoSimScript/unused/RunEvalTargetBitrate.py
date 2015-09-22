import os, sys, math
import ExcelPlot, BDelta, cfgFile, testSeqs
########################################
## 
## Output files
##
########################################


########################################
##
## Configuration
##
########################################

### read config ###
assert len(sys.argv) == 3, "expected two arguments"
inputFile1 = sys.argv[1]
inputFile2 = sys.argv[2]
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
cfgSettings1 = conf1['cfgfile']
cfgSettings2 = conf1['cfgfile']
[resultsDir1, dummy] = os.path.split(inputFile1)
[resultsDir2, dummy] = os.path.split(inputFile2) 
resultsSubDir1 = resultsDir1+'/'+conf1['resultsSubDir']
resultsSubDir2 = resultsDir2+'/'+conf2['resultsSubDir']
numQps = len(simIds1[0])

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
mismatch1 = 0
mismatch2 = 0
for seq in range(len(seqs)):
    results[seq] = ([], [], [], [], [], [])
    results2[seq] = ([], [], [], [], [], [])
    resultslog[seq] = ([], [], [], [], [], [])
    resultslog2[seq] = ([], [], [], [], [], [])
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
          encTime1_usr = 0.0
          encTime1_sys = 0.0
          encTime1_all = 0.0

        if decode1:
            text = resultsSubDir1 + '/dec-rd_' + simIds1[seq][qpCnt] + ".txt"
        else:
            text = resultsSubDir1 + '/rd_' + simIds1[seq][qpCnt] + ".txt"
        text2 = "Reading anchor info %s" % text
        print text2

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
          print bitrate
          print psnrY

          results[seq][0].append(bitrate/1000)
          results[seq][1].append(psnrY)
          results[seq][2].append(psnrU)
          results[seq][3].append(psnrV)
          results[seq][4].append([encTime1_usr, encTime1_sys, encTime1_all])
          results[seq][5].append([decTime1_usr, decTime1_sys, decTime1_all])
          resultslog[seq][0].append(bitrate)
          resultslog[seq][1].append(psnrY)
          resultslog[seq][2].append(psnrU)
          resultslog[seq][3].append(psnrV)
          resultslog[seq][4].append([encTime1_usr, encTime1_sys, encTime1_all])
          resultslog[seq][5].append([decTime1_usr, decTime1_sys, decTime1_all])

          rangepsnrY[seq].append(maxpsnrY-minpsnrY)
          rangepsnrU[seq].append(maxpsnrU-minpsnrU)
          rangepsnrV[seq].append(maxpsnrV-minpsnrV)
        else:
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
          encTime2_usr = 0.0
          encTime2_sys = 0.0
          encTime2_all = 0.0

        if decode2:
            text = resultsSubDir2 + '/dec-rd_' + simIds2[seq][qpCnt] + ".txt"
        else:
            text = resultsSubDir2 + '/rd_' + simIds2[seq][qpCnt] + ".txt"
        text2 = "Reading test info %s" % text
        print text2
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
          print bitrate
          print psnrY

          results2[seq][0].append(bitrate/1000)
          results2[seq][1].append(psnrY)
          results2[seq][2].append(psnrU)
          results2[seq][3].append(psnrV)
          results2[seq][4].append([encTime2_usr, encTime2_sys, encTime2_all])
          results2[seq][5].append([decTime2_usr, decTime2_sys, decTime2_all])
          resultslog2[seq][0].append(bitrate)
          resultslog2[seq][1].append(psnrY)
          resultslog2[seq][2].append(psnrU)
          resultslog2[seq][3].append(psnrV)
          resultslog2[seq][4].append([encTime2_usr, encTime2_sys, encTime2_all])
          resultslog2[seq][5].append([decTime2_usr, decTime2_sys, decTime2_all])

          rangepsnrY2[seq].append(maxpsnrY-minpsnrY)
          rangepsnrU2[seq].append(maxpsnrU-minpsnrU)
          rangepsnrV2[seq].append(maxpsnrV-minpsnrV)
        else:
          theseqOK = 0
          text = text + 'NOT FOUND\n'
          print text

        #check if enc and dec match
        if (decode2 and theseqOK):
          if((psnrY!=psnrYEnc) or (psnrU!=psnrUEnc) or (psnrV!=psnrVEnc) or (bitrate!=bitrateEnc)):
            mismatch2=1

    seqOK.append(theseqOK)

# check if test match target bitrate, indicate also if QP should be increase or decreased (using changeQPframe) to better match bitrate

#Class D 256 kbit/s	384 kbit/s	512 kbit/s	850 kbit/s	1.5 Mbit/s
#Class C 384 kbit/s	512 kbit/s	768 kbit/s	1.2 Mbit/s	2 Mbit/s

targetrateC = [384.0, 512.0, 768.0, 1200.0, 2000.0]
targetrateD = [256.0, 384.0, 512.0, 850.0, 1500.0]


print "number of points=%d\n" % numQps

#seqnr = 0
bestQPs = {}
bestcnts = {}
bestsignQPs = {}

#cfgdir = 'cfg_jm/cfg_seq_CS1/'

for seq in range(len(seqs)):
    #seq=6
    bestQPs[seq] = []
    bestsignQPs[seq] = []
    bestcnts[seq] = []

    if(seq>3):  # class C    
      targetrate = targetrateC
    else:
      targetrate = targetrateD

    #print simIds1[seq][0]
    for tnum in range(5):
      bestdist = 10000000
      for cnt in range(numQps):
        dist = targetrate[tnum]-float(resultslog[seq][0][cnt])/1000.0
        if(abs(dist) < bestdist):
          anchorrate = float(resultslog[seq][0][cnt])/1000.0
          bestdist = abs(dist)
          bestcnt = cnt
          if(dist>0):
            QPchange=-1
          else: 
            QPchange=1
      thelen = len(simIds1[seq][bestcnt])
      bestQP = simIds1[seq][bestcnt][thelen-5:thelen-3]

      bestcnts[seq].append(bestcnt)
      deltarate=100.0*float(bestdist)/targetrate[tnum]
      bestQPs[seq].append(simIds1[seq][bestcnt])
      bestsignQPs[seq].append(QPchange)          

      tmplen=len(simSettings1)
      cfgname = 'jobs'+'/'+simSettings1[0:tmplen-3]+'/'+simIds1[seq][bestcnt][0:thelen-3]+'/'+cfgSettings1
      #cfgname = "%s%s%s" % (cfgdir,bestQPs[seq][0][0:thelen-7],'.cfg')
      #print cfgname
      ok = os.path.isfile(cfgname)
      if ok:   
        evalConf = cfgFile.cfgFile(cfgname)
        #qpSet = evalConf['qpSet']
        QPISlice = evalConf['QPISlice']
        QPPSlice = evalConf['QPPSlice']
        QPBSlice = evalConf['QPBSlice']
        ChangeQPI = evalConf['ChangeQPI']
        ChangeQPP = evalConf['ChangeQPP']
        ChangeQPB = evalConf['ChangeQPB']
        ChangeQPFrame = evalConf['ChangeQPFrame']
        #lenqpset = len(qpSet)
        #aqp =  int(bestQP)       
        #bestnum = 0
        #for n in range(lenqpset):
        #  if(aqp == int(qpSet[n][0])):
        #    bestnum = n
        #    #print "best QP is =%d num=%d\n" % (aqp,n)

        #ChangeQPFrameSet = evalConf['ChangeQPFrameSet']
        #ChangeQPISet = evalConf['ChangeQPISet']
        #ChangeQPPSet = evalConf['ChangeQPPSet']
        #ChangeQPBSet = evalConf['ChangeQPBSet']
        #print "%s targetrate=%d anchorrate=%.2f deltarate=%d QPI=%d QPP=%d QPB=%d ChangeQPFrame=%d ChangeQPI=%d ChangeQPP=%d ChangeQPB=%d PSNR Y=%.3f U=%.3f V=%.3f\n" %  (bestQPs[seq][0][0:thelen-7],int(targetrate[tnum]),anchorrate,deltarate,aqp,aqp+1,aqp+2, int(ChangeQPFrameSet[bestnum][0]), int(ChangeQPISet[bestnum][0]), int(ChangeQPPSet[bestnum][0]), int(ChangeQPBSet[bestnum][0]), resultslog[seq][1][bestcnt],resultslog[seq][2][bestcnt],resultslog[seq][3][bestcnt])  
        print "%s targetrate=%d anchorrate=%.2f deltarate=%f QPI=%d QPP=%d QPB=%d ChangeQPFrame=%d ChangeQPI=%d ChangeQPP=%d ChangeQPB=%d PSNR Y=%.3f U=%.3f V=%.3f\n" %  (bestQPs[seq][0][0:thelen-7],int(targetrate[tnum]),anchorrate,deltarate,QPISlice,QPPSlice,QPBSlice, ChangeQPFrame, ChangeQPI, ChangeQPP, ChangeQPB, resultslog[seq][1][bestcnt],resultslog[seq][2][bestcnt],resultslog[seq][3][bestcnt])  
      else:
        print "%s target rate=%f anchor rate=%f deltarate=%f best qp is %s cnt=%d QPchange=%d bestdist=%d\n" % (bestQPs[seq][0][0:thelen-7],targetrate[tnum],anchorrate,deltarate,bestQP,bestcnt, QPchange,bestdist)


#below can be used for writing cfg file with proper QPchange after iteration of QPs      
if(0):
 cfgdir = 'cfg_jm/cfg_seq_CS1_noQP/'
 for seq in range(len(seqs)):

    #print len(bestsignQPs[seq])

    QPlist = []
    QPsignlist = []
    for tnum in range(5):
      thelen = len(simIds1[seq][tnum])
      print bestQPs[seq][tnum][0:thelen-7]
      bestcnt = bestcnts[seq][tnum]
      bestQP = simIds1[seq][bestcnt][thelen-5:thelen-3]     
      QPlist.append(bestQP)
      QPsignlist.append(bestsignQPs[seq][tnum])

    name = "%s%s%s" % (cfgdir,bestQPs[seq][0][0:thelen-7],'.cfg')
    print name
    #evalConf = cfgFile.cfgFile(name)
    #print len(QPlist)
    #qpSet = evalConf['qpSet']

    thestr = "[[%d],[%d],[%d],[%d],[%d]]" % (int(QPlist[0]),int(QPlist[1]),int(QPlist[2]),int(QPlist[3]),int(QPlist[4]))
    print thestr
    #evalConf['qpSet'] = thestr

    thestr = "[[%d],[%d],[%d],[%d],[%d]]" % (int(QPsignlist[0]),int(QPsignlist[1]),int(QPsignlist[2]),int(QPsignlist[3]),int(QPsignlist[4]))
    #print thestr
    #evalConf.removeEntries('ChangeQPISet')
    #evalConf.appendEntry('ChangeQPISet', thestr)
    #evalConf.removeEntries('ChangeQPPSet')
    #evalConf.appendEntry('ChangeQPPSet', thestr)
    #evalConf.removeEntries('ChangeQPBSet')
    #evalConf.appendEntry('ChangeQPBSet', thestr)

    #evalConf.writeCfgFile()

#  if os.path.isfile(evalConfFileName):
#                evalConf = cfgFile.cfgFile(evalConfFileName)
#                if evalConf['resultsSubDir'] != self._resultsSubDir:
#                    raise RunSimException("resultsSubDir not matching in %s (%s, %s)" % (evalConfFileName, evalConf['resultsSubDir'], self._resultsSubDir))
#                if evalConf['simSettings'] != "%s-L%d" % (self._simSettings, layCnt):
#                    raise RunSimException("simSettings not matching in %s (%s, %s)" % (evalConfFileName, evalConf['simSettings'], "%s-L%d" % (self._simSettings, layCnt)))
#                seqOut = evalConf['testSequences']
#                simIdOut = evalConf['simIds']
#            else:
#                evalConf = cfgFile.cfgFile()
          

