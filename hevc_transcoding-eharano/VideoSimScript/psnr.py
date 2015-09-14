import math
import os
import testSeqs

class snrcomp:
    def __init__(self, orgSeq, recFile, results, InternalBitDepth):
        self.original = orgSeq.getPath()
        self.reconstructed = recFile
        self.results = results
        self.InternalBitDepth = InternalBitDepth
        self.numFrames = orgSeq.framesOut
        self.width = orgSeq.width
        self.height = orgSeq.height
        self.orgStartFrame = orgSeq.startFrame
        self.orgFramesDropped = orgSeq.framesDropped
        self.bitdepth = orgSeq.bitdepth
        #assert orgSeq.chromaSubsampling == '420', "unknown chroma subsampling (%s)" % orgSeq.chromaSubsampling
        self.sf = 0;
        if orgSeq.chromaSubsampling == '420':
          self.sf = 1.5;
          self.Uwidth = self.width/2;
          self.Uheight = self.height/2;
          self.Vwidth = self.width/2;
          self.Vheight = self.height/2;
        else:
          if orgSeq.chromaSubsampling == '444':
            self.sf = 3;
            self.Uwidth = self.width;
            self.Uheight = self.height;
            self.Vwidth = self.width;
            self.Vheight = self.height;
          else:
            if orgSeq.chromaSubsampling == '422':
              self.sf = 2;
              self.Uwidth = self.width/2;
              self.Uheight = self.height;
              self.Vwidth = self.width/2;
              self.Vheight = self.height;
            else:
              assert self.sf != 0, "unknown chroma subsampling (%s)" % orgSeq.chromaSubsampling
        if self.InternalBitDepth>8:
          assert os.path.getsize(self.reconstructed) == 2*self.numFrames*self.width*self.height*self.sf, "'%s': unexpected file size (%d, %d)" % (self.reconstructed, os.path.getsize(self.reconstructed), 2*self.numFrames*self.width*self.height*self.sf)
        else:
          assert os.path.getsize(self.reconstructed) == self.numFrames*self.width*self.height*self.sf, "'%s': unexpected file size (%d, %d)" % (self.reconstructed, os.path.getsize(self.reconstructed), self.numFrames*self.width*self.height*self.sf)
        self.psnrY = {}
        self.psnrU = {}
        self.psnrV = {}
    def calculateFramePsnr(self,frame1, frame2, width, height):
        a = frame1
        b = frame2

        if self.InternalBitDepth>8:
          if self.bitdepth>8:
            assert len(frame1) == len(frame2), "samples are not of the same size"
          else:
            assert len(2*frame1) == len(frame2), "samples are not of the same size"
        else:
          if self.bitdepth>8:
            assert len(frame1) == len(2*frame2), "samples are not of the same size"
          else:
            assert len(frame1) == len(frame2), "samples are not of the same size"
        #print "length = %d" % len(frame1)
        ssd = 0
        if (self.bitdepth>8):
          thelength = len(a)/2
        else:
          thelength = len(a)
        for pos in xrange(thelength):
            if self.InternalBitDepth>8:
              tmpB = (ord(b[2*pos])+ord(b[2*pos+1])*256)
              if (self.bitdepth>8):
                tmpA = (ord(a[2*pos])+ord(a[2*pos+1])*256)
              else:
                tmpA = int(math.pow(2,self.InternalBitDepth-self.bitdepth))*ord(a[pos])
            else:
              if (self.bitdepth>8):
                tmpB =  ord(b[pos])
                tmpA = ((((ord(a[2*pos])+ord(a[2*pos+1])*256)) + int(math.pow(2,self.bitdepth-self.InternalBitDepth-1))) /int(math.pow(2,self.bitdepth-self.InternalBitDepth))) & 0xff
              else:
                tmpB = ord(b[pos])
                tmpA = ord(a[pos])
            tmp = tmpA - tmpB
            ssd = ssd + tmp*tmp
        #print "ssd = %d" % ssd

        if (ssd==0):
            return 1000

        mse = float(ssd) / float(thelength) #len(a))
        #print mse
        if (self.InternalBitDepth>8):
          psnr = float(10) * math.log(float(255*255*16)/mse, 10)
        else:
          psnr = float(10) * math.log(float(255*255)/mse, 10)

        return psnr

    def AvgPSNR(self):
        #open files
        f_original = open(self.original, "rb")
        f_rec = open(self.reconstructed, "rb")

        if self.bitdepth>8:
          orgFramesDropped = long((self.orgFramesDropped)*self.width*self.height*self.sf*2.0)
          orgOffset = long(self.orgStartFrame*self.width*self.height*self.sf*2.0)
        else:
          orgFramesDropped = long((self.orgFramesDropped)*self.width*self.height*self.sf)
          orgOffset = long(self.orgStartFrame*self.width*self.height*self.sf)
        f_original.seek(long(orgOffset))

        psnrY = 0
        psnrU = 0
        psnrV = 0

        if (self.bitdepth>8)&(self.InternalBitDepth>8):
          for i in range(self.numFrames):
            # Skip dropped frames
            if i > 0:
                f_original.seek(orgFramesDropped, os.SEEK_CUR)

            # PSNR Y
            frame1 = f_original.read(2*self.width*self.height)
            frame2 = f_rec.read(2*self.width*self.height)
            framePsnr = self.calculateFramePsnr(frame1, frame2, self.width, self.height)
            self.psnrY[i] = framePsnr
            psnrY = psnrY + framePsnr
            # PSNR U
            frame1 = f_original.read(2*self.Uwidth*self.Uheight)
            frame2 = f_rec.read(2*self.Uwidth*self.Uheight)
            framePsnr = self.calculateFramePsnr(frame1, frame2, self.Uwidth, self.Uheight)
            self.psnrU[i] = framePsnr
            psnrU = psnrU + framePsnr
            # PSNR V
            frame1 = f_original.read(2*self.Vwidth*self.Vheight)
            frame2 = f_rec.read(2*self.Vwidth*self.Vheight)
            framePsnr = self.calculateFramePsnr(frame1, frame2, self.Vwidth, self.Vheight)
            self.psnrV[i] = framePsnr
            psnrV = psnrV + framePsnr
        else:
          if self.bitdepth>8:
           for i in range(self.numFrames):
            # Skip dropped frames
            if i > 0:
                f_original.seek(orgFramesDropped, os.SEEK_CUR)

            # PSNR Y
            frame1 = f_original.read(2*self.width*self.height)
            frame2 = f_rec.read(self.width*self.height)
            framePsnr = self.calculateFramePsnr(frame1, frame2, self.width, self.height)
            self.psnrY[i] = framePsnr
            psnrY = psnrY + framePsnr
            # PSNR U
            frame1 = f_original.read(2*self.Uwidth*self.Uheight)
            frame2 = f_rec.read(self.Uwidth*self.Uheight)
            framePsnr = self.calculateFramePsnr(frame1, frame2, self.Uwidth, self.Uheight)
            self.psnrU[i] = framePsnr
            psnrU = psnrU + framePsnr
            # PSNR V
            frame1 = f_original.read(2*self.Vwidth*self.Vheight)
            frame2 = f_rec.read(self.Vwidth*self.Vheight)
            framePsnr = self.calculateFramePsnr(frame1, frame2, self.Vwidth, self.Vheight)
            self.psnrV[i] = framePsnr
            psnrV = psnrV + framePsnr
          else:
           if self.InternalBitDepth>8:
            for i in range(self.numFrames):
             # Skip dropped frames
             if i > 0:
                f_original.seek(orgFramesDropped, os.SEEK_CUR)

             # PSNR Y
             frame1 = f_original.read(self.width*self.height)
             frame2 = f_rec.read(2*self.width*self.height)
             framePsnr = self.calculateFramePsnr(frame1, frame2, self.width, self.height)
             self.psnrY[i] = framePsnr
             psnrY = psnrY + framePsnr
             # PSNR U
             frame1 = f_original.read(self.Uwidth*self.Uheight)
             frame2 = f_rec.read(2*self.Uwidth*self.Uheight)
             framePsnr = self.calculateFramePsnr(frame1, frame2, self.Uwidth, self.Uheight)
             self.psnrU[i] = framePsnr
             psnrU = psnrU + framePsnr
             # PSNR V
             frame1 = f_original.read(self.Vwidth*self.Vheight)
             frame2 = f_rec.read(2*self.Vwidth*self.Vheight)
             framePsnr = self.calculateFramePsnr(frame1, frame2, self.Vwidth, self.Vheight)
             self.psnrV[i] = framePsnr
             psnrV = psnrV + framePsnr
           else:
            for i in range(self.numFrames):
             # Skip dropped frames
             if i > 0:
                f_original.seek(orgFramesDropped, os.SEEK_CUR)

             # PSNR Y
             frame1 = f_original.read(self.width*self.height)
             frame2 = f_rec.read(self.width*self.height)
             framePsnr = self.calculateFramePsnr(frame1, frame2, self.width, self.height)
             self.psnrY[i] = framePsnr
             psnrY = psnrY + framePsnr
             # PSNR U
             frame1 = f_original.read(self.Uwidth*self.Uheight)
             frame2 = f_rec.read(self.Uwidth*self.Uheight)
             framePsnr = self.calculateFramePsnr(frame1, frame2, self.Uwidth, self.Uheight)
             self.psnrU[i] = framePsnr
             psnrU = psnrU + framePsnr
             # PSNR V
             frame1 = f_original.read(self.Vwidth*self.Vheight)
             frame2 = f_rec.read(self.Vwidth*self.Vheight)
             framePsnr = self.calculateFramePsnr(frame1, frame2, self.Vwidth, self.Vheight)
             self.psnrV[i] = framePsnr
             psnrV = psnrV + framePsnr
        f_original.close()
        f_rec.close()
        return psnrY/self.numFrames, psnrU/self.numFrames, psnrV/self.numFrames

    def AvgPSNR_Y(self):
        #open files
        f_original = open(self.original, "rb")
        f_rec = open(self.reconstructed, "rb")

        if self.bitdepth>8:
          orgOffset = self.orgStartFrame*self.width*self.height*self.sf*2.0
        else:
          orgOffset = self.orgStartFrame*self.width*self.height*self.sf
        psnr = 0
        for i in range(self.numFrames):
            if self.bitdepth>8:
              f_original.seek(long(orgOffset+(i*(self.orgFramesDropped+1)*self.width*self.height*self.sf*2.0)))
              frame1 = f_original.read(2*self.width*self.height)
            else:
              f_original.seek(long(orgOffset+(i*(self.orgFramesDropped+1)*self.width*self.height*self.sf)))
              frame1 = f_original.read(self.width*self.height)
            if self.InternalBitDepth>8:
              f_rec.seek(long(i*self.width*self.height*self.sf*2.0))
              frame2 = f_rec.read(2*self.width*self.height)
            else:
              f_rec.seek(long(i*self.width*self.height*self.sf))
              frame2 = f_rec.read(self.width*self.height)
            framePsnr = self.calculateFramePsnr(frame1, frame2, self.width, self.height)
            #print "frame %d Y PSNR %f" % (i, framePsnr)
            self.psnrY[i] = framePsnr
            psnr = psnr + framePsnr
        f_original.close()
        f_rec.close()
        psnr = psnr/self.numFrames
        return psnr


    def AvgPSNR_U(self):
        #open files
        f_original = open(self.original, "rb")
        f_rec = open(self.reconstructed, "rb")

        if self.bitdepth>8:
          orgOffset = self.orgStartFrame*self.width*self.height*self.sf*2.0
        else:
          orgOffset = self.orgStartFrame*self.width*self.height*self.sf
        psnr = 0
        for i in range(self.numFrames):
            if self.bitdepth>8:
              f_original.seek(long(orgOffset+(i*(self.orgFramesDropped+1)*self.width*self.height*self.sf*2.0)+(2*self.width*self.height)))
              frame1 = f_original.read(2*self.Uwidth*self.Uheight)
            else:
              f_original.seek(long(orgOffset+(i*(self.orgFramesDropped+1)*self.width*self.height*self.sf)+(self.width*self.height)))
              frame1 = f_original.read(self.Uwidth*self.Uheight)
            if self.InternalBitDepth>8:
              f_rec.seek(long((i*self.width*self.height*self.sf*2.0) + 2*self.width*self.height))
              frame2 = f_rec.read(2*self.Uwidth*self.Uheight)
            else:
              f_rec.seek(long((i*self.width*self.height*self.sf) + self.width*self.height))
              frame2 = f_rec.read(self.Uwidth*self.Uheight)
            framePsnr = self.calculateFramePsnr(frame1, frame2, self.width, self.height)
            #print "frame %d U PSNR %f" % (i, framePsnr)
            self.psnrU[i] = framePsnr
            psnr = psnr + framePsnr
        f_original.close()
        f_rec.close()
        psnr = psnr/self.numFrames
        return psnr


    def AvgPSNR_V(self):
        #open files
        f_original = open(self.original, "rb")
        f_rec = open(self.reconstructed, "rb")

        if self.bitdepth>8:
          orgOffset = self.orgStartFrame*self.width*self.height*self.sf*2.0
        else:
          orgOffset = self.orgStartFrame*self.width*self.height*self.sf
        psnr = 0
        for i in range(self.numFrames):
            if self.bitdepth>8:
              f_original.seek(long(orgOffset+(i*(self.orgFramesDropped+1)*self.width*self.height*self.sf*2.0)+(2*self.width*self.height+2*self.Uwidth*self.Uheight)))
              frame1 = f_original.read(2*self.Vwidth*self.Vheight)
            else:
              f_original.seek(long(orgOffset+(i*(self.orgFramesDropped+1)*self.width*self.height*self.sf)+(self.width*self.height+self.Uwidth*self.Uheight)))
              frame1 = f_original.read(self.Vwidth*self.Vheight)
            if self.InternalBitDepth>8:
              f_rec.seek(long((i*self.width*self.height*self.sf*2.0) + 2*self.width*self.height + 2*self.Uwidth*self.Uheight))
              frame2 = f_rec.read(2*self.Vwidth*self.Vheight)
            else:
              f_rec.seek(long((i*self.width*self.height*self.sf) + self.width*self.height + self.Uwidth*self.Uheight))
              frame2 = f_rec.read(self.Vwidth*self.Vheight)
            framePsnr = self.calculateFramePsnr(frame1, frame2, self.width, self.height)
            #print "frame %d V PSNR %f" % (i, framePsnr)
            self.psnrV[i] = framePsnr
            psnr = psnr + framePsnr
        f_original.close()
        f_rec.close()
        psnr = psnr/self.numFrames
        return psnr

    def MakePSNRFile(self, psnrY, psnrU, psnrV):
        #open file
        f_results = open(self.results, "w")
        for i in range(self.numFrames):
            text = "%d     %f %f %f\n" % (i, self.psnrY[i], self.psnrU[i], self.psnrV[i])
            f_results.write(text)
        text = "%s %f %f %f\n" % ("total", psnrY, psnrU, psnrV)
        f_results.write(text)
        f_results.close()

    def GetMaxPSNR(self, psnrVec):
        psnrMax = psnrVec[0]
        for i in range(len(psnrVec)):
            if(psnrVec[i]>psnrMax):
                psnrMax = psnrVec[i]
        return psnrMax

    def GetMinPSNR(self, psnrVec):
        psnrMin = psnrVec[0]
        for i in range(len(psnrVec)):
            if(psnrVec[i]<psnrMin):
                psnrMin = psnrVec[i]
        return psnrMin
