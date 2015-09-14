from pyExcelerator import *
from decimal import *
import os

class ExcelPlotter:

    def __init__(self, name):
        self.dummy = 1
        self.curpath = os.path.dirname(__file__)
        self.workbook = Workbook()
        self.name = name

    def nth_root(self, num, n, digits):
	getcontext().prec = digits
	a = Decimal(num)
	oneOverN = 1 / Decimal(n)
	nMinusOne = Decimal(n) - 1
	curVal = Decimal(num) / (Decimal(n) ** 2)
	if curVal <= Decimal("1.0"):
		curVal = Decimal("1.1")
	lastVal = 0
	while lastVal != curVal:
		lastVal = curVal
		curVal = oneOverN * ( (nMinusOne * curVal) + (a / (curVal ** nMinusOne)))
	return curVal

    def plotSHVCPage(self, seqOK, seqs, results, simid, BD, BDU, BDV, setavgBDY, setavgBDU, setavgBDV, setavgtime, type, numdecimals, dqpindex, sheettype):

        if(sheettype==2):
          worksheet = self.workbook.add_sheet("SHVC Test")
        elif(sheettype==1):
          worksheet = self.workbook.add_sheet("SHVC Anchor")
        else:
          worksheet = self.workbook.add_sheet("Single layer")

        # results[seq][0..5] contain bitrate1, psnrY1, psnrU1, psnrV1, enctime1, dectime

        #mapping from our sequence order to HSVC order, currently same order
        domap=1
        themap = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19] #no map
        listlen = len(themap)-1
        x = -1
        y = 4

        # if only class B is used an offsets need to be added so that classB is plotted at correct row
        #offsetOnlyClassB = 16
        offsetOnlyClassB = 0
        #for snr scalability the second of the QP offsets (QP+4) is used
        #offsetSNR = 4;
        offsetSNR = 0;

        yoffset=0
        #intra
        if type == "i1":
         # 1.5x
         yoffset=0;
        elif type == "i2":
         # 2x
         yoffset=40+offsetOnlyClassB;
        # ld-P
        elif type == "p0":
         # snr
         yoffset=96+offsetOnlyClassB+offsetSNR;
        elif type == "p1":
         # 1.5x
         yoffset=152;
        elif type == "p2":
         # 2x
         yoffset=192+offsetOnlyClassB;
        # random access
        elif type == "r0":
         # snr
         yoffset=248+offsetOnlyClassB+offsetSNR;
        elif type == "r1":
         # 1.5x
         yoffset=304;
        elif type == "r2":
         # 2x
         yoffset=344+offsetOnlyClassB;
        # low-delay B
        elif type == "b0":
         # snr
         yoffset=400+offsetOnlyClassB+offsetSNR;
        elif type == "b1":
         # 1.5x
         yoffset=456;
        elif type == "b2":
         # 2x
         yoffset=496+offsetOnlyClassB;

        y = y+yoffset

        #Test
        # plot all sequences
        #worksheet.write(y-4, x+2, "bitrate enh")
        #worksheet.write(y-4, x+3, "PSNR Y")
        #worksheet.write(y-4, x+4, "PSNR U")
        #worksheet.write(y-4, x+5, "PSNR V")
        #worksheet.write(y-4, x+6, "bitrate base")
        #worksheet.write(y-4, x+7, "PSNR Y")
        #worksheet.write(y-4, x+8, "PSNR U")
        #worksheet.write(y-4, x+9, "PSNR V")

        #worksheet.write(y-4, x+10, "enctime [s]")
        #worksheet.write(y-4, x+11, "dectime [s]")
        seqnr = 0

        encnum2 = 1.0
        for seq in range(len(seqs)):
         numQps = len(results[seq][0])

         if((dqpindex != 0)&(seq==0)):
           seqnr = seqnr + dqpindex*numQps;
         if(seqOK[seq]):
           if(seq>listlen):
              thenum = seq;
           else:
              thenum = themap[seq]

           for qp in range(numQps):
            if(domap & (numQps==4)):
              mapqp = 3-qp
            else:
              mapqp = qp
            #worksheet.write(y+qp+seqnr, x+1, theqp[mapqp])
            thelen = len(simid[thenum][mapqp])
            thetxt = simid[thenum][mapqp]
            #print "%s\n" % (thetxt)
            worksheet.write(y+qp+seqnr-4, x+1, thetxt)

            val = "%s%s" % (thetxt[thelen-5],thetxt[thelen-4])
            #worksheet.write(y+qp+seqnr, x+1, int(val))
            #bitrate L1 '0.0001'
            if(sheettype>0):
              worksheet.write(y+qp+seqnr-4, x+2, round(results[thenum][10][mapqp],numdecimals))
            else:
              worksheet.write(y+qp+seqnr-4, x+2, round(results[thenum][0][mapqp],numdecimals))
            #PSNR Y
            worksheet.write(y+qp+seqnr-4, x+3, round(results[thenum][1][mapqp],numdecimals))
            #PSNR U
            worksheet.write(y+qp+seqnr-4, x+4, round(results[thenum][2][mapqp],numdecimals))
            #PSNR V
            worksheet.write(y+qp+seqnr-4, x+5, round(results[thenum][3][mapqp],numdecimals))

            if(sheettype>0):

              #bitrate L0 '0.0001'
              worksheet.write(y+qp+seqnr-4, x+6, round(results[thenum][6][mapqp],numdecimals))
              #PSNR Y
              worksheet.write(y+qp+seqnr-4, x+7, round(results[thenum][7][mapqp],numdecimals))
              #PSNR U
              worksheet.write(y+qp+seqnr-4, x+8, round(results[thenum][8][mapqp],numdecimals))
              #PSNR V
              worksheet.write(y+qp+seqnr-4, x+9, round(results[thenum][9][mapqp],numdecimals))

              #enctime [s]
              worksheet.write(y+qp+seqnr-4, x+10, round(results[thenum][4][mapqp][0],2))
              #dectime
	      worksheet.write(y+qp+seqnr-4, x+11, round(results[thenum][5][mapqp][0],2))
	    else:
              #enctime [s]
              worksheet.write(y+qp+seqnr-4, x+6, round(results[thenum][4][mapqp][0],2))
              #dectime
	      worksheet.write(y+qp+seqnr-4, x+7, round(results[thenum][5][mapqp][0],2))


           seqnr=seqnr+numQps

         seqnr= seqnr+4 #

        # computation of GEO TIME takes several seconds... since I am using an iterative method...
        #encnumgeo1 = self.nth_root(int(encnum1), 4*len(seqs), 8)
        #encnumgeo2 = self.nth_root(int(encnum2), 4*len(seqs), 8)
        #worksheet.write(y+seqnr+1, x+8, "GEO ENC TIME enc2/enc1")
        #worksheet.write(y+seqnr+2, x+8, round(100.0*(float(encnumgeo2)/float(encnumgeo1)),2))


    def plotJCTVCPage(self, seqOK, seqs, results, results2, simid, simid2, BD, BDU, BDV, setavgBDY, setavgBDU, setavgBDV, setavgtime, type, numdecimals): #, lc):
        worksheet = self.workbook.add_sheet("JCTVC")

        # results[seq][0..5] contain bitrate1, psnrY1, psnrU1, psnrV1, enctime1, dectime

        #mapping from our sequence order to JCTVC order
        domap=1
        if domap:
          if type=='i':
            themap = [13, 14, 16, 15, 8, 9, 10, 12, 11, 7, 4, 5, 6, 2, 0, 3, 1, 17, 18, 19] #I
          elif type=='l':
            themap = [8, 9, 10, 12, 11, 7, 4, 5, 6, 2, 0, 3, 1, 13, 14, 15] #lowdel
          elif type=='r':
            themap = [13, 14, 16, 15, 8, 9, 10, 12, 11, 7, 4, 5, 6, 2, 0, 3, 1] #random access
          else:
            themap = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19] #no map

        else:
          print "warning no mapping of sequence number is used\n"
          domap = 1
          themap = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19] #no map
        listlen = len(themap)-1
        x = 2
        y = 5
        worksheet.write(y-4, x-2, "Sequence")

        # BD RATE
        # plot all sequences
        x = 2
        y = 5
        worksheet.write(y-4, x+2, "BD Y")
        worksheet.write(y-4, x+3, "BD low Y")
        worksheet.write(y-4, x+4, "BD high Y")
        worksheet.write(y-4, x+5, "BD U")
        worksheet.write(y-4, x+6, "BD U low")
        worksheet.write(y-4, x+7, "BD U high")
        worksheet.write(y-4, x+8, "BD V")
        worksheet.write(y-4, x+9, "BD V low")
        worksheet.write(y-4, x+10, "BD V high")
        seqnr = 0
        for seq in range(len(seqs)):
         if(seqOK[seq]):
            if(seq>listlen):
              thenum = seq;
            else:
              thenum = themap[seq]


            if(((type=='i8') | (type=='r8'))&(seqnr==8)):
              seqnr=seqnr+numQps+numQps

            if(((type=='r8'))&(seqnr==68)):
              seqnr=seqnr+numQps+numQps+numQps

            worksheet.write(y+seqnr, x-2, seqs[thenum].name)
            numQps = len(results[seq][0])
            worksheet.write(y+seqnr, x+2, round(BD[thenum][0],numdecimals))
            worksheet.write(y+seqnr+1, x+3, round(BD[thenum][2],numdecimals))
            worksheet.write(y+seqnr+1, x+4, round(BD[thenum][1],numdecimals))
            worksheet.write(y+seqnr, x+5, round(BDU[thenum][0],numdecimals))
            worksheet.write(y+seqnr+1, x+6, round(BDU[thenum][2],numdecimals))
            worksheet.write(y+seqnr+1, x+7, round(BDU[thenum][1],numdecimals))
            worksheet.write(y+seqnr, x+8, round(BDV[thenum][0],numdecimals))
            worksheet.write(y+seqnr+1, x+9, round(BDV[thenum][2],numdecimals))
            worksheet.write(y+seqnr+1, x+10, round(BDV[thenum][1],numdecimals))

            seqnr=seqnr+numQps

        worksheet.write(y+seqnr+2, x, "Average")
        worksheet.write(y+seqnr+1, x+2, "BD Y")
        worksheet.write(y+seqnr+1, x+3, "BD low Y")
        worksheet.write(y+seqnr+1, x+4, "BD high Y")
        worksheet.write(y+seqnr+1, x+5, "BD U")
        worksheet.write(y+seqnr+1, x+6, "BD U low")
        worksheet.write(y+seqnr+1, x+7, "BD U high")
        worksheet.write(y+seqnr+1, x+8, "BD V")
        worksheet.write(y+seqnr+1, x+9, "BD V low")
        worksheet.write(y+seqnr+1, x+10, "BD V high")
        worksheet.write(y+seqnr+2, x+2, setavgBDY[0])
        worksheet.write(y+seqnr+3, x+3, setavgBDY[2])
        worksheet.write(y+seqnr+3, x+4, setavgBDY[1])
        worksheet.write(y+seqnr+2, x+5, setavgBDU[0])
        worksheet.write(y+seqnr+3, x+6, setavgBDU[2])
        worksheet.write(y+seqnr+3, x+7, setavgBDU[1])
        worksheet.write(y+seqnr+2, x+8, setavgBDV[0])
        worksheet.write(y+seqnr+3, x+9, setavgBDV[2])
        worksheet.write(y+seqnr+3, x+10, setavgBDV[1])

        #average time (NOTE JCTVC compute GEOMETRICAL AVERAGE and then enc2/enc1)
        # we use (enc2-enc1)/enc1
        worksheet.write(y+seqnr+1, x+12, "Enc usr [%]")
        worksheet.write(y+seqnr+1, x+13, "Dec usr [%]")
        worksheet.write(y+seqnr+2, x+12, setavgtime[0][0])
        worksheet.write(y+seqnr+2, x+13, setavgtime[1][0])

        #Anchor
        x = 2+10
        y = 5
        # plot all sequences
        worksheet.write(y-4, x+1, "QP")
        worksheet.write(y-5, x+2, "Anchor")
        worksheet.write(y-4, x+2, "bitrate")
        worksheet.write(y-4, x+3, "PSNR Y")
        worksheet.write(y-4, x+4, "PSNR U")
        worksheet.write(y-4, x+5, "PSNR V")
        worksheet.write(y-4, x+6, "enctime")
        worksheet.write(y-4, x+8, "enctime")
        worksheet.write(y-4, x+7, "dectime")
        worksheet.write(y-3, x+2, "[kbps]")
        worksheet.write(y-3, x+3, "[dB]")
        worksheet.write(y-3, x+4, "[dB]")
        worksheet.write(y-3, x+5, "[dB]")
        worksheet.write(y-3, x+6, "[s]")
        worksheet.write(y-3, x+8, "[h]")
        worksheet.write(y-3, x+7, "[s]")
        seqnr = 0
        theqp = [37, 32, 27, 22]

        encnum1=1.0
        for seq in range(len(seqs)):
         if(seqOK[seq]):
           if(seq>listlen):
              thenum = seq;
           else:
              thenum = themap[seq]
           numQps = len(results[seq][0])
           #worksheet.write(y+seqnr, x-2, seqs[thenum].name)
           numQps = len(results[seq][0])

           if(((type=='i8') | (type=='r8'))&(seqnr==8)):
             seqnr=seqnr+numQps+numQps
           if(((type=='r8'))&(seqnr==68)):
             seqnr=seqnr+numQps+numQps+numQps

           for qp in range(numQps):
            if(domap & (numQps==4)):
            #if domap:
              mapqp = 3-qp
            else:
              mapqp = qp
            #worksheet.write(y+qp+seqnr, x+1, theqp[mapqp])
            thelen = len(simid[thenum][mapqp])
            thetxt = simid[thenum][mapqp]
            val = "%s%s" % (thetxt[thelen-5],thetxt[thelen-4])
            if(val.isdigit()==0):
             val = "%s" % (thetxt[thelen-4])

            worksheet.write(y+qp+seqnr, x+1, int(val))
            #bitrate
            worksheet.write(y+qp+seqnr, x+2, round(results[thenum][0][mapqp],numdecimals))
            #PSNR Y
            worksheet.write(y+qp+seqnr, x+3, round(results[thenum][1][mapqp],numdecimals))
            #PSNR U
            worksheet.write(y+qp+seqnr, x+4, round(results[thenum][2][mapqp],numdecimals))
            #PSNR V
            worksheet.write(y+qp+seqnr, x+5, round(results[thenum][3][mapqp],numdecimals))
            #bitrate
            #worksheet.write(y+qp+seqnr, x+5, round(results[thenum][0][mapqp],numdecimals))
            #enctime [s]
            worksheet.write(y+qp+seqnr, x+6, round(results[thenum][4][mapqp][0],2))
            #enctime [h]
            worksheet.write(y+qp+seqnr, x+8, round(results[thenum][4][mapqp][0]/3600.0,2))
            encnum1 = encnum1*(results[thenum][4][mapqp][0]/3600.0)
            #dectime
	    worksheet.write(y+qp+seqnr, x+7, round(results[thenum][5][mapqp][0],2))
           seqnr=seqnr+numQps

        #Test
        # plot all sequences
        x = 10+10
        y = 5
        #worksheet.write(y-4, x-2, "Sequence")
        #worksheet.write(y-4, x+1, "QP")
        worksheet.write(y-5, x+2, "Test")
        worksheet.write(y-4, x+2, "bitrate")
        worksheet.write(y-4, x+3, "PSNR Y")
        worksheet.write(y-4, x+4, "PSNR U")
        worksheet.write(y-4, x+5, "PSNR V")
        worksheet.write(y-4, x+6, "enctime")
        worksheet.write(y-4, x+8, "enctime")
        worksheet.write(y-4, x+7, "dectime")
        worksheet.write(y-3, x+2, "[kbps]")
        worksheet.write(y-3, x+3, "[dB]")
        worksheet.write(y-3, x+4, "[dB]")
        worksheet.write(y-3, x+5, "[dB]")
        worksheet.write(y-3, x+6, "[s]")
        worksheet.write(y-3, x+8, "[h]")
        worksheet.write(y-3, x+7, "[s]")
        seqnr = 0

        encnum2 = 1.0
        for seq in range(len(seqs)):
         if(seqOK[seq]):
           if(seq>listlen):
              thenum = seq;
           else:
              thenum = themap[seq]
           numQps = len(results[seq][0])
           #worksheet.write(y+seqnr, x-2, seqs[thenum].name)
           numQps = len(results[seq][0])

           if(((type=='i8') | (type=='r8'))&(seqnr==8)):
             seqnr=seqnr+numQps+numQps

           if(((type=='r8'))&(seqnr==68)):
              seqnr=seqnr+numQps+numQps+numQps

           for qp in range(numQps):
            if(domap & (numQps==4)):
            #if domap:
              mapqp = 3-qp
            else:
              mapqp = qp
            #worksheet.write(y+qp+seqnr, x+1, theqp[mapqp])
            thelen = len(simid2[thenum][mapqp])
            thetxt = simid2[thenum][mapqp]
            val = "%s%s" % (thetxt[thelen-5],thetxt[thelen-4])
            if(val.isdigit()==0):
             val = "%s" % (thetxt[thelen-4])

            worksheet.write(y+qp+seqnr, x+1, int(val))
            #bitrate '0.0001'
            worksheet.write(y+qp+seqnr, x+2, round(results2[thenum][0][mapqp],numdecimals))
            #PSNR Y
            worksheet.write(y+qp+seqnr, x+3, round(results2[thenum][1][mapqp],numdecimals))
            #PSNR U
            worksheet.write(y+qp+seqnr, x+4, round(results2[thenum][2][mapqp],numdecimals))
            #PSNR V
            worksheet.write(y+qp+seqnr, x+5, round(results2[thenum][3][mapqp],numdecimals))
            #bitrate
            #worksheet.write(y+qp+seqnr, x+5, round(results2[thenum][0][mapqp],numdecimals))
            #enctime [s]
            worksheet.write(y+qp+seqnr, x+6, round(results2[thenum][4][mapqp][0],2))
            #enctime [h]
            worksheet.write(y+qp+seqnr, x+8, round(results2[thenum][4][mapqp][0]/3600.0,2))
            encnum2 = encnum2*(results2[thenum][4][mapqp][0]/3600.0)
            #dectime
	    worksheet.write(y+qp+seqnr, x+7, round(results2[thenum][5][mapqp][0],2))
           seqnr=seqnr+numQps

        # computation of GEO TIME takes several seconds... since I am using an iterative method...
        #encnumgeo1 = self.nth_root(int(encnum1), 4*len(seqs), 8)
        #encnumgeo2 = self.nth_root(int(encnum2), 4*len(seqs), 8)
        #worksheet.write(y+seqnr+1, x+8, "GEO ENC TIME enc2/enc1")
        #worksheet.write(y+seqnr+2, x+8, round(100.0*(float(encnumgeo2)/float(encnumgeo1)),2))


    def plotJCTVCPagedQP(self, seqOK, seqs, results, results2, simid, simid2, BD, BDU, BDV, setavgBDY, setavgBDU, setavgBDV, setavgtime, type, numdecimals):
        worksheet = self.workbook.add_sheet("JCTVC")

        # results[seq][0..6] contain bitrate1, psnrY1, psnrU1, psnrV1, enctime1, dectime, dQPbitrate

        #mapping from our sequence order to JCTVC order
        domap=1
        if domap:
          if type=='i':
            themap = [13, 14, 19, 18, 8, 9, 10, 12, 11, 7, 4, 5, 6, 2, 0, 3, 1, 15, 16, 17] #I
          if type=='l':
            themap = [8, 9, 10, 12, 11, 7, 4, 5, 6, 2, 0, 3, 1, 13, 14, 15] #lowdel
          if type=='r':
            themap = [13, 14, 16, 15, 8, 9, 10, 12, 11, 7, 4, 5, 6, 2, 0, 3, 1] #random access
        else:
          print "warning no mapping of sequence number is used\n"
          domap = 1
          themap = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19] #no map
        listlen = len(themap)-1
        x = 2
        y = 5
        worksheet.write(y-4, x-2, "Sequence")

        # BD RATE
        # plot all sequences
        x = 2
        y = 5
        worksheet.write(y-4, x+2, "BD Y")
        worksheet.write(y-4, x+3, "BD low Y")
        worksheet.write(y-4, x+4, "BD high Y")
        worksheet.write(y-4, x+5, "BD U")
        worksheet.write(y-4, x+6, "BD U low")
        worksheet.write(y-4, x+7, "BD U high")
        worksheet.write(y-4, x+8, "BD V")
        worksheet.write(y-4, x+9, "BD V low")
        worksheet.write(y-4, x+10, "BD V high")
        seqnr = 0
        for seq in range(len(seqs)):
         if(seqOK[seq]):
            if(seq>listlen):
              thenum = seq;
            else:
              thenum = themap[seq]
            worksheet.write(y+seqnr, x-2, seqs[thenum].name)
            numQps = len(results[seq][0])
            worksheet.write(y+seqnr, x+2, round(BD[thenum][0],numdecimals))
            worksheet.write(y+seqnr+1, x+3, round(BD[thenum][2],numdecimals))
            worksheet.write(y+seqnr+1, x+4, round(BD[thenum][1],numdecimals))
            worksheet.write(y+seqnr, x+5, round(BDU[thenum][0],numdecimals))
            worksheet.write(y+seqnr+1, x+6, round(BDU[thenum][2],numdecimals))
            worksheet.write(y+seqnr+1, x+7, round(BDU[thenum][1],numdecimals))
            worksheet.write(y+seqnr, x+8, round(BDV[thenum][0],numdecimals))
            worksheet.write(y+seqnr+1, x+9, round(BDV[thenum][2],numdecimals))
            worksheet.write(y+seqnr+1, x+10, round(BDV[thenum][1],numdecimals))

            seqnr=seqnr+numQps

        worksheet.write(y+seqnr+2, x, "Average")
        worksheet.write(y+seqnr+1, x+2, "BD Y")
        worksheet.write(y+seqnr+1, x+3, "BD low Y")
        worksheet.write(y+seqnr+1, x+4, "BD high Y")
        worksheet.write(y+seqnr+1, x+5, "BD U")
        worksheet.write(y+seqnr+1, x+6, "BD U low")
        worksheet.write(y+seqnr+1, x+7, "BD U high")
        worksheet.write(y+seqnr+1, x+8, "BD V")
        worksheet.write(y+seqnr+1, x+9, "BD V low")
        worksheet.write(y+seqnr+1, x+10, "BD V high")
        worksheet.write(y+seqnr+2, x+2, setavgBDY[0])
        worksheet.write(y+seqnr+3, x+3, setavgBDY[2])
        worksheet.write(y+seqnr+3, x+4, setavgBDY[1])
        worksheet.write(y+seqnr+2, x+5, setavgBDU[0])
        worksheet.write(y+seqnr+3, x+6, setavgBDU[2])
        worksheet.write(y+seqnr+3, x+7, setavgBDU[1])
        worksheet.write(y+seqnr+2, x+8, setavgBDV[0])
        worksheet.write(y+seqnr+3, x+9, setavgBDV[2])
        worksheet.write(y+seqnr+3, x+10, setavgBDV[1])

        #average time (NOTE JCTVC compute GEOMETRICAL AVERAGE and then enc2/enc1)
        # we use (enc2-enc1)/enc1
        worksheet.write(y+seqnr+1, x+12, "Enc usr [%]")
        worksheet.write(y+seqnr+1, x+13, "Dec usr [%]")
        worksheet.write(y+seqnr+2, x+12, setavgtime[0][0])
        worksheet.write(y+seqnr+2, x+13, setavgtime[1][0])

        #Anchor
        x = 2+10
        y = 5
        # plot all sequences
        worksheet.write(y-4, x+1, "QP")
        worksheet.write(y-5, x+2, "Anchor")
        worksheet.write(y-4, x+2, "bitrate")
        worksheet.write(y-4, x+3, "PSNR Y")
        worksheet.write(y-4, x+4, "PSNR U")
        worksheet.write(y-4, x+5, "PSNR V")
        worksheet.write(y-4, x+6, "dQPbitrate")
        worksheet.write(y-4, x+7, "enctime")
        worksheet.write(y-4, x+10, "enctime")
        worksheet.write(y-4, x+8, "dectime")
        worksheet.write(y-4, x+9, "dQP ratio")

        worksheet.write(y-3, x+2, "[kbps]")
        worksheet.write(y-3, x+3, "[dB]")
        worksheet.write(y-3, x+4, "[dB]")
        worksheet.write(y-3, x+5, "[dB]")
        worksheet.write(y-3, x+6, "[kbps]")
        worksheet.write(y-3, x+7, "[s]")
        worksheet.write(y-3, x+10, "[h]")
        worksheet.write(y-3, x+8, "[s]")
        worksheet.write(y-3, x+9, "[%]")
        seqnr = 0
        theqp = [37, 32, 27, 22]

        encnum1=1.0
        for seq in range(len(seqs)):
         if(seqOK[seq]):
           if(seq>listlen):
              thenum = seq;
           else:
              thenum = themap[seq]
           numQps = len(results[seq][0])
           #worksheet.write(y+seqnr, x-2, seqs[thenum].name)
           numQps = len(results[seq][0])
           for qp in range(numQps):
            if domap:
              mapqp = 3-qp
            else:
              mapqp = qp
            #worksheet.write(y+qp+seqnr, x+1, theqp[mapqp])
            thelen = len(simid[thenum][mapqp])
            thetxt = simid[thenum][mapqp]
            val = "%s%s" % (thetxt[thelen-5],thetxt[thelen-4])
            worksheet.write(y+qp+seqnr, x+1, int(val))
            #bitrate
            worksheet.write(y+qp+seqnr, x+2, round(results[thenum][0][mapqp],numdecimals))
            #PSNR Y
            worksheet.write(y+qp+seqnr, x+3, round(results[thenum][1][mapqp],numdecimals))
            #PSNR U
            worksheet.write(y+qp+seqnr, x+4, round(results[thenum][2][mapqp],numdecimals))
            #PSNR V
            worksheet.write(y+qp+seqnr, x+5, round(results[thenum][3][mapqp],numdecimals))
            #bitrate
            #worksheet.write(y+qp+seqnr, x+5, round(results[thenum][0][mapqp],numdecimals))
            worksheet.write(y+qp+seqnr, x+6, round(results[thenum][6][mapqp],numdecimals))
            #enctime [s]
            worksheet.write(y+qp+seqnr, x+7, round(results[thenum][4][mapqp][0],2))
            #enctime [h]
            worksheet.write(y+qp+seqnr, x+10, round(results[thenum][4][mapqp][0]/3600.0,2))
            encnum1 = encnum1*(results[thenum][4][mapqp][0]/3600.0)
            #dectime
	    worksheet.write(y+qp+seqnr, x+8, round(results[thenum][5][mapqp][0],2))
            #dQP ratio
            if results[thenum][0][mapqp] != 0:
              ratio = results[thenum][6][mapqp]/results[thenum][0][mapqp]
            else:
              ratio = 0.000;
	    worksheet.write(y+qp+seqnr, x+9, round(ratio,6))
           seqnr=seqnr+numQps

        #Test
        # plot all sequences
        x = 11+10
        y = 5
        #worksheet.write(y-4, x-2, "Sequence")
        #worksheet.write(y-4, x+1, "QP")
        worksheet.write(y-5, x+2, "Test")
        worksheet.write(y-4, x+2, "bitrate")
        worksheet.write(y-4, x+3, "PSNR Y")
        worksheet.write(y-4, x+4, "PSNR U")
        worksheet.write(y-4, x+5, "PSNR V")
        worksheet.write(y-4, x+6, "dQPbitrate")
        worksheet.write(y-4, x+7, "enctime")
        worksheet.write(y-4, x+10, "enctime")
        worksheet.write(y-4, x+8, "dectime")
        worksheet.write(y-4, x+9, "dQP ratio")
        worksheet.write(y-3, x+2, "[kbps]")
        worksheet.write(y-3, x+3, "[dB]")
        worksheet.write(y-3, x+4, "[dB]")
        worksheet.write(y-3, x+5, "[dB]")
        worksheet.write(y-3, x+6, "[kbps]")
        worksheet.write(y-3, x+7, "[s]")
        worksheet.write(y-3, x+10, "[h]")
        worksheet.write(y-3, x+8, "[s]")
        worksheet.write(y-3, x+9, "[%]")
        seqnr = 0

        encnum2 = 1.0
        for seq in range(len(seqs)):
         if(seqOK[seq]):
           if(seq>listlen):
              thenum = seq;
           else:
              thenum = themap[seq]
           numQps = len(results[seq][0])
           #worksheet.write(y+seqnr, x-2, seqs[thenum].name)
           numQps = len(results[seq][0])
           for qp in range(numQps):
            if domap:
              mapqp = 3-qp
            else:
              mapqp = qp
            #worksheet.write(y+qp+seqnr, x+1, theqp[mapqp])
            thelen = len(simid2[thenum][mapqp])
            thetxt = simid2[thenum][mapqp]
            val = "%s%s" % (thetxt[thelen-5],thetxt[thelen-4])
            worksheet.write(y+qp+seqnr, x+1, int(val))
            #bitrate '0.0001'
            worksheet.write(y+qp+seqnr, x+2, round(results2[thenum][0][mapqp],numdecimals))
            #PSNR Y
            worksheet.write(y+qp+seqnr, x+3, round(results2[thenum][1][mapqp],numdecimals))
            #PSNR U
            worksheet.write(y+qp+seqnr, x+4, round(results2[thenum][2][mapqp],numdecimals))
            #PSNR V
            worksheet.write(y+qp+seqnr, x+5, round(results2[thenum][3][mapqp],numdecimals))
            #bitrate
            #worksheet.write(y+qp+seqnr, x+5, round(results2[thenum][0][mapqp],numdecimals))
            worksheet.write(y+qp+seqnr, x+6, round(results2[thenum][6][mapqp],numdecimals))
            #enctime [s]
            worksheet.write(y+qp+seqnr, x+7, round(results2[thenum][4][mapqp][0],2))
            #enctime [h]
            worksheet.write(y+qp+seqnr, x+10, round(results2[thenum][4][mapqp][0]/3600.0,2))
            encnum2 = encnum2*(results2[thenum][4][mapqp][0]/3600.0)
            #dectime
	    worksheet.write(y+qp+seqnr, x+8, round(results2[thenum][5][mapqp][0],2))
            #dQP ratio
            if results2[thenum][0][mapqp] != 0:
              ratio = results2[thenum][6][mapqp]/results2[thenum][0][mapqp]
            else:
              ratio = 0.000;
	    worksheet.write(y+qp+seqnr, x+9, round(ratio,6))

           seqnr=seqnr+numQps

        # computation of GEO TIME takes several seconds... since I am using an iterative method...
        #encnumgeo1 = self.nth_root(int(encnum1), 4*len(seqs), 8)
        #encnumgeo2 = self.nth_root(int(encnum2), 4*len(seqs), 8)
        #worksheet.write(y+seqnr+1, x+8, "GEO ENC TIME enc2/enc1")
        #worksheet.write(y+seqnr+2, x+8, round(100.0*(float(encnumgeo2)/float(encnumgeo1)),2))


    def plotSummary(self, name, seqOK, setavgBDY, setavgBDU, setavgBDV, setavgBDYOverlap, setavgBDUOverlap, setavgBDVOverlap, setavgBDPSNRY, setavgBDPSNRU, setavgBDPSNRV, setavgBDPSNRYOverlap, setavgBDPSNRUOverlap, setavgBDPSNRVOverlap, setavgbitrate, setavgpsnr, seqs, BD, BDU, BDV, BDOverlap, BDUOverlap, BDVOverlap, BDPSNR, BDPSNRU, BDPSNRV, BDPSNROverlap, BDPSNRUOverlap, BDPSNRVOverlap, avgbitrate, avgpsnrY, TimeData, setavgtime):
        worksheet = self.workbook.add_sheet(name)

        # plot all sequences
        x = 2
        y = 5
        worksheet.write(y-4, x, "Sequence")
        worksheet.write(y-4, x+2, "Anchor")
        worksheet.write(y-3, x+2, "avgbitrate")
        worksheet.write(y-3, x+3, "avgPSNR")
        worksheet.write(y-2, x+2, "[kbps]")
        worksheet.write(y-2, x+3, "[dB]")
        worksheet.write(y-4, x+4, "Test")
        worksheet.write(y-3, x+4, "avgbitrate")
        worksheet.write(y-3, x+5, "avgPSNR")
        worksheet.write(y-2, x+4, "[kbps]")
        worksheet.write(y-2, x+5, "[dB]")
        worksheet.write(y-4, x+6, "BD rate Luma [%]")
        worksheet.write(y-1, x+6, "avg")
        worksheet.write(y-1, x+7, "overlap")
        worksheet.write(y-1, x+8, "low")
        worksheet.write(y-1, x+9, "overlap")
        worksheet.write(y-1, x+10, "high")
        worksheet.write(y-1, x+11, "overlap")
        worksheet.write(y-4, x+12, "BD rate U [%]")
        worksheet.write(y-1, x+12, "avg")
        worksheet.write(y-1, x+13, "overlap")
        worksheet.write(y-1, x+14, "low")
        worksheet.write(y-1, x+15, "overlap")
        worksheet.write(y-1, x+16, "high")
        worksheet.write(y-1, x+17, "overlap")
        worksheet.write(y-4, x+18, "BD rate V [%]")
        worksheet.write(y-1, x+18, "avg")
        worksheet.write(y-1, x+19, "overlap")
        worksheet.write(y-1, x+20, "low")
        worksheet.write(y-1, x+21, "overlap")
        worksheet.write(y-1, x+22, "high")
        worksheet.write(y-1, x+23, "overlap")
        worksheet.write(y-4, x+24, "Enc usr [%]")
        worksheet.write(y-4, x+25, "Enc sys [%]")
        worksheet.write(y-4, x+26, "Enc all [%]")
        worksheet.write(y-4, x+27, "Dec usr [%]")
        worksheet.write(y-4, x+28, "Dec sys [%]")
        worksheet.write(y-4, x+29, "Dec all [%]")
        seqnr = 0
        for seq in range(len(seqs)):
         if(seqOK[seq]):
          worksheet.write(y+seqnr, x, seqs[seq].name)
          worksheet.write(y+seqnr, x+2, avgbitrate[seqnr][0])
          worksheet.write(y+seqnr, x+3, avgpsnrY[seqnr][0])
          worksheet.write(y+seqnr, x+4, avgbitrate[seqnr][1])
          worksheet.write(y+seqnr, x+5, avgpsnrY[seqnr][1])
          worksheet.write(y+seqnr, x+6, BD[seqnr][0])
          worksheet.write(y+seqnr, x+7, BDOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+8, BD[seqnr][2])
          worksheet.write(y+seqnr, x+9, BDOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+10, BD[seqnr][1])
          worksheet.write(y+seqnr, x+11, BDOverlap[seqnr][1])
          worksheet.write(y+seqnr, x+12, BDU[seqnr][0])
          worksheet.write(y+seqnr, x+13, BDUOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+14, BDU[seqnr][2])
          worksheet.write(y+seqnr, x+15, BDUOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+16, BDU[seqnr][1])
          worksheet.write(y+seqnr, x+17, BDUOverlap[seqnr][1])
          worksheet.write(y+seqnr, x+18, BDV[seqnr][0])
          worksheet.write(y+seqnr, x+19, BDVOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+20, BDV[seqnr][2])
          worksheet.write(y+seqnr, x+21, BDVOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+22, BDV[seqnr][1])
          worksheet.write(y+seqnr, x+23, BDVOverlap[seqnr][1])
          worksheet.write(y+seqnr, x+24, TimeData[seqnr][0][0])
          worksheet.write(y+seqnr, x+25, TimeData[seqnr][1][0])
          worksheet.write(y+seqnr, x+26, TimeData[seqnr][2][0])
          worksheet.write(y+seqnr, x+27, TimeData[seqnr][0][1])
          worksheet.write(y+seqnr, x+28, TimeData[seqnr][1][1])
          worksheet.write(y+seqnr, x+29, TimeData[seqnr][2][1])
          seqnr = seqnr + 1

        worksheet.write(y+seqnr+2, x, "Average")
        worksheet.write(y+seqnr+2, x+2, setavgbitrate[0])
        worksheet.write(y+seqnr+2, x+3, setavgpsnr[0])
        worksheet.write(y+seqnr+2, x+4, setavgbitrate[1])
        worksheet.write(y+seqnr+2, x+5, setavgpsnr[1])
        worksheet.write(y+seqnr+2, x+6, setavgBDY[0])
        worksheet.write(y+seqnr+2, x+7, setavgBDYOverlap[0])
        worksheet.write(y+seqnr+2, x+8, setavgBDY[2])
        worksheet.write(y+seqnr+2, x+9, setavgBDYOverlap[2])
        worksheet.write(y+seqnr+2, x+10, setavgBDY[1])
        worksheet.write(y+seqnr+2, x+11, setavgBDYOverlap[1])
        worksheet.write(y+seqnr+2, x+12, setavgBDU[0])
        worksheet.write(y+seqnr+2, x+13, setavgBDUOverlap[0])
        worksheet.write(y+seqnr+2, x+14, setavgBDU[2])
        worksheet.write(y+seqnr+2, x+15, setavgBDUOverlap[2])
        worksheet.write(y+seqnr+2, x+16, setavgBDU[1])
        worksheet.write(y+seqnr+2, x+17, setavgBDUOverlap[1])
        worksheet.write(y+seqnr+2, x+18, setavgBDV[0])
        worksheet.write(y+seqnr+2, x+19, setavgBDVOverlap[0])
        worksheet.write(y+seqnr+2, x+20, setavgBDV[2])
        worksheet.write(y+seqnr+2, x+21, setavgBDVOverlap[2])
        worksheet.write(y+seqnr+2, x+22, setavgBDV[1])
        worksheet.write(y+seqnr+2, x+23, setavgBDVOverlap[1])
        worksheet.write(y+seqnr+2, x+24, setavgtime[0][0])
        worksheet.write(y+seqnr+2, x+25, setavgtime[0][1])
        worksheet.write(y+seqnr+2, x+26, setavgtime[0][2])
        worksheet.write(y+seqnr+2, x+27, setavgtime[1][0])
        worksheet.write(y+seqnr+2, x+28, setavgtime[1][1])
        worksheet.write(y+seqnr+2, x+29, setavgtime[1][2])

        #BDPSNR
        # plot all sequences
        x = 2
        y = y+seqnr+2+8
        worksheet.write(y-4, x, "Sequence")
        worksheet.write(y-4, x+2, "Anchor")
        worksheet.write(y-3, x+2, "avgbitrate")
        worksheet.write(y-3, x+3, "avgPSNR")
        worksheet.write(y-2, x+2, "[kbps]")
        worksheet.write(y-2, x+3, "[dB]")
        worksheet.write(y-4, x+4, "Test")
        worksheet.write(y-3, x+4, "avgbitrate")
        worksheet.write(y-3, x+5, "avgPSNR")
        worksheet.write(y-2, x+4, "[kbps]")
        worksheet.write(y-2, x+5, "[dB]")
        worksheet.write(y-4, x+6, "BD PSNR Luma [dB]")
        worksheet.write(y-1, x+6, "avg")
        worksheet.write(y-1, x+7, "overlap")
        worksheet.write(y-1, x+8, "low")
        worksheet.write(y-1, x+9, "overlap")
        worksheet.write(y-1, x+10, "high")
        worksheet.write(y-1, x+11, "overlap")
        worksheet.write(y-4, x+12, "BD PSNR U [dB]")
        worksheet.write(y-1, x+12, "avg")
        worksheet.write(y-1, x+13, "overlap")
        worksheet.write(y-1, x+14, "low")
        worksheet.write(y-1, x+15, "overlap")
        worksheet.write(y-1, x+16, "high")
        worksheet.write(y-1, x+17, "overlap")
        worksheet.write(y-4, x+18, "BD PSNR V [dB]")
        worksheet.write(y-1, x+18, "avg")
        worksheet.write(y-1, x+19, "overlap")
        worksheet.write(y-1, x+20, "low")
        worksheet.write(y-1, x+21, "overlap")
        worksheet.write(y-1, x+22, "high")
        worksheet.write(y-1, x+23, "overlap")
        seqnr = 0
        for seq in range(len(seqs)):
         if(seqOK[seq]):
          worksheet.write(y+seqnr, x, seqs[seq].name)
          worksheet.write(y+seqnr, x+2, avgbitrate[seqnr][0])
          worksheet.write(y+seqnr, x+3, avgpsnrY[seqnr][0])
          worksheet.write(y+seqnr, x+4, avgbitrate[seqnr][1])
          worksheet.write(y+seqnr, x+5, avgpsnrY[seqnr][1])
          worksheet.write(y+seqnr, x+6, BDPSNR[seqnr][0])
          worksheet.write(y+seqnr, x+7, BDPSNROverlap[seqnr][0])
          worksheet.write(y+seqnr, x+8, BDPSNR[seqnr][2])
          worksheet.write(y+seqnr, x+9, BDPSNROverlap[seqnr][2])
          worksheet.write(y+seqnr, x+10, BDPSNR[seqnr][1])
          worksheet.write(y+seqnr, x+11, BDPSNROverlap[seqnr][1])
          worksheet.write(y+seqnr, x+12, BDPSNRU[seqnr][0])
          worksheet.write(y+seqnr, x+13, BDPSNRUOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+14, BDPSNRU[seqnr][2])
          worksheet.write(y+seqnr, x+15, BDPSNRUOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+16, BDPSNRU[seqnr][1])
          worksheet.write(y+seqnr, x+17, BDPSNRUOverlap[seqnr][1])
          worksheet.write(y+seqnr, x+18, BDPSNRV[seqnr][0])
          worksheet.write(y+seqnr, x+19, BDPSNRVOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+20, BDPSNRV[seqnr][2])
          worksheet.write(y+seqnr, x+21, BDPSNRVOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+22, BDPSNRV[seqnr][1])
          worksheet.write(y+seqnr, x+23, BDPSNRVOverlap[seqnr][1])
          seqnr = seqnr + 1

        worksheet.write(y+seqnr+2, x, "Average")
        worksheet.write(y+seqnr+2, x+2, setavgbitrate[0])
        worksheet.write(y+seqnr+2, x+3, setavgpsnr[0])
        worksheet.write(y+seqnr+2, x+4, setavgbitrate[1])
        worksheet.write(y+seqnr+2, x+5, setavgpsnr[1])
        worksheet.write(y+seqnr+2, x+6, setavgBDPSNRY[0])
        worksheet.write(y+seqnr+2, x+7, setavgBDPSNRYOverlap[0])
        worksheet.write(y+seqnr+2, x+8, setavgBDPSNRY[2])
        worksheet.write(y+seqnr+2, x+9, setavgBDPSNRYOverlap[2])
        worksheet.write(y+seqnr+2, x+10, setavgBDPSNRY[1])
        worksheet.write(y+seqnr+2, x+11, setavgBDPSNRYOverlap[1])
        worksheet.write(y+seqnr+2, x+12, setavgBDPSNRU[0])
        worksheet.write(y+seqnr+2, x+13, setavgBDPSNRUOverlap[0])
        worksheet.write(y+seqnr+2, x+14, setavgBDPSNRU[2])
        worksheet.write(y+seqnr+2, x+15, setavgBDPSNRUOverlap[2])
        worksheet.write(y+seqnr+2, x+16, setavgBDPSNRU[1])
        worksheet.write(y+seqnr+2, x+17, setavgBDPSNRUOverlap[1])
        worksheet.write(y+seqnr+2, x+18, setavgBDPSNRV[0])
        worksheet.write(y+seqnr+2, x+19, setavgBDPSNRVOverlap[0])
        worksheet.write(y+seqnr+2, x+20, setavgBDPSNRV[2])
        worksheet.write(y+seqnr+2, x+21, setavgBDPSNRVOverlap[2])
        worksheet.write(y+seqnr+2, x+22, setavgBDPSNRV[1])
        worksheet.write(y+seqnr+2, x+23, setavgBDPSNRVOverlap[1])

    def plotSummaryWithSSIM(self, name, seqOK, setavgBDY, setavgBDU, setavgBDV, setavgBDYOverlap, setavgBDUOverlap, setavgBDVOverlap, setavgBDPSNRY, setavgBDPSNRU, setavgBDPSNRV, setavgBDPSNRYOverlap, setavgBDPSNRUOverlap, setavgBDPSNRVOverlap, setavgbitrate, setavgpsnr, seqs, BD, BDU, BDV, BDOverlap, BDUOverlap, BDVOverlap, BDPSNR, BDPSNRU, BDPSNRV, BDPSNROverlap, BDPSNRUOverlap, BDPSNRVOverlap, avgbitrate, avgpsnrY, TimeData, setavgtime, avgssimY, setavgssim, setavgBDSSIMRateY, setavgBDSSIMRateU, setavgBDSSIMRateV, setavgBDSSIMRateYOverlap, setavgBDSSIMRateUOverlap, setavgBDSSIMRateVOverlap, setavgBDSSIMY, setavgBDSSIMU, setavgBDSSIMV, setavgBDSSIMYOverlap, setavgBDSSIMUOverlap, setavgBDSSIMVOverlap, BDSSIMRate, BDSSIMRateU, BDSSIMRateV, BDSSIMRateOverlap, BDSSIMRateUOverlap, BDSSIMRateVOverlap, BDSSIM, BDSSIMU, BDSSIMV, BDSSIMOverlap, BDSSIMUOverlap, BDSSIMVOverlap):
        worksheet = self.workbook.add_sheet(name)

        # plot all sequences
        x = 2
        y = 5
        worksheet.write(y-4, x, "Sequence")
        worksheet.write(y-4, x+2, "Anchor")
        worksheet.write(y-3, x+2, "avgbitrate")
        worksheet.write(y-3, x+3, "avgPSNR")
        worksheet.write(y-2, x+2, "[kbps]")
        worksheet.write(y-2, x+3, "[dB]")
        worksheet.write(y-4, x+4, "Test")
        worksheet.write(y-3, x+4, "avgbitrate")
        worksheet.write(y-3, x+5, "avgPSNR")
        worksheet.write(y-2, x+4, "[kbps]")
        worksheet.write(y-2, x+5, "[dB]")
        worksheet.write(y-4, x+6, "BD rate Luma [%]")
        worksheet.write(y-1, x+6, "avg")
        worksheet.write(y-1, x+7, "overlap")
        worksheet.write(y-1, x+8, "low")
        worksheet.write(y-1, x+9, "overlap")
        worksheet.write(y-1, x+10, "high")
        worksheet.write(y-1, x+11, "overlap")
        worksheet.write(y-4, x+12, "BD rate U [%]")
        worksheet.write(y-1, x+12, "avg")
        worksheet.write(y-1, x+13, "overlap")
        worksheet.write(y-1, x+14, "low")
        worksheet.write(y-1, x+15, "overlap")
        worksheet.write(y-1, x+16, "high")
        worksheet.write(y-1, x+17, "overlap")
        worksheet.write(y-4, x+18, "BD rate V [%]")
        worksheet.write(y-1, x+18, "avg")
        worksheet.write(y-1, x+19, "overlap")
        worksheet.write(y-1, x+20, "low")
        worksheet.write(y-1, x+21, "overlap")
        worksheet.write(y-1, x+22, "high")
        worksheet.write(y-1, x+23, "overlap")
        worksheet.write(y-4, x+24, "Enc usr [%]")
        worksheet.write(y-4, x+25, "Enc sys [%]")
        worksheet.write(y-4, x+26, "Enc all [%]")
        worksheet.write(y-4, x+27, "Dec usr [%]")
        worksheet.write(y-4, x+28, "Dec sys [%]")
        worksheet.write(y-4, x+29, "Dec all [%]")
        seqnr = 0
        for seq in range(len(seqs)):
         if(seqOK[seq]):
          worksheet.write(y+seqnr, x, seqs[seq].name)
          worksheet.write(y+seqnr, x+2, avgbitrate[seqnr][0])
          worksheet.write(y+seqnr, x+3, avgpsnrY[seqnr][0])
          worksheet.write(y+seqnr, x+4, avgbitrate[seqnr][1])
          worksheet.write(y+seqnr, x+5, avgpsnrY[seqnr][1])
          worksheet.write(y+seqnr, x+6, BD[seqnr][0])
          worksheet.write(y+seqnr, x+7, BDOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+8, BD[seqnr][2])
          worksheet.write(y+seqnr, x+9, BDOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+10, BD[seqnr][1])
          worksheet.write(y+seqnr, x+11, BDOverlap[seqnr][1])
          worksheet.write(y+seqnr, x+12, BDU[seqnr][0])
          worksheet.write(y+seqnr, x+13, BDUOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+14, BDU[seqnr][2])
          worksheet.write(y+seqnr, x+15, BDUOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+16, BDU[seqnr][1])
          worksheet.write(y+seqnr, x+17, BDUOverlap[seqnr][1])
          worksheet.write(y+seqnr, x+18, BDV[seqnr][0])
          worksheet.write(y+seqnr, x+19, BDVOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+20, BDV[seqnr][2])
          worksheet.write(y+seqnr, x+21, BDVOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+22, BDV[seqnr][1])
          worksheet.write(y+seqnr, x+23, BDVOverlap[seqnr][1])
          worksheet.write(y+seqnr, x+24, TimeData[seqnr][0][0])
          worksheet.write(y+seqnr, x+25, TimeData[seqnr][1][0])
          worksheet.write(y+seqnr, x+26, TimeData[seqnr][2][0])
          worksheet.write(y+seqnr, x+27, TimeData[seqnr][0][1])
          worksheet.write(y+seqnr, x+28, TimeData[seqnr][1][1])
          worksheet.write(y+seqnr, x+29, TimeData[seqnr][2][1])
          seqnr = seqnr + 1

        worksheet.write(y+seqnr+2, x, "Average")
        worksheet.write(y+seqnr+2, x+2, setavgbitrate[0])
        worksheet.write(y+seqnr+2, x+3, setavgpsnr[0])
        worksheet.write(y+seqnr+2, x+4, setavgbitrate[1])
        worksheet.write(y+seqnr+2, x+5, setavgpsnr[1])
        worksheet.write(y+seqnr+2, x+6, setavgBDY[0])
        worksheet.write(y+seqnr+2, x+7, setavgBDYOverlap[0])
        worksheet.write(y+seqnr+2, x+8, setavgBDY[2])
        worksheet.write(y+seqnr+2, x+9, setavgBDYOverlap[2])
        worksheet.write(y+seqnr+2, x+10, setavgBDY[1])
        worksheet.write(y+seqnr+2, x+11, setavgBDYOverlap[1])
        worksheet.write(y+seqnr+2, x+12, setavgBDU[0])
        worksheet.write(y+seqnr+2, x+13, setavgBDUOverlap[0])
        worksheet.write(y+seqnr+2, x+14, setavgBDU[2])
        worksheet.write(y+seqnr+2, x+15, setavgBDUOverlap[2])
        worksheet.write(y+seqnr+2, x+16, setavgBDU[1])
        worksheet.write(y+seqnr+2, x+17, setavgBDUOverlap[1])
        worksheet.write(y+seqnr+2, x+18, setavgBDV[0])
        worksheet.write(y+seqnr+2, x+19, setavgBDVOverlap[0])
        worksheet.write(y+seqnr+2, x+20, setavgBDV[2])
        worksheet.write(y+seqnr+2, x+21, setavgBDVOverlap[2])
        worksheet.write(y+seqnr+2, x+22, setavgBDV[1])
        worksheet.write(y+seqnr+2, x+23, setavgBDVOverlap[1])
        worksheet.write(y+seqnr+2, x+24, setavgtime[0][0])
        worksheet.write(y+seqnr+2, x+25, setavgtime[0][1])
        worksheet.write(y+seqnr+2, x+26, setavgtime[0][2])
        worksheet.write(y+seqnr+2, x+27, setavgtime[1][0])
        worksheet.write(y+seqnr+2, x+28, setavgtime[1][1])
        worksheet.write(y+seqnr+2, x+29, setavgtime[1][2])

        #BDPSNR
        # plot all sequences
        x = 2
        y = y+seqnr+2+8
        worksheet.write(y-4, x, "Sequence")
        worksheet.write(y-4, x+2, "Anchor")
        worksheet.write(y-3, x+2, "avgbitrate")
        worksheet.write(y-3, x+3, "avgPSNR")
        worksheet.write(y-2, x+2, "[kbps]")
        worksheet.write(y-2, x+3, "[dB]")
        worksheet.write(y-4, x+4, "Test")
        worksheet.write(y-3, x+4, "avgbitrate")
        worksheet.write(y-3, x+5, "avgPSNR")
        worksheet.write(y-2, x+4, "[kbps]")
        worksheet.write(y-2, x+5, "[dB]")
        worksheet.write(y-4, x+6, "BD PSNR Luma [dB]")
        worksheet.write(y-1, x+6, "avg")
        worksheet.write(y-1, x+7, "overlap")
        worksheet.write(y-1, x+8, "low")
        worksheet.write(y-1, x+9, "overlap")
        worksheet.write(y-1, x+10, "high")
        worksheet.write(y-1, x+11, "overlap")
        worksheet.write(y-4, x+12, "BD PSNR U [dB]")
        worksheet.write(y-1, x+12, "avg")
        worksheet.write(y-1, x+13, "overlap")
        worksheet.write(y-1, x+14, "low")
        worksheet.write(y-1, x+15, "overlap")
        worksheet.write(y-1, x+16, "high")
        worksheet.write(y-1, x+17, "overlap")
        worksheet.write(y-4, x+18, "BD PSNR V [dB]")
        worksheet.write(y-1, x+18, "avg")
        worksheet.write(y-1, x+19, "overlap")
        worksheet.write(y-1, x+20, "low")
        worksheet.write(y-1, x+21, "overlap")
        worksheet.write(y-1, x+22, "high")
        worksheet.write(y-1, x+23, "overlap")
        seqnr = 0
        for seq in range(len(seqs)):
         if(seqOK[seq]):
          worksheet.write(y+seqnr, x, seqs[seq].name)
          worksheet.write(y+seqnr, x+2, avgbitrate[seqnr][0])
          worksheet.write(y+seqnr, x+3, avgpsnrY[seqnr][0])
          worksheet.write(y+seqnr, x+4, avgbitrate[seqnr][1])
          worksheet.write(y+seqnr, x+5, avgpsnrY[seqnr][1])
          worksheet.write(y+seqnr, x+6, BDPSNR[seqnr][0])
          worksheet.write(y+seqnr, x+7, BDPSNROverlap[seqnr][0])
          worksheet.write(y+seqnr, x+8, BDPSNR[seqnr][2])
          worksheet.write(y+seqnr, x+9, BDPSNROverlap[seqnr][2])
          worksheet.write(y+seqnr, x+10, BDPSNR[seqnr][1])
          worksheet.write(y+seqnr, x+11, BDPSNROverlap[seqnr][1])
          worksheet.write(y+seqnr, x+12, BDPSNRU[seqnr][0])
          worksheet.write(y+seqnr, x+13, BDPSNRUOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+14, BDPSNRU[seqnr][2])
          worksheet.write(y+seqnr, x+15, BDPSNRUOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+16, BDPSNRU[seqnr][1])
          worksheet.write(y+seqnr, x+17, BDPSNRUOverlap[seqnr][1])
          worksheet.write(y+seqnr, x+18, BDPSNRV[seqnr][0])
          worksheet.write(y+seqnr, x+19, BDPSNRVOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+20, BDPSNRV[seqnr][2])
          worksheet.write(y+seqnr, x+21, BDPSNRVOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+22, BDPSNRV[seqnr][1])
          worksheet.write(y+seqnr, x+23, BDPSNRVOverlap[seqnr][1])
          seqnr = seqnr + 1

        worksheet.write(y+seqnr+2, x, "Average")
        worksheet.write(y+seqnr+2, x+2, setavgbitrate[0])
        worksheet.write(y+seqnr+2, x+3, setavgpsnr[0])
        worksheet.write(y+seqnr+2, x+4, setavgbitrate[1])
        worksheet.write(y+seqnr+2, x+5, setavgpsnr[1])
        worksheet.write(y+seqnr+2, x+6, setavgBDPSNRY[0])
        worksheet.write(y+seqnr+2, x+7, setavgBDPSNRYOverlap[0])
        worksheet.write(y+seqnr+2, x+8, setavgBDPSNRY[2])
        worksheet.write(y+seqnr+2, x+9, setavgBDPSNRYOverlap[2])
        worksheet.write(y+seqnr+2, x+10, setavgBDPSNRY[1])
        worksheet.write(y+seqnr+2, x+11, setavgBDPSNRYOverlap[1])
        worksheet.write(y+seqnr+2, x+12, setavgBDPSNRU[0])
        worksheet.write(y+seqnr+2, x+13, setavgBDPSNRUOverlap[0])
        worksheet.write(y+seqnr+2, x+14, setavgBDPSNRU[2])
        worksheet.write(y+seqnr+2, x+15, setavgBDPSNRUOverlap[2])
        worksheet.write(y+seqnr+2, x+16, setavgBDPSNRU[1])
        worksheet.write(y+seqnr+2, x+17, setavgBDPSNRUOverlap[1])
        worksheet.write(y+seqnr+2, x+18, setavgBDPSNRV[0])
        worksheet.write(y+seqnr+2, x+19, setavgBDPSNRVOverlap[0])
        worksheet.write(y+seqnr+2, x+20, setavgBDPSNRV[2])
        worksheet.write(y+seqnr+2, x+21, setavgBDPSNRVOverlap[2])
        worksheet.write(y+seqnr+2, x+22, setavgBDPSNRV[1])
        worksheet.write(y+seqnr+2, x+23, setavgBDPSNRVOverlap[1])

        #SSIM BD Rate
        x = 2
        y = y+seqnr+2+8
        worksheet.write(y-4, x, "Sequence")
        worksheet.write(y-4, x+2, "Anchor")
        worksheet.write(y-3, x+2, "avgbitrate")
        worksheet.write(y-3, x+3, "avgSSIM")
        worksheet.write(y-2, x+2, "[kbps]")
        worksheet.write(y-4, x+4, "Test")
        worksheet.write(y-3, x+4, "avgbitrate")
        worksheet.write(y-3, x+5, "avgSSIM")
        worksheet.write(y-2, x+4, "[kbps]")
        worksheet.write(y-4, x+6, "BD SSIM rate Luma [%]")
        worksheet.write(y-1, x+6, "avg")
        worksheet.write(y-1, x+7, "overlap")
        worksheet.write(y-1, x+8, "low")
        worksheet.write(y-1, x+9, "overlap")
        worksheet.write(y-1, x+10, "high")
        worksheet.write(y-1, x+11, "overlap")
        worksheet.write(y-4, x+12, "BD SSIM  rate U [%]")
        worksheet.write(y-1, x+12, "avg")
        worksheet.write(y-1, x+13, "overlap")
        worksheet.write(y-1, x+14, "low")
        worksheet.write(y-1, x+15, "overlap")
        worksheet.write(y-1, x+16, "high")
        worksheet.write(y-1, x+17, "overlap")
        worksheet.write(y-4, x+18, "BD SSIM  rate V [%]")
        worksheet.write(y-1, x+18, "avg")
        worksheet.write(y-1, x+19, "overlap")
        worksheet.write(y-1, x+20, "low")
        worksheet.write(y-1, x+21, "overlap")
        worksheet.write(y-1, x+22, "high")
        worksheet.write(y-1, x+23, "overlap")

        seqnr = 0
        for seq in range(len(seqs)):
         if(seqOK[seq]):
          worksheet.write(y+seqnr, x, seqs[seq].name)
          worksheet.write(y+seqnr, x+2, avgbitrate[seqnr][0])
          worksheet.write(y+seqnr, x+3, avgssimY[seqnr][0])
          worksheet.write(y+seqnr, x+4, avgbitrate[seqnr][1])
          worksheet.write(y+seqnr, x+5, avgssimY[seqnr][1])
          worksheet.write(y+seqnr, x+6, BDSSIMRate[seqnr][0])
          worksheet.write(y+seqnr, x+7, BDSSIMRateOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+8, BDSSIMRate[seqnr][2])
          worksheet.write(y+seqnr, x+9, BDSSIMRateOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+10, BDSSIMRate[seqnr][1])
          worksheet.write(y+seqnr, x+11, BDSSIMRateOverlap[seqnr][1])
          worksheet.write(y+seqnr, x+12, BDSSIMRateU[seqnr][0])
          worksheet.write(y+seqnr, x+13, BDSSIMRateUOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+14, BDSSIMRateU[seqnr][2])
          worksheet.write(y+seqnr, x+15, BDSSIMRateUOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+16, BDSSIMRateU[seqnr][1])
          worksheet.write(y+seqnr, x+17, BDSSIMRateUOverlap[seqnr][1])
          worksheet.write(y+seqnr, x+18, BDSSIMRateV[seqnr][0])
          worksheet.write(y+seqnr, x+19, BDSSIMRateVOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+20, BDSSIMRateV[seqnr][2])
          worksheet.write(y+seqnr, x+21, BDSSIMRateVOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+22, BDSSIMRateV[seqnr][1])
          worksheet.write(y+seqnr, x+23, BDSSIMRateVOverlap[seqnr][1])
          seqnr = seqnr + 1

        worksheet.write(y+seqnr+2, x, "Average")
        worksheet.write(y+seqnr+2, x+2, setavgbitrate[0])
        worksheet.write(y+seqnr+2, x+3, setavgssim[0])
        worksheet.write(y+seqnr+2, x+4, setavgbitrate[1])
        worksheet.write(y+seqnr+2, x+5, setavgssim[1])
        worksheet.write(y+seqnr+2, x+6, setavgBDSSIMRateY[0])
        worksheet.write(y+seqnr+2, x+7, setavgBDSSIMRateYOverlap[0])
        worksheet.write(y+seqnr+2, x+8, setavgBDSSIMRateY[2])
        worksheet.write(y+seqnr+2, x+9, setavgBDSSIMRateYOverlap[2])
        worksheet.write(y+seqnr+2, x+10, setavgBDSSIMRateY[1])
        worksheet.write(y+seqnr+2, x+11, setavgBDSSIMRateYOverlap[1])
        worksheet.write(y+seqnr+2, x+12, setavgBDSSIMRateU[0])
        worksheet.write(y+seqnr+2, x+13, setavgBDSSIMRateUOverlap[0])
        worksheet.write(y+seqnr+2, x+14, setavgBDSSIMRateU[2])
        worksheet.write(y+seqnr+2, x+15, setavgBDSSIMRateUOverlap[2])
        worksheet.write(y+seqnr+2, x+16, setavgBDSSIMRateU[1])
        worksheet.write(y+seqnr+2, x+17, setavgBDSSIMRateUOverlap[1])
        worksheet.write(y+seqnr+2, x+18, setavgBDSSIMRateV[0])
        worksheet.write(y+seqnr+2, x+19, setavgBDSSIMRateVOverlap[0])
        worksheet.write(y+seqnr+2, x+20, setavgBDSSIMRateV[2])
        worksheet.write(y+seqnr+2, x+21, setavgBDSSIMRateVOverlap[2])
        worksheet.write(y+seqnr+2, x+22, setavgBDSSIMRateV[1])
        worksheet.write(y+seqnr+2, x+23, setavgBDSSIMRateVOverlap[1])

        #BD SSIM
        x = 2
        y = y+seqnr+2+8
        worksheet.write(y-4, x, "Sequence")
        worksheet.write(y-4, x+2, "Anchor")
        worksheet.write(y-3, x+2, "avgbitrate")
        worksheet.write(y-3, x+3, "avgSSIM")
        worksheet.write(y-2, x+2, "[kbps]")
        worksheet.write(y-4, x+4, "Test")
        worksheet.write(y-3, x+4, "avgbitrate")
        worksheet.write(y-3, x+5, "avgSSIM")
        worksheet.write(y-2, x+4, "[kbps]")
        worksheet.write(y-4, x+6, "BD SSIM Luma")
        worksheet.write(y-1, x+6, "avg")
        worksheet.write(y-1, x+7, "overlap")
        worksheet.write(y-1, x+8, "low")
        worksheet.write(y-1, x+9, "overlap")
        worksheet.write(y-1, x+10, "high")
        worksheet.write(y-1, x+11, "overlap")
        worksheet.write(y-4, x+12, "BD SSIM U")
        worksheet.write(y-1, x+12, "avg")
        worksheet.write(y-1, x+13, "overlap")
        worksheet.write(y-1, x+14, "low")
        worksheet.write(y-1, x+15, "overlap")
        worksheet.write(y-1, x+16, "high")
        worksheet.write(y-1, x+17, "overlap")
        worksheet.write(y-4, x+18, "BD SSIM  V")
        worksheet.write(y-1, x+18, "avg")
        worksheet.write(y-1, x+19, "overlap")
        worksheet.write(y-1, x+20, "low")
        worksheet.write(y-1, x+21, "overlap")
        worksheet.write(y-1, x+22, "high")
        worksheet.write(y-1, x+23, "overlap")

        seqnr = 0
        for seq in range(len(seqs)):
         if(seqOK[seq]):
          worksheet.write(y+seqnr, x, seqs[seq].name)
          worksheet.write(y+seqnr, x+2, avgbitrate[seqnr][0])
          worksheet.write(y+seqnr, x+3, avgssimY[seqnr][0])
          worksheet.write(y+seqnr, x+4, avgbitrate[seqnr][1])
          worksheet.write(y+seqnr, x+5, avgssimY[seqnr][1])
          worksheet.write(y+seqnr, x+6, BDSSIM[seqnr][0])
          worksheet.write(y+seqnr, x+7, BDSSIMOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+8, BDSSIM[seqnr][2])
          worksheet.write(y+seqnr, x+9, BDSSIMOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+10, BDSSIM[seqnr][1])
          worksheet.write(y+seqnr, x+11, BDSSIMOverlap[seqnr][1])
          worksheet.write(y+seqnr, x+12, BDSSIMU[seqnr][0])
          worksheet.write(y+seqnr, x+13, BDSSIMUOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+14, BDSSIMU[seqnr][2])
          worksheet.write(y+seqnr, x+15, BDSSIMUOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+16, BDSSIMU[seqnr][1])
          worksheet.write(y+seqnr, x+17, BDSSIMUOverlap[seqnr][1])
          worksheet.write(y+seqnr, x+18, BDSSIMV[seqnr][0])
          worksheet.write(y+seqnr, x+19, BDSSIMVOverlap[seqnr][0])
          worksheet.write(y+seqnr, x+20, BDSSIMV[seqnr][2])
          worksheet.write(y+seqnr, x+21, BDSSIMVOverlap[seqnr][2])
          worksheet.write(y+seqnr, x+22, BDSSIMV[seqnr][1])
          worksheet.write(y+seqnr, x+23, BDSSIMVOverlap[seqnr][1])
          seqnr = seqnr + 1

        worksheet.write(y+seqnr+2, x, "Average")
        worksheet.write(y+seqnr+2, x+2, setavgbitrate[0])
        worksheet.write(y+seqnr+2, x+3, setavgssim[0])
        worksheet.write(y+seqnr+2, x+4, setavgbitrate[1])
        worksheet.write(y+seqnr+2, x+5, setavgssim[1])
        worksheet.write(y+seqnr+2, x+6, setavgBDSSIMY[0])
        worksheet.write(y+seqnr+2, x+7, setavgBDSSIMYOverlap[0])
        worksheet.write(y+seqnr+2, x+8, setavgBDSSIMY[2])
        worksheet.write(y+seqnr+2, x+9, setavgBDSSIMYOverlap[2])
        worksheet.write(y+seqnr+2, x+10, setavgBDSSIMY[1])
        worksheet.write(y+seqnr+2, x+11, setavgBDSSIMYOverlap[1])
        worksheet.write(y+seqnr+2, x+12, setavgBDSSIMU[0])
        worksheet.write(y+seqnr+2, x+13, setavgBDSSIMUOverlap[0])
        worksheet.write(y+seqnr+2, x+14, setavgBDSSIMU[2])
        worksheet.write(y+seqnr+2, x+15, setavgBDSSIMUOverlap[2])
        worksheet.write(y+seqnr+2, x+16, setavgBDSSIMU[1])
        worksheet.write(y+seqnr+2, x+17, setavgBDSSIMUOverlap[1])
        worksheet.write(y+seqnr+2, x+18, setavgBDSSIMV[0])
        worksheet.write(y+seqnr+2, x+19, setavgBDSSIMVOverlap[0])
        worksheet.write(y+seqnr+2, x+20, setavgBDSSIMV[2])
        worksheet.write(y+seqnr+2, x+21, setavgBDSSIMVOverlap[2])
        worksheet.write(y+seqnr+2, x+22, setavgBDSSIMV[1])
        worksheet.write(y+seqnr+2, x+23, setavgBDSSIMVOverlap[1])


    def plot(self, seq, bitrate1, psnrY1, psnrU1, psnrV1, enctime1, dectime1, bitrate2, psnrY2, psnrU2, psnrV2, enctime2, dectime2, BD, BDU, BDV, BDOverlap, BDUOverlap, BDVOverlap, BDPSNR, BDPSNRU, BDPSNRV, BDPSNROverlap, BDPSNRUOverlap, BDPSNRVOverlap, avgbitrate, avgpsnrY):
        worksheet = self.workbook.add_sheet(seq)

        numQps = len(bitrate1)
        assert len(psnrY1)==numQps
        assert len(psnrU1)==numQps
        assert len(psnrV1)==numQps
        assert len(enctime1)==numQps
        assert len(dectime1)==numQps
        assert len(bitrate2)==numQps
        assert len(psnrY2)==numQps
        assert len(psnrU2)==numQps
        assert len(psnrV2)==numQps
        assert len(enctime2)==numQps
        assert len(dectime2)==numQps

        # starting point
        # to have extra points just repeat the
        # code below with new starting points
        x = 4
        y = 3

        worksheet.write(y-1, x, "Anchor")
        worksheet.write(y, x-3, "Enc usr [s]")
        worksheet.write(y, x-2, "Dec usr [s]")
        worksheet.write(y+2+numQps, x-3, "Enc sys [s]")
        worksheet.write(y+2+numQps, x-2, "Dec sys [s]")
        worksheet.write(y+4+2*numQps, x-3, "Enc all [s]")
        worksheet.write(y+4+2*numQps, x-2, "Dec all [s]")
        worksheet.write(y, x, "bitrate [kbps]")
        worksheet.write(y, x+1, "PSNR Y")
        worksheet.write(y, x+2, "PSNR U")
        worksheet.write(y, x+3, "PSNR V")
        for i in range(numQps):
            worksheet.write(y+1+i, x, bitrate1[i])
            worksheet.write(y+1+i, x+1, psnrY1[i])
            worksheet.write(y+1+i, x+2, psnrU1[i])
            worksheet.write(y+1+i, x+3, psnrV1[i])
            worksheet.write(y+1+i, x-3, enctime1[i][0])
            worksheet.write(y+1+i, x-2, dectime1[i][0])
            worksheet.write(y+1+i+2+numQps, x-3, enctime1[i][1])
            worksheet.write(y+1+i+2+numQps, x-2, dectime1[i][1])
            worksheet.write(y+1+i+4+2*numQps, x-3, enctime1[i][2])
            worksheet.write(y+1+i+4+2*numQps, x-2, dectime1[i][2])


        x = 4
        y = 5+numQps
        worksheet.write(y, x, "avg bitrate")
        worksheet.write(y+1, x, avgbitrate[0])
        worksheet.write(y, x+1, "avg PSNR")
        worksheet.write(y+1, x+1, avgpsnrY[0])

        x = 10
        y = 3

        worksheet.write(y-1, x, "Test")
        worksheet.write(y, x+5, "Enc usr [s]")
        worksheet.write(y, x+6, "Dec usr [s]")
        worksheet.write(y+2+numQps, x+5, "Enc sys [s]")
        worksheet.write(y+2+numQps, x+6, "Dec sys [s]")
        worksheet.write(y+4+2*numQps, x+5, "Enc all [s]")
        worksheet.write(y+4+2*numQps, x+6, "Dec all [s]")
        worksheet.write(y, x, "bitrate [kbps]")
        worksheet.write(y, x+1, "PSNR Y")
        worksheet.write(y, x+2, "PSNR U")
        worksheet.write(y, x+3, "PSNR V")
        for i in range(numQps):
            worksheet.write(y+1+i, x, bitrate2[i])
            worksheet.write(y+1+i, x+1, psnrY2[i])
            worksheet.write(y+1+i, x+2, psnrU2[i])
            worksheet.write(y+1+i, x+3, psnrV2[i])
            worksheet.write(y+1+i, x+5, enctime2[i][0])
            worksheet.write(y+1+i, x+6, dectime2[i][0])
            worksheet.write(y+1+i+2+numQps, x+5, enctime2[i][1])
            worksheet.write(y+1+i+2+numQps, x+6, dectime2[i][1])
            worksheet.write(y+1+i+4+2*numQps, x+5, enctime2[i][2])
            worksheet.write(y+1+i+4+2*numQps, x+6, dectime2[i][2])

        x = 10
        y = 5+numQps
        worksheet.write(y, x, "avg bitrate")
        worksheet.write(y+1, x, avgbitrate[1])
        worksheet.write(y, x+1, "avg PSNR")
        worksheet.write(y+1, x+1, avgpsnrY[1])

        x = 4
        y = 8+numQps
        worksheet.write(y, x, "BD Rates Luma [%]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BD[0])
        worksheet.write(y+2, x+1, BD[1])
        worksheet.write(y+3, x+1, BD[2])
        worksheet.write(y+1, x+2, BDOverlap[0])
        worksheet.write(y+2, x+2, BDOverlap[1])
        worksheet.write(y+3, x+2, BDOverlap[2])

        y = 8+numQps+5
        worksheet.write(y, x, "BD Rates U [%]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDU[0])
        worksheet.write(y+2, x+1, BDU[1])
        worksheet.write(y+3, x+1, BDU[2])
        worksheet.write(y+1, x+2, BDUOverlap[0])
        worksheet.write(y+2, x+2, BDUOverlap[1])
        worksheet.write(y+3, x+2, BDUOverlap[2])


        y = 8+numQps+5+5
        worksheet.write(y, x, "BD Rates V [%]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDV[0])
        worksheet.write(y+2, x+1, BDV[1])
        worksheet.write(y+3, x+1, BDV[2])
        worksheet.write(y+1, x+2, BDVOverlap[0])
        worksheet.write(y+2, x+2, BDVOverlap[1])
        worksheet.write(y+3, x+2, BDVOverlap[2])


        x = 10
        y = 8+numQps
        worksheet.write(y, x, "BD PSNR Luma [dB]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDPSNR[0])
        worksheet.write(y+2, x+1, BDPSNR[1])
        worksheet.write(y+3, x+1, BDPSNR[2])
        worksheet.write(y+1, x+2, BDPSNROverlap[0])
        worksheet.write(y+2, x+2, BDPSNROverlap[1])
        worksheet.write(y+3, x+2, BDPSNROverlap[2])

        y = 8+numQps+5
        worksheet.write(y, x, "BD PSNR U [dB]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDPSNRU[0])
        worksheet.write(y+2, x+1, BDPSNRU[1])
        worksheet.write(y+3, x+1, BDPSNRU[2])
        worksheet.write(y+1, x+2, BDPSNRUOverlap[0])
        worksheet.write(y+2, x+2, BDPSNRUOverlap[1])
        worksheet.write(y+3, x+2, BDPSNRUOverlap[2])


        y = 8+numQps+5+5
        worksheet.write(y, x, "BD PSNR V [dB]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDPSNRV[0])
        worksheet.write(y+2, x+1, BDPSNRV[1])
        worksheet.write(y+3, x+1, BDPSNRV[2])
        worksheet.write(y+1, x+2, BDPSNRVOverlap[0])
        worksheet.write(y+2, x+2, BDPSNRVOverlap[1])
        worksheet.write(y+3, x+2, BDPSNRVOverlap[2])


    def plotWithSSIM(self, seq, bitrate1, psnrY1, psnrU1, psnrV1, enctime1, dectime1, bitrate2, psnrY2, psnrU2, psnrV2, enctime2, dectime2, BD, BDU, BDV, BDOverlap, BDUOverlap, BDVOverlap, BDPSNR, BDPSNRU, BDPSNRV, BDPSNROverlap, BDPSNRUOverlap, BDPSNRVOverlap, avgbitrate, avgpsnrY, ssimY1, ssimU1, ssimV1, ssimY2, ssimU2, ssimV2, avgssimY, BDSSIMRate, BDSSIMRateU, BDSSIMRateV, BDSSIMRateOverlap, BDSSIMRateUOverlap, BDSSIMRateVOverlap, BDSSIM, BDSSIMU, BDSSIMV, BDSSIMOverlap, BDSSIMUOverlap, BDSSIMVOverlap):
        worksheet = self.workbook.add_sheet(seq)

        numQps = len(bitrate1)
        assert len(psnrY1)==numQps
        assert len(psnrU1)==numQps
        assert len(psnrV1)==numQps
        assert len(enctime1)==numQps
        assert len(dectime1)==numQps
        assert len(bitrate2)==numQps
        assert len(psnrY2)==numQps
        assert len(psnrU2)==numQps
        assert len(psnrV2)==numQps
        assert len(enctime2)==numQps
        assert len(dectime2)==numQps

        # starting point
        # to have extra points just repeat the
        # code below with new starting points
        x = 4
        y = 3

        worksheet.write(y-1, x, "Anchor")
        worksheet.write(y, x-3, "Enc usr [s]")
        worksheet.write(y, x-2, "Dec usr [s]")
        worksheet.write(y+2+numQps, x-3, "Enc sys [s]")
        worksheet.write(y+2+numQps, x-2, "Dec sys [s]")
        worksheet.write(y+4+2*numQps, x-3, "Enc all [s]")
        worksheet.write(y+4+2*numQps, x-2, "Dec all [s]")
        worksheet.write(y, x, "bitrate [kbps]")
        worksheet.write(y, x+1, "PSNR Y")
        worksheet.write(y, x+2, "PSNR U")
        worksheet.write(y, x+3, "PSNR V")
        for i in range(numQps):
            worksheet.write(y+1+i, x, bitrate1[i])
            worksheet.write(y+1+i, x+1, psnrY1[i])
            worksheet.write(y+1+i, x+2, psnrU1[i])
            worksheet.write(y+1+i, x+3, psnrV1[i])
            worksheet.write(y+1+i, x-3, enctime1[i][0])
            worksheet.write(y+1+i, x-2, dectime1[i][0])
            worksheet.write(y+1+i+2+numQps, x-3, enctime1[i][1])
            worksheet.write(y+1+i+2+numQps, x-2, dectime1[i][1])
            worksheet.write(y+1+i+4+2*numQps, x-3, enctime1[i][2])
            worksheet.write(y+1+i+4+2*numQps, x-2, dectime1[i][2])


        x = 4
        y = 5+numQps
        worksheet.write(y, x, "avg bitrate")
        worksheet.write(y+1, x, avgbitrate[0])
        worksheet.write(y, x+1, "avg PSNR")
        worksheet.write(y+1, x+1, avgpsnrY[0])

        x = 10
        y = 3

        worksheet.write(y-1, x, "Test")
        worksheet.write(y, x+5, "Enc usr [s]")
        worksheet.write(y, x+6, "Dec usr [s]")
        worksheet.write(y+2+numQps, x+5, "Enc sys [s]")
        worksheet.write(y+2+numQps, x+6, "Dec sys [s]")
        worksheet.write(y+4+2*numQps, x+5, "Enc all [s]")
        worksheet.write(y+4+2*numQps, x+6, "Dec all [s]")
        worksheet.write(y, x, "bitrate [kbps]")
        worksheet.write(y, x+1, "PSNR Y")
        worksheet.write(y, x+2, "PSNR U")
        worksheet.write(y, x+3, "PSNR V")
        for i in range(numQps):
            worksheet.write(y+1+i, x, bitrate2[i])
            worksheet.write(y+1+i, x+1, psnrY2[i])
            worksheet.write(y+1+i, x+2, psnrU2[i])
            worksheet.write(y+1+i, x+3, psnrV2[i])
            worksheet.write(y+1+i, x+5, enctime2[i][0])
            worksheet.write(y+1+i, x+6, dectime2[i][0])
            worksheet.write(y+1+i+2+numQps, x+5, enctime2[i][1])
            worksheet.write(y+1+i+2+numQps, x+6, dectime2[i][1])
            worksheet.write(y+1+i+4+2*numQps, x+5, enctime2[i][2])
            worksheet.write(y+1+i+4+2*numQps, x+6, dectime2[i][2])

        x = 10
        y = 5+numQps
        worksheet.write(y, x, "avg bitrate")
        worksheet.write(y+1, x, avgbitrate[1])
        worksheet.write(y, x+1, "avg PSNR")
        worksheet.write(y+1, x+1, avgpsnrY[1])

        x = 4
        y = 8+numQps
        worksheet.write(y, x, "BD Rates Luma [%]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BD[0])
        worksheet.write(y+2, x+1, BD[1])
        worksheet.write(y+3, x+1, BD[2])
        worksheet.write(y+1, x+2, BDOverlap[0])
        worksheet.write(y+2, x+2, BDOverlap[1])
        worksheet.write(y+3, x+2, BDOverlap[2])

        y = 8+numQps+5
        worksheet.write(y, x, "BD Rates U [%]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDU[0])
        worksheet.write(y+2, x+1, BDU[1])
        worksheet.write(y+3, x+1, BDU[2])
        worksheet.write(y+1, x+2, BDUOverlap[0])
        worksheet.write(y+2, x+2, BDUOverlap[1])
        worksheet.write(y+3, x+2, BDUOverlap[2])


        y = 8+numQps+5+5
        worksheet.write(y, x, "BD Rates V [%]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDV[0])
        worksheet.write(y+2, x+1, BDV[1])
        worksheet.write(y+3, x+1, BDV[2])
        worksheet.write(y+1, x+2, BDVOverlap[0])
        worksheet.write(y+2, x+2, BDVOverlap[1])
        worksheet.write(y+3, x+2, BDVOverlap[2])


        x = 10
        y = 8+numQps
        worksheet.write(y, x, "BD PSNR Luma [dB]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDPSNR[0])
        worksheet.write(y+2, x+1, BDPSNR[1])
        worksheet.write(y+3, x+1, BDPSNR[2])
        worksheet.write(y+1, x+2, BDPSNROverlap[0])
        worksheet.write(y+2, x+2, BDPSNROverlap[1])
        worksheet.write(y+3, x+2, BDPSNROverlap[2])

        y = 8+numQps+5
        worksheet.write(y, x, "BD PSNR U [dB]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDPSNRU[0])
        worksheet.write(y+2, x+1, BDPSNRU[1])
        worksheet.write(y+3, x+1, BDPSNRU[2])
        worksheet.write(y+1, x+2, BDPSNRUOverlap[0])
        worksheet.write(y+2, x+2, BDPSNRUOverlap[1])
        worksheet.write(y+3, x+2, BDPSNRUOverlap[2])


        y = 8+numQps+5+5
        worksheet.write(y, x, "BD PSNR V [dB]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDPSNRV[0])
        worksheet.write(y+2, x+1, BDPSNRV[1])
        worksheet.write(y+3, x+1, BDPSNRV[2])
        worksheet.write(y+1, x+2, BDPSNRVOverlap[0])
        worksheet.write(y+2, x+2, BDPSNRVOverlap[1])
        worksheet.write(y+3, x+2, BDPSNRVOverlap[2])

        # SSIM
        x = 4
        y = 3+numQps+21

        worksheet.write(y, x, "bitrate [kbps]")
        worksheet.write(y, x+1, "SSIM Y")
        worksheet.write(y, x+2, "SSIM U")
        worksheet.write(y, x+3, "SSIM V")
        for i in range(numQps):
            worksheet.write(y+1+i, x, bitrate1[i])
            worksheet.write(y+1+i, x+1, ssimY1[i])
            worksheet.write(y+1+i, x+2, ssimU1[i])
            worksheet.write(y+1+i, x+3, ssimV1[i])


        x = 4
        y = 5+numQps+numQps+21
        worksheet.write(y, x, "avg bitrate")
        worksheet.write(y+1, x, avgbitrate[0])
        worksheet.write(y, x+1, "avg SSIM")
        worksheet.write(y+1, x+1, avgssimY[0])

        x = 10
        y = 3+numQps+21

        worksheet.write(y, x, "bitrate [kbps]")
        worksheet.write(y, x+1, "SSIM Y")
        worksheet.write(y, x+2, "SSIM U")
        worksheet.write(y, x+3, "SSIM V")
        for i in range(numQps):
            worksheet.write(y+1+i, x, bitrate2[i])
            worksheet.write(y+1+i, x+1, ssimY2[i])
            worksheet.write(y+1+i, x+2, ssimU2[i])
            worksheet.write(y+1+i, x+3, ssimV2[i])


        x = 10
        y = 5+numQps+numQps+21
        worksheet.write(y, x, "avg bitrate")
        worksheet.write(y+1, x, avgbitrate[1])
        worksheet.write(y, x+1, "avg SSIM")
        worksheet.write(y+1, x+1, avgssimY[1])

        x = 4
        y = 8+numQps+numQps+21
        worksheet.write(y, x, "BD SSIM Rates Luma [%]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDSSIMRate[0])
        worksheet.write(y+2, x+1, BDSSIMRate[1])
        worksheet.write(y+3, x+1, BDSSIMRate[2])
        worksheet.write(y+1, x+2, BDSSIMRateOverlap[0])
        worksheet.write(y+2, x+2, BDSSIMRateOverlap[1])
        worksheet.write(y+3, x+2, BDSSIMRateOverlap[2])

        y = 8+numQps+5+numQps+21
        worksheet.write(y, x, "BD SSIM Rates U [%]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDSSIMRateU[0])
        worksheet.write(y+2, x+1, BDSSIMRateU[1])
        worksheet.write(y+3, x+1, BDSSIMRateU[2])
        worksheet.write(y+1, x+2, BDSSIMRateUOverlap[0])
        worksheet.write(y+2, x+2, BDSSIMRateUOverlap[1])
        worksheet.write(y+3, x+2, BDSSIMRateUOverlap[2])


        y = 8+numQps+5+5+numQps+21
        worksheet.write(y, x, "BD SSIM Rates V [%]")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDSSIMRateV[0])
        worksheet.write(y+2, x+1, BDSSIMRateV[1])
        worksheet.write(y+3, x+1, BDSSIMRateV[2])
        worksheet.write(y+1, x+2, BDSSIMRateVOverlap[0])
        worksheet.write(y+2, x+2, BDSSIMRateVOverlap[1])
        worksheet.write(y+3, x+2, BDSSIMRateVOverlap[2])


        x = 10
        y = 8+numQps+numQps+21
        worksheet.write(y, x, "BD SSIM Luma")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDSSIM[0])
        worksheet.write(y+2, x+1, BDSSIM[1])
        worksheet.write(y+3, x+1, BDSSIM[2])
        worksheet.write(y+1, x+2, BDSSIMOverlap[0])
        worksheet.write(y+2, x+2, BDSSIMOverlap[1])
        worksheet.write(y+3, x+2, BDSSIMOverlap[2])

        y = 8+numQps+5+numQps+21
        worksheet.write(y, x, "BD SSIM U")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDSSIMU[0])
        worksheet.write(y+2, x+1, BDSSIMU[1])
        worksheet.write(y+3, x+1, BDSSIMU[2])
        worksheet.write(y+1, x+2, BDSSIMUOverlap[0])
        worksheet.write(y+2, x+2, BDSSIMUOverlap[1])
        worksheet.write(y+3, x+2, BDSSIMUOverlap[2])


        y = 8+numQps+5+5+numQps+21
        worksheet.write(y, x, "BD SSIM V")
        worksheet.write(y, x+2, "Overlap [%]")
        worksheet.write(y+1, x, "Average")
        worksheet.write(y+2, x, "High")
        worksheet.write(y+3, x, "Low")
        worksheet.write(y+1, x+1, BDSSIMV[0])
        worksheet.write(y+2, x+1, BDSSIMV[1])
        worksheet.write(y+3, x+1, BDSSIMV[2])
        worksheet.write(y+1, x+2, BDSSIMVOverlap[0])
        worksheet.write(y+2, x+2, BDSSIMVOverlap[1])
        worksheet.write(y+3, x+2, BDSSIMVOverlap[2])

    def plotPSNR(self, seq, psnrY1, psnrU1, psnrV1):
        worksheet = self.workbook.add_sheet(seq)
        x=3
        y=12
        for theQP in range(len(psnrY1)):
          for i in range(len(psnrY1[theQP])):
            worksheet.write(y+i, x, psnrY1[theQP][i])
            worksheet.write(y+i, x+1, psnrU1[theQP][i])
            worksheet.write(y+i, x+2, psnrV1[theQP][i])
          x=x+6

    def plotBits(self, seq, psnrY1, psnrU1, psnrV1, bits, totbits):
        worksheet = self.workbook.add_sheet(seq)
        x=3
        y=12
        print seq
        for theQP in range(len(psnrY1)):
          i=0
          worksheet.write(y+i-2, x-1, totbits[theQP][0])
          for i in range(len(psnrY1[theQP])):
            worksheet.write(y+i, x-1, bits[theQP][i])
            worksheet.write(y+i, x, psnrY1[theQP][i])
            worksheet.write(y+i, x+1, psnrU1[theQP][i])
            worksheet.write(y+i, x+2, psnrV1[theQP][i])
          x=x+6


    def plotMPEGHDRPage(self, seqOK, sheetname, seqs, seqrange, results1, results2, simid, simid2, BD, BDU, BDV, setavgBDY, setavgBDU, setavgBDV, setavgtime, type, numdecimals): #, lc):
        worksheet = self.workbook.add_sheet(sheetname)

        # results[seq][0...N] contain bitrate, tPSNR X, tPSNR Y, tPSNR Z, tPSNR XYZ, Y psnr, U psnr, V psnr, YUV psnr, deltaE psnr, mPsnr, enctime, dectime

        # mapping from our sequence order to MPEG order
        domap=1
        if domap:
             themap = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19]
        else:
             print "warning no mapping of sequence number is used\n"
             themap = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19] #no map
        listlen = len(themap)-1

        res1    = {idx: results1[idx] for idx in seqrange}
        res2    = {idx: results2[idx] for idx in seqrange}

        x = 2
        y = 2

        if sheetname == "CT.2020":
            if len(results2[0][0]) == 1: # Uncompressed
                worksheet.write(y,    x-2, "class A")
                worksheet.write(y+3,  x-2, "class B")
                worksheet.write(y+5,  x-2, "class C")
                worksheet.write(y+6,  x-2, "class D")
                worksheet.write(y+8,  x-2, "class G")
                rowsskipped = [0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0]
            else: # Compressed
                worksheet.write(y,    x-2, "class A")
                worksheet.write(y+12, x-2, "class B")
                worksheet.write(y+20, x-2, "class C")
                worksheet.write(y+24, x-2, "class D")
                worksheet.write(y+32, x-2, "class G")
                rowsskipped = [0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0]
        elif sheetname == "class A_prime":
                worksheet.write(y, x-2, "class A' (709 container)")
                rowsskipped = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        elif sheetname == "709 problem sequences":
                worksheet.write(y, x-2, "709 problem sequences")
                rowsskipped = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]


        for entity in ["Reference", "Tested"]:
            if entity == "Reference":
                offset  = 0
                res = res1
            elif entity == "Tested":
                offset  = 18
                res = res2
            seqnr = 0
            worksheet.write(y-2, x+offset+1, entity)
            worksheet.write(y-1, x+offset+0, "QP Slice")
            worksheet.write(y-1, x+offset+1, "kbps")
            worksheet.write(y-1, x+offset+2, "tPSNR X")
            worksheet.write(y-1, x+offset+3, "tPSNR Y")
            worksheet.write(y-1, x+offset+4, "tPSNR Z")
            worksheet.write(y-1, x+offset+5, "tPSNR XYZ")
            worksheet.write(y-1, x+offset+6, "Y psnr")
            worksheet.write(y-1, x+offset+7, "U psnr")
            worksheet.write(y-1, x+offset+8, "V psnr")
            worksheet.write(y-1, x+offset+9, "YUV psnr")
            worksheet.write(y-1, x+offset+10, "deltaE psnr")
            worksheet.write(y-1, x+offset+11, "mPsnr")
            worksheet.write(y-1, x+offset+12, "Enc T [s]")
            worksheet.write(y-1, x+offset+13, "Dec T [s]")
            worksheet.write(y-1, x+offset+14, "Enc T [h]")
            worksheet.write(y-1, x+offset+15, "Preproc T [s]")
            worksheet.write(y-1, x+offset+16, "Postproc T [s]")

            for seq in seqrange:
             if domap:
                 seqnr+=rowsskipped[seq]

             if(seqOK[seq]):
               if(seq>listlen):
                  thenum = seq;
               else:
                  thenum = themap[seq]
               numQps = len(res[seq][0])

               if entity == "Reference":
                  worksheet.write(y+seqnr, x-1, seqs[thenum].name)

               for qp in range(numQps):
                    mapqp = qp
                    thelen = len(simid[thenum][mapqp])
                    thetxt = simid[thenum][mapqp]
                    val = "%s%s" % (thetxt[thelen-5],thetxt[thelen-4])
                    if(val.isdigit()==0):
                        val = "%s" % (thetxt[thelen-4])

                    worksheet.write(y+qp+seqnr, x+offset+0, int(val))                                  # QP
                    worksheet.write(y+qp+seqnr, x+offset+1, round(res[thenum][0][mapqp],numdecimals))  # bitrate
                    worksheet.write(y+qp+seqnr, x+offset+2, round(res[thenum][1][mapqp],numdecimals))  # tPSNR X
                    worksheet.write(y+qp+seqnr, x+offset+3, round(res[thenum][2][mapqp],numdecimals))  # tPSNR Y
                    worksheet.write(y+qp+seqnr, x+offset+4, round(res[thenum][3][mapqp],numdecimals))  # tPSNR Z
                    worksheet.write(y+qp+seqnr, x+offset+5, round(res[thenum][4][mapqp],numdecimals))  # tPSNR XYZ
                    worksheet.write(y+qp+seqnr, x+offset+6, round(res[thenum][5][mapqp],numdecimals))  # Y PSNR
                    worksheet.write(y+qp+seqnr, x+offset+7, round(res[thenum][6][mapqp],numdecimals))  # U PSNR
                    worksheet.write(y+qp+seqnr, x+offset+8, round(res[thenum][7][mapqp],numdecimals))  # V PSNR
                    worksheet.write(y+qp+seqnr, x+offset+9, round(res[thenum][8][mapqp],numdecimals))  # YUV PSNR
                    worksheet.write(y+qp+seqnr, x+offset+10, round(res[thenum][9][mapqp],numdecimals))  # deltaE
                    worksheet.write(y+qp+seqnr, x+offset+11, round(res[thenum][10][mapqp],numdecimals))  # mPSNR
                    worksheet.write(y+qp+seqnr, x+offset+12, round(res[thenum][11][mapqp][0],2))         # enctime [s]
                    worksheet.write(y+qp+seqnr, x+offset+14, round(res[thenum][11][mapqp][0]/3600.0,2)) # enctime [h]
                    worksheet.write(y+qp+seqnr, x+offset+13, round(res[thenum][12][mapqp][0],2))         # dectime
                    worksheet.write(y+qp+seqnr, x+offset+15, round(res[thenum][13][mapqp],2))         # preprocTime
                    worksheet.write(y+qp+seqnr, x+offset+16, round(res[thenum][14][mapqp],2))         # postprocTime
               seqnr=seqnr+numQps

        # plot average BD rates
        offset  = 34
        bdranges = ["", "High rates ", "Low rates "]
        for bdrange in range(3):
            seqnr = 0
            thestr = bdranges[bdrange] + "BD rate " +"with different metrics (not exact match with MPEG HDR template)"
            worksheet.write(y-2, x+offset+2, thestr)
            #worksheet.write(y-2, x+offset+1, "BD rate with different metrics (not exactlt same results as MPEG HDR template")
            worksheet.write(y-1, x+offset+2, "tPSNR X")
            worksheet.write(y-1, x+offset+3, "tPSNR Y")
            worksheet.write(y-1, x+offset+4, "tPSNR Z")
            worksheet.write(y-1, x+offset+5, "tPSNR XYZ")
            worksheet.write(y-1, x+offset+6, "Y psnr")
            worksheet.write(y-1, x+offset+7, "U psnr")
            worksheet.write(y-1, x+offset+8, "V psnr")
            worksheet.write(y-1, x+offset+9, "YUV psnr")
            worksheet.write(y-1, x+offset+10, "deltaE psnr")
            worksheet.write(y-1, x+offset+11, "mPsnr")

            for seq in seqrange:
             if domap:
                 seqnr+=rowsskipped[seq]

             if(seqOK[seq]):
               if(seq>listlen):
                  thenum = seq;
               else:
                  thenum = themap[seq]
               #bdrange = 0 # corresponds to average
               if isinstance(BD[thenum][1],dict):
                   worksheet.write(y+seqnr, x+offset+2, round(BD[thenum][1][bdrange],numdecimals))  # tPSNR X
                   worksheet.write(y+seqnr, x+offset+3, round(BD[thenum][2][bdrange],numdecimals))  # tPSNR Y
                   worksheet.write(y+seqnr, x+offset+4, round(BD[thenum][3][bdrange],numdecimals))  # tPSNR Z
                   worksheet.write(y+seqnr, x+offset+5, round(BD[thenum][4][bdrange],numdecimals))  # tPSNR XYZ
                   worksheet.write(y+seqnr, x+offset+6, round(BD[thenum][5][bdrange],numdecimals))  # Y PSNR
                   worksheet.write(y+seqnr, x+offset+7, round(BD[thenum][6][bdrange],numdecimals))  # U PSNR
                   worksheet.write(y+seqnr, x+offset+8, round(BD[thenum][7][bdrange],numdecimals))  # V PSNR
                   worksheet.write(y+seqnr, x+offset+9, round(BD[thenum][8][bdrange],numdecimals))  # YUV PSNR
                   worksheet.write(y+seqnr, x+offset+10, round(BD[thenum][9][bdrange],numdecimals))  # deltaE
                   worksheet.write(y+seqnr, x+offset+11, round(BD[thenum][10][bdrange],numdecimals))  # mPSNR
                   seqnr=seqnr+len(res1[seq][0])
            offset = offset + 11



    def plotMPEGHDRSHVCPage(self, seqOK, sheetname, seqs, seqrange, results1, results2, simid, simid2, BD, BDU, BDV, setavgBDY, setavgBDU, setavgBDV, setavgtime, type, numdecimals): #, lc):
        #if shvcSet2:
        #    worksheet = self.workbook.get_sheet([n for n, s in enumerate(self.workbook._Workbook__worksheets) if s._Worksheet__name == 'SHVC'][0])
        #else:
        worksheet = self.workbook.add_sheet(sheetname)


        # results[seq][0...N] contain bitrate, tPSNR X, tPSNR Y, tPSNR Z, tPSNR XYZ, Y psnr, U psnr, V psnr, YUV psnr, deltaE psnr, mPsnr, enctime, dectime

        themap = [0,1,2,4,5,6,7,3,8,9,10,11,12,13,14,15,16,17,18,19]

        res1    = {idx: results1[idx] for idx in seqrange}
        res2    = {idx: results2[idx] for idx in seqrange}

        # Reference and Tested
        x = 2
        y = 2

        if sheetname == "SHVC":
            worksheet.write(y,    x-2, "class A (SA)")
            worksheet.write(y+24, x-2, "class G (SA)")
            worksheet.write(y+32, x-2, "class A (SC)")
            worksheet.write(y+56, x-2, "class B (SC)")
            worksheet.write(y+72, x-2, "class C (SC)")
            worksheet.write(y+80, x-2, "class D (SC)")
            worksheet.write(y+96, x-2, "class G (SC)")

            rowsskippedLoop1 = [0,0,0,0,0,0,0,0,0,0,0]
            rowsskippedLoop2 = [0,0,0,0,0,8,0,0,0,0,0]
            loopOver = ['_AG_','_CG_']

        for entity in ["Reference", "Tested"]:
            if entity == "Reference":
                offset  = 1
                res = res1
            elif entity == "Tested":
                offset  = 17
                res = res2
            seqnr = 0

            worksheet.write(y-2, x+offset+1, entity)
            worksheet.write(y-1, x+offset+0, "QP Slice")
            worksheet.write(y-1, x+offset+1, "kbps")
            worksheet.write(y-1, x+offset+2, "tPSNR X")
            worksheet.write(y-1, x+offset+3, "tPSNR Y")
            worksheet.write(y-1, x+offset+4, "tPSNR Z")
            worksheet.write(y-1, x+offset+5, "tPSNR XYZ")
            worksheet.write(y-1, x+offset+6, "Y psnr")
            worksheet.write(y-1, x+offset+7, "U psnr")
            worksheet.write(y-1, x+offset+8, "V psnr")
            worksheet.write(y-1, x+offset+9, "YUV psnr")
            worksheet.write(y-1, x+offset+10, "deltaE psnr")
            worksheet.write(y-1, x+offset+11, "mPsnr")
            worksheet.write(y-1, x+offset+12, "Enc T [s]")
            worksheet.write(y-1, x+offset+13, "Dec T [s]")
            worksheet.write(y-1, x+offset+14, "Enc T [h]")

            for loopCnt, txtPattern in enumerate(loopOver):
                for seq in seqrange:
                   if not loopCnt:
                        seqnr+=rowsskippedLoop1[seq]
                   else:
                        seqnr+=rowsskippedLoop2[seq]

                   thenum = themap[seq]
                   qpRange = [simid[themap[seq]].index(i) for i in [sid for sid in simid[themap[seq]] if txtPattern in sid]]

                   if entity == "Reference":
                      worksheet.write(y+seqnr, 1, seqs[thenum].name)

                   for qpCnt, mapqp in enumerate(qpRange):

                        qpBL = simid[thenum][mapqp].split('qp')[1].split('_')[0]
                        qpEL = simid[thenum][mapqp].split('qp')[-1].split('_')[0]
                        worksheet.write(y+qpCnt+seqnr, x+offset-1, int(qpBL))                               # QP BL
                        worksheet.write(y+qpCnt+seqnr, x+offset+0, int(qpEL))                                  # QP EL
                        #worksheet.write(y+qpCnt+seqnr, 1, simid[themap[seq]][mapqp])
                        worksheet.write(y+qpCnt+seqnr, x+offset+1, round(res[thenum][0][mapqp],numdecimals))  # bitrate
                        worksheet.write(y+qpCnt+seqnr, x+offset+2, round(res[thenum][1][mapqp],numdecimals))  # tPSNR X
                        worksheet.write(y+qpCnt+seqnr, x+offset+3, round(res[thenum][2][mapqp],numdecimals))  # tPSNR Y
                        worksheet.write(y+qpCnt+seqnr, x+offset+4, round(res[thenum][3][mapqp],numdecimals))  # tPSNR Z
                        worksheet.write(y+qpCnt+seqnr, x+offset+5, round(res[thenum][4][mapqp],numdecimals))  # tPSNR XYZ
                        worksheet.write(y+qpCnt+seqnr, x+offset+6, round(res[thenum][5][mapqp],numdecimals))  # Y PSNR
                        worksheet.write(y+qpCnt+seqnr, x+offset+7, round(res[thenum][6][mapqp],numdecimals))  # U PSNR
                        worksheet.write(y+qpCnt+seqnr, x+offset+8, round(res[thenum][7][mapqp],numdecimals))  # V PSNR
                        worksheet.write(y+qpCnt+seqnr, x+offset+9, round(res[thenum][8][mapqp],numdecimals))  # YUV PSNR
                        worksheet.write(y+qpCnt+seqnr, x+offset+10, round(res[thenum][9][mapqp],numdecimals))  # deltaE
                        worksheet.write(y+qpCnt+seqnr, x+offset+11, round(res[thenum][10][mapqp],numdecimals))  # mPSNR
                        worksheet.write(y+qpCnt+seqnr, x+offset+12, round(res[thenum][11][mapqp][0],2))         # enctime [s]
                        worksheet.write(y+qpCnt+seqnr, x+offset+14, round(res[thenum][11][mapqp][0]/3600.0,2)) # enctime [h]
                        worksheet.write(y+qpCnt+seqnr, x+offset+13, round(res[thenum][12][mapqp][0],2))         # dectime
                   seqnr+=len(qpRange)

        # plot average BD rates
        # offset  = 32
        # bdranges = ["", "High rates ", "Low rates "]
        # for bdrange in range(3):
        #     seqnr = 0
        #     thestr = bdranges[bdrange] + "BD rate " +"with different metrics (not exact match with MPEG HDR template)"
        #     worksheet.write(y-2, x+offset+2, thestr)
        #     #worksheet.write(y-2, x+offset+1, "BD rate with different metrics (not exactlt same results as MPEG HDR template")
        #     worksheet.write(y-1, x+offset+2, "tPSNR X")
        #     worksheet.write(y-1, x+offset+3, "tPSNR Y")
        #     worksheet.write(y-1, x+offset+4, "tPSNR Z")
        #     worksheet.write(y-1, x+offset+5, "tPSNR XYZ")
        #     worksheet.write(y-1, x+offset+6, "Y psnr")
        #     worksheet.write(y-1, x+offset+7, "U psnr")
        #     worksheet.write(y-1, x+offset+8, "V psnr")
        #     worksheet.write(y-1, x+offset+9, "YUV psnr")
        #     worksheet.write(y-1, x+offset+10, "deltaE psnr")
        #     worksheet.write(y-1, x+offset+11, "mPsnr")
        #
        #     for seq in seqrange:
        #      if domap:
        #          seqnr+=rowsskipped[seq]
        #
        #      if(seqOK[seq]):
        #        if(seq>listlen):
        #           thenum = seq;
        #        else:
        #           thenum = themap[seq]
        #        #bdrange = 0 # corresponds to average
        #        if isinstance(BD[thenum][1],dict):
        #            worksheet.write(y+s+seqnr, x+offset+2, round(BD[thenum][1][bdrange],numdecimals))  # tPSNR X
        #            worksheet.write(y+s+seqnr, x+offset+3, round(BD[thenum][2][bdrange],numdecimals))  # tPSNR Y
        #            worksheet.write(y+s+seqnr, x+offset+4, round(BD[thenum][3][bdrange],numdecimals))  # tPSNR Z
        #            worksheet.write(y+s+seqnr, x+offset+5, round(BD[thenum][4][bdrange],numdecimals))  # tPSNR XYZ
        #            worksheet.write(y+s+seqnr, x+offset+6, round(BD[thenum][5][bdrange],numdecimals))  # Y PSNR
        #            worksheet.write(y+s+seqnr, x+offset+7, round(BD[thenum][6][bdrange],numdecimals))  # U PSNR
        #            worksheet.write(y+s+seqnr, x+offset+8, round(BD[thenum][7][bdrange],numdecimals))  # V PSNR
        #            worksheet.write(y+s+seqnr, x+offset+9, round(BD[thenum][8][bdrange],numdecimals))  # YUV PSNR
        #            worksheet.write(y+s+seqnr, x+offset+10, round(BD[thenum][9][bdrange],numdecimals))  # deltaE
        #            worksheet.write(y+s+seqnr, x+offset+11, round(BD[thenum][10][bdrange],numdecimals))  # mPSNR
        #            seqnr=seqnr+len(res1[seq][0])
        #     offset = offset + 11


    def plotMPEGHDRSDRPage(self, seqOK, sheetname, seqs, seqrange, results1, results2, simid, simid2, BD, BDU, BDV, setavgBDY, setavgBDU, setavgBDV, setavgtime, type, numdecimals): #, lc):
        worksheet = self.workbook.add_sheet(sheetname)

        # results[seq][0...N] contain bitrate, tPSNR X, tPSNR Y, tPSNR Z, tPSNR XYZ, Y psnr, U psnr, V psnr, YUV psnr, deltaE psnr, mPsnr, enctime, dectime

        # mapping from our sequence order to MPEG order
        themap = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19] #no map
        listlen = len(themap)-1

        res1    = {idx: results1[idx] for idx in seqrange}
        res2    = {idx: results2[idx] for idx in seqrange}

        # Reference and Tested
        x = 2
        y = 2

        rowsskipped = [0,0,0,0,0,0,0]

        for entity in ["Reference", "Tested"]:
            if entity == "Reference":
                offset  = 0
                res = res1
            elif entity == "Tested":
                offset  = 10
                res = res2
            seqnr = 0
            worksheet.write(y-2, x+offset+1, entity)
            worksheet.write(y-1, x+offset+0, "QP Slice")
            worksheet.write(y-1, x+offset+1, "kbps")
            worksheet.write(y-1, x+offset+2, "Y psnr")
            worksheet.write(y-1, x+offset+3, "U psnr")
            worksheet.write(y-1, x+offset+4, "V psnr")
            worksheet.write(y-1, x+offset+5, "YUV psnr")
            worksheet.write(y-1, x+offset+6, "Enc T [s]")
            worksheet.write(y-1, x+offset+7, "Dec T [s]")
            worksheet.write(y-1, x+offset+8, "Enc T [h]")

            for seq in seqrange:
             seqnr+=rowsskipped[seq]

             if(seqOK[seq]):
               if(seq>listlen):
                  thenum = seq;
               else:
                  thenum = themap[seq]
               numQps = len(res[seq][0])

               if entity == "Reference":
                  worksheet.write(y+seqnr, x-1, seqs[thenum].name)

               for qp in range(numQps):
                    mapqp = qp
                    thelen = len(simid[thenum][mapqp])
                    thetxt = simid[thenum][mapqp]
                    val = "%s%s" % (thetxt[thelen-5],thetxt[thelen-4])
                    if(val.isdigit()==0):
                        val = "%s" % (thetxt[thelen-4])

                    worksheet.write(y+qp+seqnr, x+offset+0, int(val))                                   # QP
                    worksheet.write(y+qp+seqnr, x+offset+1, round(res[thenum][0][mapqp],numdecimals))   # bitrate
                    worksheet.write(y+qp+seqnr, x+offset+2, round(res[thenum][5][mapqp],numdecimals))   # Y PSNR
                    worksheet.write(y+qp+seqnr, x+offset+3, round(res[thenum][6][mapqp],numdecimals))   # U PSNR
                    worksheet.write(y+qp+seqnr, x+offset+4, round(res[thenum][7][mapqp],numdecimals))   # V PSNR
                    worksheet.write(y+qp+seqnr, x+offset+5, round(res[thenum][8][mapqp],numdecimals))   # YUV PSNR
                    worksheet.write(y+qp+seqnr, x+offset+6, round(res[thenum][11][mapqp][0],2))         # enctime [s]
                    worksheet.write(y+qp+seqnr, x+offset+7, round(res[thenum][11][mapqp][0]/3600.0,2))  # enctime [h]
                    worksheet.write(y+qp+seqnr, x+offset+8, round(res[thenum][12][mapqp][0],2))         # dectime
               seqnr=seqnr+numQps

    def save(self):
        self.workbook.save(os.path.join(self.curpath, self.name))
