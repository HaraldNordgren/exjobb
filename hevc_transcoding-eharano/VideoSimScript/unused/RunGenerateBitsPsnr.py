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
[resultsDir1, dummy] = os.path.split(inputFile1)
[resultsDir2, dummy] = os.path.split(inputFile2) 
resultsSubDir1 = resultsDir1+'/'+conf1['resultsSubDir']
resultsSubDir2 = resultsDir2+'/'+conf2['resultsSubDir']
logSubDir1 = conf1['resultsSubDir']
logSubDir1 = logSubDir1[3:len(logSubDir1)]
logSubDir1 = 'jobs/' + logSubDir1
logSubDir2 = conf2['resultsSubDir']
logSubDir2 = logSubDir2[3:len(logSubDir2)]
logSubDir2 = 'jobs/' + logSubDir2

print logSubDir1
numQps = len(simIds1[0])

#### do some checks ###
for seqCnt in range(len(seqs)):
    assert len(simIds1[seqCnt]) == numQps
    assert len(simIds2[seqCnt]) == numQps


########################################

resultsTotBits = {}
resultsBits = {}
resultsPSNRY = {}
resultsPSNRU = {}
resultsPSNRV = {}
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
#for seq in range(1):
for seq in range(len(seqs)):
    resultsTotBits[seq] = ([], [], [], [], [])
    resultsBits[seq] = ([], [], [], [], [])
    resultsPSNRY[seq] = ([], [], [], [], [])
    resultsPSNRU[seq] = ([], [], [], [], [])
    resultsPSNRV[seq] = ([], [], [], [], [])
    #results[seq] = ([], [], [], [], [], [])
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
        #get bits 
        # NOTE it assumes that the encoding order is the same as decoding order
        # If this is not the case a reordering is needed when plotting the bits per frame
        theFile = simIds1[seq][qpCnt]
        theFile = theFile[0:len(theFile)-3];
        textEnc = logSubDir1 + '/' + theFile + '/log_' + theFile + '.txt'
        print textEnc

        ok = os.path.isfile(textEnc)
        if(ok):
          afile = open(textEnc,'r')
          run = 1
          while(run): 
            aline = afile.readline()

            astr = str(aline)
            alist = astr.split(' ')
            pos=0
            thelen = len(alist)
            while(ok & (pos<thelen)):
              if(alist[pos]=='Sequence'):
                #find header bits
                while((pos<thelen)): # & (alist[pos].isdigit()==0)):
                  pos=pos+1
                headerbits=float(alist[pos-1])
                ok=0
                run=0
              pos=pos+1
          aline = afile.readline()
          astr = str(aline)
          alist = astr.split(' ')
          run = 1
          while(run): 

           thelen = len(alist)
           pos=0
           ok=1
           while(ok & (pos<thelen)):
             if(alist[pos]=='uneven'):
               ok=0
             if((alist[pos]=='I')|(alist[pos]=='P')|(alist[pos]=='B')):
               pos=pos+1
               while(alist[pos]==''):
                 pos=pos+1
               bits = float(alist[pos])+headerbits
               headerbits=0
               resultsBits[seq][qpCnt].append(bits)               
               ok = 0
             pos = pos+1
           if(pos==thelen):
             run=0

           aline = afile.readline()
           astr = str(aline)
           alist = astr.split(' ')
     
          afile.close()
    
        else:
          theseqOK = 0
          text = textEnc + 'NOT FOUND\n'
          print text

        #get psnr 
        textEnc = resultsSubDir1 + '/psnr_' + simIds1[seq][qpCnt] + ".txt"
        ok = os.path.isfile(textEnc)
        if(ok):
          afile = open(textEnc,'r')
          while(ok):
            aline = afile.readline()
            astr = str(aline)
            alist = astr.split(' ')
            if(alist[0] == 'total'):
               ok = 0
            else:
               ok = 1
	       psnrY = float(alist[5])
               psnrU = float(alist[6])
               psnrV = float(alist[7])
               resultsPSNRY[seq][qpCnt].append(psnrY)
               resultsPSNRU[seq][qpCnt].append(psnrU)
               resultsPSNRV[seq][qpCnt].append(psnrV)
          afile.close()
    
        else:
          theseqOK = 0
          text = textEnc + 'NOT FOUND\n'
          print text

        # get total bits
        textEnc = resultsSubDir1 + '/rd_' + simIds1[seq][qpCnt] + ".txt"
        ok = os.path.isfile(textEnc)
        if(ok):
          afile = open(textEnc,'r')
          commandref = afile.readline()
          aline = afile.readline()
          aline = afile.readline()
          astr = str(aline)
          alist = astr.split(' ')
          bitrate = float(alist[0])/8.0                     
          bits = bitrate*float(seqs1[seq].framesOut)/float(seqs1[seq].fpsOut)
          bytes = bits
          resultsTotBits[seq][qpCnt].append(bytes)
        else:
          theseqOK = 0
          text = textEnc + 'NOT FOUND\n'
          print text

        


# print results into excel
outFileExcel = "%s/bits_%s.xls" % (resultsDir2, simSettings1)

 
plotter = ExcelPlot.ExcelPlotter(outFileExcel)
for seq in range(len(seqOK)):
  plotter.plotBits(seqs[seq].name, resultsPSNRY[seq], resultsPSNRU[seq], resultsPSNRV[seq], resultsBits[seq], resultsTotBits[seq])

plotter.save()


