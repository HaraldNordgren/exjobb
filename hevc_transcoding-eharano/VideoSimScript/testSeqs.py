import os, platform, posixpath, math, copy

""" testSeqs - video test sequence definitions

>>> Usage examples:

1) Sequence attributes and test set definitions should be added in this file (see section "sequence definitions" below)

2a) Obtaining sequence attributes based on identifier, and instantiating sequence objects
import testSeqs
a = testSeqs.seqAttr['ForemanQCIF15']
s = testSeqs.testSeq(*a)                                     # '*' converts list 'a' into a list of arguments
print "%s %dx%d@%d" % (s.name, s.width, s.height, s.fpsOut)

2b) Use of test sets
import testSeqs
seqs = []
for a in testSeqs.vcegCIF:                  # example: official VCEG test set with CIF sequences
    seqs.append(testSeqs.testSeq(*a))
for s in seqs:
    print "%s %dx%d@%d" % (s.name, s.width, s.height, s.fpsOut)

3a) Sequence attributes
import testSeqs
s = testSeqs.testSeq(*testSeqs.seqAttr['ForemanQCIF15'])
print s.name                # sequence identifier
print s.getPath()           # get path to sequence in format of the machine the script runs on
print s.getPath('linux')    # get path to sequence in linux format
print s.getPath('windows')  # get path to sequence in windows format
print s.width               # frame width
print s.height              # frame height
print s.fpsIn               # fps of sequence file
print s.fpsOut              # actually used fps
print s.framesDropped       # number of frames dropped between two used frames
print s.chromaSubsampling   # chroma subsampling
print s.startFrame          # first frame used in sequence file
print s.framesIn            # number of frames used in sequence file (considering fpsIn)
print s.framesOut           # number of frames used in sequence file (considering fpsOut)
print s.getAttributes()     # list all attributes that are required to instantiate a new sequence object

3b) Passing attributes to another sequence object
import testSeqs
s = testSeqs.testSeq(*testSeqs.seqAttr['ForemanQCIF15'])
print "%s %dx%d@%d" % (s.name, s.width, s.height, s.fpsOut)
a = s.getAttributes()       # this could e.g. be written into simulation configuration file
s2 = testSeqs.testSeq(*a)   # instantiation of sequence based on given attributes
print "%s %dx%d@%d" % (s2.name, s2.width, s2.height, s2.fpsOut)

4) Changing the root path
import testSeqs
class myLocalTestSeq(testSeqs.testSeq):
    rootPathWinDefault = r'C:\data\org'
    rootPathLinDefault = None
s = myLocalTestSeq(*testSeqs.seqAttr['ForemanQCIF15'])
print s.getPath()

"""

### General Definitions ###
###########################

seqAttr = {}

def addSeqAttr(attr):
    seqAttr[attr[0]] = attr

class testSeq:
    rootPathWinDefault = r'\\vhub.rnd.ki.sw.ericsson.se\proj\video_data\org'
    rootPathLinDefault = r'/proj/video_data/org'
    #rootPathWinDefault = r'E:\Sequences'

    def __init__(self, uniqueName, relPath, width, height, fpsIn, fpsOut=None, startFrame=0, framesIn=None, chromaSubsampling='420', rootPathWin=None, rootPathLin=None, bitdepth=8):
        self.name    = uniqueName
        self.relPath = os.path.expandvars(relPath)
        self.width   = width
        self.height  = height
        self.fpsIn   = fpsIn
        if not fpsOut:
            self.fpsOut = fpsIn
        else:
            assert not (self.fpsIn % fpsOut), "fpsIn must be a multiple of fpsOut (%f, %f)" % (self.fpsIn, fpsOut)
            self.fpsOut  = fpsOut
        self.framesDropped = int(self.fpsIn / self.fpsOut) - 1
        self.chromaSubsampling = chromaSubsampling.replace(':','')
        self.bitdepth = bitdepth
        if self.chromaSubsampling == '420':
            assert not (self.width % 2), "unexpected width (%d)" % self.width
            assert not (self.height % 2), "unexpected height (%d)" % self.height
            if self.bitdepth>8:
              self.frameSize = self.width * self.height * 1.5 * 2.0
            else:
              self.frameSize = self.width * self.height * 1.5
        else:
            if self.chromaSubsampling == '444':
              if self.bitdepth>8:
                self.frameSize = self.width * self.height * 3 * 2.0
              else:
                self.frameSize = self.width * self.height * 3
            else:
              assert False, "unknown chroma subsampling (%s)" % self.chromaSubsampling
        if rootPathWin:
            self.rootPathWin = os.path.expandvars(rootPathWin)
        else:
            self.rootPathWin = self.rootPathWinDefault
        if rootPathLin:
            self.rootPathLin = os.path.expandvars(rootPathLin)
        else:
            self.rootPathLin = self.rootPathLinDefault
        if self.getPath().endswith('exr') or self.getPath().endswith('tiff') or self.getPath().endswith('tif'):
            d = os.path.dirname(self.getPath())
            if os.path.isdir(d):
                self.frameSize = 0
                self.fileSize = 0
                frameCnt = 0
                for f in os.listdir(d):
                    if f.endswith('exr'):
                        if frameCnt < framesIn:
                            frameCnt += 1
                            self.frameSize = os.path.getsize(d+os.sep+f)
                            self.fileSize += self.frameSize
                    if f.endswith('tiff') or f.endswith('tif'):
                        frameCnt += 1
                        if not self.frameSize:
                            print d+os.sep+f
                            self.frameSize = os.path.getsize(d+os.sep+f)
                if not self.fileSize:
                    self.fileSize = self.frameSize * frameCnt

                assert not (self.fileSize % self.frameSize), "file size does not match frame dimensions"
                self.maxFrames = int(self.fileSize / self.frameSize)
                assert startFrame < self.maxFrames, "startFrame too high (%d, %d)" % (startFrame, self.maxFrames)
            else:
                print "WARNING: cannot find exr or tiff files in '%s'" % os.path.dirname(self.getPath())
                self.fileSize = None
                self.maxFrames = None
        elif os.path.isfile(self.getPath()):
            self.fileSize  = os.path.getsize(self.getPath())
            assert not (self.fileSize % self.frameSize), "file size does not match frame dimensions"
            self.maxFrames = int(self.fileSize / self.frameSize)
            assert startFrame < self.maxFrames, "startFrame too high (%d, %d)" % (startFrame, self.maxFrames)
        else:
            print "WARNING: cannot find yuv file '%s'" % self.getPath()
            self.fileSize = None
            self.maxFrames = None
        self.startFrame = startFrame
        if framesIn:
            assert self.maxFrames is None or startFrame + framesIn <= self.maxFrames, "number of frames too high (%d, %d, %d)" % (startFrame, framesIn, self.maxFrames)
            self.framesIn = framesIn
        else:
            if self.maxFrames is not None:
                self.framesIn = self.maxFrames - self.startFrame
            else:
                self.framesIn = None
        if self.framesIn is not None:
            self.framesOut = int(math.ceil(float(self.framesIn * self.fpsOut) / float(self.fpsIn)))
        else:
            self.framesOut = None
        self.fileSizeOut = self.frameSize * self.framesOut
        if self.framesIn is None or self.framesOut is None or self.maxFrames is None:
            print "initializing test sequence '%s' (simple): %dx%d@%dHz, %d-? (%d framesDropped)" % (self.name, self.width, self.height, self.fpsOut, self.startFrame, self.framesDropped)
        else:
            print "initializing test sequence '%s': %dx%d@%dHz, %d-%d (%d framesIn (max %d), %d framesOut, %d framesDropped)" % (self.name, self.width, self.height, self.fpsOut, self.startFrame, self.startFrame + self.framesIn-1, self.framesIn, self.maxFrames, self.framesOut, self.framesDropped)

    def setFramesOut(self, framesOut):
        """Set the number of frames that are actually used for encoding."""
        assert framesOut == int(framesOut), "framesOut must be integer"
        assert not (self.fpsIn % self.fpsOut), "fpsIn must be a multiple of fpsOut (%f, %f)" % (self.fpsIn, self.fpsOut)
        framesIn = int(self.fpsIn / self.fpsOut) * int(framesOut - 1) + 1

        assert self.startFrame + framesIn <= self.maxFrames, "number of frames too high (%d, %d, %d)" % (self.startFrame, framesIn, self.maxFrames)
        self.framesIn = framesIn
        self.framesOut = int(framesOut)

    def setFpsOut(self, fpsOut):
        """Set output frame rate."""
        assert not (self.fpsIn % fpsOut), "fpsIn must be a multiple of fpsOut (%f, %f)" % (self.fpsIn, fpsOut)
        self.fpsOut = fpsOut
        self.framesDropped = int(self.fpsIn / self.fpsOut) - 1
        self.framesOut = int(math.ceil(float(self.framesIn * self.fpsOut) / float(self.fpsIn)))
        self.fileSizeOut = self.frameSize * self.framesOut

    def __eq__(self, other):
        if self.name               != other.name:               return False
        if self.relPath            != other.relPath:            return False
        if self.width              != other.width:              return False
        if self.height             != other.height:             return False
        if self.fpsIn              != other.fpsIn:              return False
        if self.fpsOut             != other.fpsOut:             return False
        if self.framesDropped      != other.framesDropped:      return False
        if self.chromaSubsampling  != other.chromaSubsampling:  return False
        if self.frameSize          != other.frameSize:          return False
        if self.rootPathWin        != other.rootPathWin:        return False
        if self.rootPathLin        != other.rootPathLin:        return False
        if self.rootPathWinDefault != other.rootPathWinDefault: return False
        if self.rootPathLinDefault != other.rootPathLinDefault: return False
        if self.fileSize           != other.fileSize:           return False
        if self.fileSizeOut        != other.fileSizeOut:        return False
        if self.maxFrames          != other.maxFrames:          return False
        if self.startFrame         != other.startFrame:         return False
        if self.framesIn           != other.framesIn:           return False
        if self.framesOut          != other.framesOut:          return False
        if self.bitdepth           != other.bitdepth:           return False
        return True

    def equal(self, other, ignoreName=True, ignoreYuv=True):
        if self.width              != other.width:              return False
        if self.height             != other.height:             return False
        if self.fpsIn              != other.fpsIn:              return False
        if self.fpsOut             != other.fpsOut:             return False
        if self.framesDropped      != other.framesDropped:      return False
        if self.chromaSubsampling  != other.chromaSubsampling:  return False
        if self.frameSize          != other.frameSize:          return False
        if self.fileSizeOut        != other.fileSizeOut:        return False
        if self.startFrame         != other.startFrame:         return False
        if self.framesIn           != other.framesIn:           return False
        if self.framesOut          != other.framesOut:          return False
        if self.bitdepth           != other.bitdepth:           return False
        if not ignoreName:
            if self.name               != other.name:               return False
        if not ignoreYuv:
            if self.relPath            != other.relPath:            return False
            if self.rootPathWin        != other.rootPathWin:        return False
            if self.rootPathLin        != other.rootPathLin:        return False
            if self.rootPathWinDefault != other.rootPathWinDefault: return False
            if self.rootPathLinDefault != other.rootPathLinDefault: return False
            if self.fileSize           != other.fileSize:           return False
            if self.maxFrames          != other.maxFrames:          return False
        return True

    def copy(self, uniqueName=None, relPath=None, rootPathWin=None, rootPathLin=None, resChange=None):
        ret = copy.deepcopy(self)
        if uniqueName is not None:
            ret.name = uniqueName
        if relPath is not None:
            ret.relPath = relPath
        if rootPathWin is not None:
            ret.rootPathWin = rootPathWin
        if rootPathLin is not None:
            ret.rootPathLin = rootPathLin
        if resChange is not None:
            assert len(resChange) == 2
            ret.width       = int(round( ret.width       * resChange[0] ))
            ret.height      = int(round( ret.height      * resChange[1] ))
            ret.frameSize   = int(round( ret.frameSize   * resChange[0] * resChange[1] ))
            ret.fileSize    = int(round( ret.fileSize    * resChange[0] * resChange[1] ))
            ret.fileSizeOut = int(round( ret.fileSizeOut * resChange[0] * resChange[1] ))
        return ret

    def getPath(self, system=None, relPath=None):
        if not system:
            system = platform.system()
        system = system.lower()
        if not relPath:
            relPath = self.relPath
        if (system == 'linux') or (system == 'sunos') or (system == 'solaris'):
            assert self.rootPathLin, "sequence root path for linux unavailable"
            return posixpath.join(self.rootPathLin, relPath)
        elif (system == 'windows') or (system == 'microsoft'):
            assert self.rootPathWin, "sequence root path for windows unavailable"
            return os.path.join(self.rootPathWin, relPath)
        else:
            assert False, "unknown platform: " + system

    def getAttributes(self):
        return [ self.name, self.relPath, self.width, self.height, self.fpsIn, self.fpsOut, self.startFrame, self.framesIn, self.chromaSubsampling, self.rootPathWin, self.rootPathLin, self.bitdepth ]


######################################################################################################################

### sequence attributes
#######################
# format: identifier, relative path (relative to root path (/proj/video_data)), width, height, fpsIn, (fpsOut, (startFrame, (numFramesIn, (chromaSubsampling, (rootPathWin, (rootPathLin))))))
#         use right slash ('/') in relative path to make sure it works on both Windows and Linux

addSeqAttr(['ContainerQCIF15', 'vceg/container.yuv', 176, 144, 30, 15, 0, 300])
addSeqAttr(['ForemanQCIF15',   'vceg/foremanq.yuv',  176, 144, 30, 15, 0, 300])
addSeqAttr(['SilentQCIF15',    'vceg/silent.yuv',    176, 144, 30, 15, 0, 300])

addSeqAttr(['ForemanQCIF30',   'vceg/foremanq.yuv',  176, 144, 30, 30, 0, 300])
addSeqAttr(['ForemanQCIF7.5',  'vceg/foremanq.yuv',  176, 144, 30, 7.5, 0, 300])

addSeqAttr(['dcgrotCIF',   'synthetic/dcgrotCIF.yuv',   352, 288, 30, 30, 0, 10])

addSeqAttr(['ParisCIF30',   'vceg/paris.yuv',   352, 288, 30, 30, 0, 300])
addSeqAttr(['ForemanCIF30', 'vceg/foreman.yuv', 352, 288, 30, 30, 0, 300])
addSeqAttr(['MobileCIF30',  'vceg/mobile.yuv',  352, 288, 30, 30, 0, 300])
addSeqAttr(['TempeteCIF30', 'vceg/tempete.yuv', 352, 288, 30, 30, 0, 260])

addSeqAttr(['nuts3_416x240_30',   'vceg/nuts3_416x240_30.yuv',  416, 240, 30, 30, 0, 300])
addSeqAttr(['nuts5_416x240_30',   'vceg/nuts5_416x240_30.yuv',  416, 240, 30, 30, 0, 300])
addSeqAttr(['flower4_416x240_30',   'vceg/flower4_416x240_30.yuv',  416, 240, 30, 30, 0, 300])
addSeqAttr(['keiba3_416x240_30',   'vceg/keiba3_416x240_30.yuv',  416, 240, 30, 30, 0, 300])

addSeqAttr(['nuts3_832x480_30',   'vceg/nuts3_832x480_30.yuv',  832, 480, 30, 30, 0, 300])
addSeqAttr(['nuts5_832x480_30',   'vceg/nuts5_832x480_30.yuv',  832, 480, 30, 30, 0, 300])
addSeqAttr(['flower4_832x480_30',   'vceg/flower4_832x480_30.yuv',  832, 480, 30, 30, 0, 300])
addSeqAttr(['keiba3_832x480_30',   'vceg/keiba3_832x480_30.yuv',  832, 480, 30, 30, 0, 300])

addSeqAttr(['Janine1_8SVGA25',     'vceg/Janine1_8.yuv',     1024, 768, 25, 25, 0, 150])

addSeqAttr(['RSM_ABC_Football720p60',     'tandberg/720p/RSM_ABC_Football.yuv',     1280, 720, 60, 60, 0, 150])
addSeqAttr(['RSM_American_Football_720p60',     'tandberg/720p/RSM_American_Football_720.yuv',     1280, 720, 60, 60, 0, 150])
addSeqAttr(['RSM_Confetti720p60',     'tandberg/720p/RSM_Confetti.yuv',     1280, 720, 60, 60, 0, 150])
addSeqAttr(['RSM_Fairground720p60',     'tandberg/720p/RSM_Fairground.yuv',     1280, 720, 60, 60, 0, 150])
addSeqAttr(['RSM_Horse_Racing720p60',     'tandberg/720p/RSM_Horse_Racing.yuv',     1280, 720, 60, 60, 0, 150])
addSeqAttr(['RSM_Pop_Idol720p60',     'tandberg/720p/RSM_Pop_Idol.yuv',     1280, 720, 60, 60, 0, 150])

addSeqAttr(['BigShips720p60',     'vceg/BigShips.yuv',     1280, 720, 60, 60, 0, 150])
addSeqAttr(['City720p60',         'vceg/City.yuv',         1280, 720, 60, 60, 0, 150])
addSeqAttr(['Crew720p60',         'vceg/Crew.yuv',         1280, 720, 60, 60, 0, 150])
addSeqAttr(['Night720p60',        'vceg/Night.yuv',        1280, 720, 60, 60, 0, 150])
addSeqAttr(['Raven720p60',        'vceg/Raven.yuv',        1280, 720, 60, 60, 0, 150])
addSeqAttr(['Jets720p60',         'vceg/Jets.yuv',         1280, 720, 60, 60, 300, 150])
#addSeqAttr(['ShuttleStart720p60', 'vceg/ShuttleStart.yuv', 1280, 720, 60, 300, 150])

addSeqAttr(['Vidyo1_720p60',        'vceg/vidyo1_720p_60.yuv', 1280, 720, 60, 60, 0, 600])
addSeqAttr(['Vidyo2_720p60',        'vceg/vidyo2_720p_60.yuv', 1280, 720, 60, 60, 0, 600])
addSeqAttr(['Vidyo3_720p60',        'vceg/vidyo3_720p_60.yuv', 1280, 720, 60, 60, 0, 600])
addSeqAttr(['Vidyo4_720p60',        'vceg/vidyo4_720p_60.yuv', 1280, 720, 60, 60, 0, 600])
addSeqAttr(['Vidyo1_1280x720_30',   'svc_vt/vidyo1_1280x720_30.yuv', 1280, 720, 30, 30, 0, 300])
addSeqAttr(['Vidyo2_1280x720_30',   'svc_vt/vidyo2_1280x720_30.yuv', 1280, 720, 30, 30, 0, 300])
addSeqAttr(['Vidyo3_1280x720_30',   'svc_vt/vidyo3_1280x720_30.yuv', 1280, 720, 30, 30, 0, 300])
addSeqAttr(['Vidyo4_1280x720_30',   'svc_vt/vidyo4_1280x720_30.yuv', 1280, 720, 30, 30, 0, 300])
addSeqAttr(['Vidyo1_640x360_30',    'svc_vt/vidyo1_640x360_30.yuv', 640, 360, 30, 30, 0, 300])
addSeqAttr(['Vidyo2_640x360_30',    'svc_vt/vidyo2_640x360_30.yuv', 640, 360, 30, 30, 0, 300])
addSeqAttr(['Vidyo3_640x360_30',    'svc_vt/vidyo3_640x360_30.yuv', 640, 360, 30, 30, 0, 300])
addSeqAttr(['Vidyo4_640x360_30',    'svc_vt/vidyo4_640x360_30.yuv', 640, 360, 30, 30, 0, 300])
addSeqAttr(['Vidyo1_320x180_30',    'svc_vt/vidyo1_320x180_30.yuv', 320, 180, 30, 30, 0, 300])
addSeqAttr(['Vidyo2_320x180_30',    'svc_vt/vidyo2_320x180_30.yuv', 320, 180, 30, 30, 0, 300])
addSeqAttr(['Vidyo3_320x180_30',    'svc_vt/vidyo3_320x180_30.yuv', 320, 180, 30, 30, 0, 300])
addSeqAttr(['Vidyo4_320x180_30',    'svc_vt/vidyo4_320x180_30.yuv', 320, 180, 30, 30, 0, 300])
addSeqAttr(['Vidyo1_320x180_15',    'svc_vt/vidyo1_320x180_15.yuv', 320, 180, 15, 15, 0, 150])
addSeqAttr(['Vidyo2_320x180_15',    'svc_vt/vidyo2_320x180_15.yuv', 320, 180, 15, 15, 0, 150])
addSeqAttr(['Vidyo3_320x180_15',    'svc_vt/vidyo3_320x180_15.yuv', 320, 180, 15, 15, 0, 150])
addSeqAttr(['Vidyo4_320x180_15',    'svc_vt/vidyo4_320x180_15.yuv', 320, 180, 15, 15, 0, 150])
addSeqAttr(['Vidyo1_256x144_30',    'svc_vt/vidyo1_256x144_30.yuv', 256, 144, 30, 30, 0, 300])
addSeqAttr(['Vidyo2_256x144_30',    'svc_vt/vidyo2_256x144_30.yuv', 256, 144, 30, 30, 0, 300])
addSeqAttr(['Vidyo3_256x144_30',    'svc_vt/vidyo3_256x144_30.yuv', 256, 144, 30, 30, 0, 300])
addSeqAttr(['Vidyo4_256x144_30',    'svc_vt/vidyo4_256x144_30.yuv', 256, 144, 30, 30, 0, 300])
addSeqAttr(['Vidyo1_176x144_30',    'svc_vt/vidyo1_176x144_30.yuv', 176, 144, 30, 30, 0, 300])
addSeqAttr(['Vidyo2_176x144_30',    'svc_vt/vidyo2_176x144_30.yuv', 176, 144, 30, 30, 0, 300])
addSeqAttr(['Vidyo3_176x144_30',    'svc_vt/vidyo3_176x144_30.yuv', 176, 144, 30, 30, 0, 300])
addSeqAttr(['Vidyo4_176x144_30',    'svc_vt/vidyo4_176x144_30.yuv', 176, 144, 30, 30, 0, 300])

#new vidyo sequence
addSeqAttr(['Johnny_1280x720_60',        'jctvc/Johnny_1280x720_60.yuv', 1280, 720, 60, 60, 0, 600])
addSeqAttr(['KristenAndSara_1280x720_60',        'jctvc/KristenAndSara_1280x720_60.yuv', 1280, 720, 60, 60, 0, 600])
addSeqAttr(['FourPeople_1280x720_60',        'jctvc/FourPeople_1280x720_60.yuv', 1280, 720, 60, 60, 0, 600])

# deblocking improvements
addSeqAttr(['Vidyo3_LDLC_720p60',        'vceg/vidyo3_720p_60.yuv', 1280, 720, 60, 60, 0, 600])
addSeqAttr(['Vidyo4_LDLC_720p60',        'vceg/vidyo4_720p_60.yuv', 1280, 720, 60, 60, 0, 600])
addSeqAttr(['Vidyo3_LDHE_720p60',        'vceg/vidyo3_720p_60.yuv', 1280, 720, 60, 60, 0, 600])
addSeqAttr(['Vidyo4_LDHE_720p60',        'vceg/vidyo4_720p_60.yuv', 1280, 720, 60, 60, 0, 600])
addSeqAttr(['BQMall_RAHE_832x480_60', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BQMall_832x480_60.yuv',   832, 480, 60, 60, 0, 600])
addSeqAttr(['BasketballDrill_RAHE_832x480_50', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BasketballDrill_832x480_50.yuv',   832, 480, 50, 50, 0, 500])
addSeqAttr(['ParkScene_RAHE_1920x1080_24', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/ParkScene_1920x1080_24.yuv',   1920, 1080, 24, 24, 0, 240])
addSeqAttr(['Kimono1_RAHE_1920x1080_24',     'mpeg/orig/Kimono1_1920x1080_24.yuv',     1920, 1080, 24, 24, 0, 240])
addSeqAttr(['BQMall_LDLC_832x480_60', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BQMall_832x480_60.yuv',   832, 480, 60, 60, 0, 600])
addSeqAttr(['BasketballDrill_LDLC_832x480_50', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BasketballDrill_832x480_50.yuv',   832, 480, 50, 50, 0, 500])
addSeqAttr(['ParkScene_LDLC_1920x1080_24', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/ParkScene_1920x1080_24.yuv',   1920, 1080, 24, 24, 0, 240])
addSeqAttr(['Kimono1_LDLC_1920x1080_24',     'mpeg/orig/Kimono1_1920x1080_24.yuv',     1920, 1080, 24, 24, 0, 240])
addSeqAttr(['PartyScene_RAHE_832x480_50', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/PartyScene_832x480_50.yuv',   832, 480, 50, 50, 0, 500])
addSeqAttr(['PartyScene_LDLC_832x480_50', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/PartyScene_832x480_50.yuv',   832, 480, 50, 50, 0, 500])

# additional sequences for SVC video conferencing tests
addSeqAttr(['Pico1_1280x720_25',        'svc_conf/pico1_1280x720_25.yuv',        1280,  720, 25, 25, 0, 250])
addSeqAttr(['Pico1_640x360_25',         'svc_conf/pico1_640x360_25.yuv',          640,  360, 25, 25, 0, 250])
addSeqAttr(['Pico1_256x144_25',         'svc_conf/pico1_256x144_25.yuv',          256,  144, 25, 25, 0, 250])
addSeqAttr(['Pico1_176x144_25',         'svc_conf/pico1_176x144_25.yuv',          176,  144, 25, 25, 0, 250])
addSeqAttr(['Pico2_1280x720_25',        'svc_conf/pico2_1280x720_25.yuv',        1280,  720, 25, 25, 0, 250])
addSeqAttr(['Pico2_640x360_25',         'svc_conf/pico2_640x360_25.yuv',          640,  360, 25, 25, 0, 250])
addSeqAttr(['Pico2_256x144_25',         'svc_conf/pico2_256x144_25.yuv',          256,  144, 25, 25, 0, 250])
addSeqAttr(['Pico2_176x144_25',         'svc_conf/pico2_176x144_25.yuv',          176,  144, 25, 25, 0, 250])
addSeqAttr(['c1mroom08_1920x1080_25',   'svc_conf/c1mroom08_1920x1080_25.yuv',   1920, 1080, 25, 25, 0, 250])
addSeqAttr(['c1mroom08_1280x720_25',    'svc_conf/c1mroom08_1280x720_25.yuv',    1280,  720, 25, 25, 0, 250])
addSeqAttr(['c1mroom08_640x360_25',     'svc_conf/c1mroom08_640x360_25.yuv',      640,  360, 25, 25, 0, 250])
addSeqAttr(['c1mroom08_256x144_25',     'svc_conf/c1mroom08_256x144_25.yuv',      256,  144, 25, 25, 0, 250])
addSeqAttr(['c1mroom08_176x144_25',     'svc_conf/c1mroom08_176x144_25.yuv',      176,  144, 25, 25, 0, 250])
addSeqAttr(['c2hoffice04_1920x1080_25', 'svc_conf/c2hoffice04_1920x1080_25.yuv', 1920, 1080, 25, 25, 0, 250])
addSeqAttr(['c2hoffice04_1280x720_25',  'svc_conf/c2hoffice04_1280x720_25.yuv',  1280,  720, 25, 25, 0, 250])
addSeqAttr(['c2hoffice04_640x360_25',   'svc_conf/c2hoffice04_640x360_25.yuv',    640,  360, 25, 25, 0, 250])
addSeqAttr(['c2hoffice04_256x144_25',   'svc_conf/c2hoffice04_256x144_25.yuv',    256,  144, 25, 25, 0, 250])
addSeqAttr(['c2hoffice04_176x144_25',   'svc_conf/c2hoffice04_176x144_25.yuv',    176,  144, 25, 25, 0, 250])
addSeqAttr(['c2office04_1920x1080_25',  'svc_conf/c2office04_1920x1080_25.yuv',  1920, 1080, 25, 25, 0, 250])
addSeqAttr(['c2office04_1280x720_25',   'svc_conf/c2office04_1280x720_25.yuv',   1280,  720, 25, 25, 0, 250])
addSeqAttr(['c2office04_640x360_25',    'svc_conf/c2office04_640x360_25.yuv',     640,  360, 25, 25, 0, 250])
addSeqAttr(['c2office04_256x144_25',    'svc_conf/c2office04_256x144_25.yuv',     256,  144, 25, 25, 0, 250])
addSeqAttr(['c2office04_176x144_25',    'svc_conf/c2office04_176x144_25.yuv',     176,  144, 25, 25, 0, 250])

addSeqAttr(['CrowdRun1080p60',     'vceg/crowdrun_1920x1080_50Hz_i420_8.yuv',     1920, 1080, 50, 50, 0, 125])
addSeqAttr(['ParkJoy1080p60',     'vceg/parkjoy_1920x1080_50Hz_i420_8.yuv',     1920, 1080, 50, 50, 0, 125])
addSeqAttr(['sunflower1080p25',     'vceg/sunflower.yuv',     1920, 1080, 25, 25, 0, 125])
addSeqAttr(['toys_and_calendar1080p25',     'vceg/toys_and_calendar.yuv',     1920, 1080, 25, 25, 0, 125])
addSeqAttr(['Traffic_1920x1080_p30',     'vceg/Traffic_1920x1080_30.yuv',     1920, 1080, 30, 30, 0, 125])

addSeqAttr(['Kimono1_1080p24',     'mpeg/orig/Kimono1_1920x1080_24.yuv',     1920, 1080, 24, 24, 0, 240])
addSeqAttr(['ParkScene_1080p24',     'mpeg/orig/ParkScene_1920x1080_24.yuv',     1920, 1080, 24, 24, 0, 240])
addSeqAttr(['Tennis_1080p24',     'mpeg/orig/Tennis_1920x1080_24.yuv',     1920, 1080, 24, 24, 0, 240])

addSeqAttr(['ParkJoy_2560x1600_crop_p50',     'mpeg/orig/ParkJoy_2560x1600_50_crop.yuv',     2560, 1600, 50, 50, 0, 500])
addSeqAttr(['PeopleOnStreet_2560x1600_crop_p30',     'mpeg/orig/PeopleOnStreet_2560x1600_30_crop.yuv',     2560, 1600, 30, 30, 0, 150])
addSeqAttr(['Traffic_2560x1600_crop_p30',     'mpeg/orig/Traffic_2560x1600_30_crop.yuv',     2560, 1600, 30, 30, 0, 300])


addSeqAttr(['CrowdRunVGA25','svc_vt/640x480@25_CrowdRun.yuv',  640, 480, 25, 25, 0, 250])
addSeqAttr(['SeekingVGA25', 'svc_vt/640x480@25_Seeking.yuv',   640, 480, 25, 25, 0, 250])
addSeqAttr(['CrewVGA30',    'svc_vt/640x480@30_Crew.yuv',      640, 480, 30, 30, 0, 300])
addSeqAttr(['SoccerVGA30',  'svc_vt/640x480@30_Soccer.yuv',    640, 480, 30, 30, 0, 300])

addSeqAttr(['CrowdRunQVGA12.5','svc_vt/320x240@12.5_CrowdRun.yuv', 320, 240, 12.5, 12.5, 0, 125])
addSeqAttr(['SeekingQVGA12.5', 'svc_vt/320x240@12.5_Seeking.yuv',  320, 240, 12.5, 12.5, 0, 125])
addSeqAttr(['CrewQVGA15',      'svc_vt/320x240@15_Crew.yuv',       320, 240, 15, 15, 0, 150])
addSeqAttr(['SoccerQVGA15',    'svc_vt/320x240@15_Soccer.yuv',     320, 240, 15, 15, 0, 150])

addSeqAttr(['CrowdRunQVGA6.25', 'svc_vt/320x240@12.5_CrowdRun.yuv', 320, 240, 12.5, 6.25, 0, 125])
addSeqAttr(['SeekingQVGA6.25',  'svc_vt/320x240@12.5_Seeking.yuv',  320, 240, 12.5, 6.25, 0, 125])
addSeqAttr(['CrewQVGA7.5',      'svc_vt/320x240@15_Crew.yuv',       320, 240, 15, 7.5, 0, 150])
addSeqAttr(['SoccerQVGA7.5',    'svc_vt/320x240@15_Soccer.yuv',     320, 240, 15, 7.5, 0, 150])

addSeqAttr(['AlohaWave720p50', 'svc_vt/720p50_AlohaWave.yuv', 1280, 720, 50, 50, 0, 500])
addSeqAttr(['CrowdRun720p50',  'svc_vt/720p50_CrowdRun.yuv',  1280, 720, 50, 50, 0, 500])
addSeqAttr(['Seeking720p50',   'svc_vt/720p50_Seeking.yuv',   1280, 720, 50, 50, 0, 500])
addSeqAttr(['Umbrella720p50',  'svc_vt/720p50_Umbrella.yuv',  1280, 720, 50, 50, 0, 500])

addSeqAttr(['AlohaWave1080p50', 'svc_vt/1080p50_AlohaWave.yuv', 1920, 1080, 50, 50, 0, 500])
addSeqAttr(['CrowdRun1080p50',  'svc_vt/1080p50_CrowdRun.yuv',  1920, 1080, 50, 50, 0, 500])
addSeqAttr(['Seeking1080p50',   'svc_vt/1080p50_Seeking.yuv',   1920, 1080, 50, 50, 0, 500])
addSeqAttr(['Umbrella1080p50',  'svc_vt/1080p50_Umbrella.yuv',  1920, 1080, 50, 50, 0, 500])

addSeqAttr(['City4CIF30','svc/CITY_704x576_30_orig_01.yuv', 704, 576, 30, 30, 0, 300])
addSeqAttr(['CityCIF15', 'svc/CITY_352x288_15_orig_01.yuv', 352, 288, 15, 15, 0, 150])
addSeqAttr(['Ice4CIF30', 'svc/ICE_704x576_30_orig_02.yuv',  704, 576, 30, 30, 0, 240])
addSeqAttr(['IceCIF15',  'svc/ICE_352x288_15_orig_02.yuv',  352, 288, 15, 15, 0, 120])

addSeqAttr(['BigShips720p60-Every8th',     'every8th/BigShips-Every8th.yuv',     1280, 720, 60, 60, 0, 19])
addSeqAttr(['City720p60-Every8th',         'every8th/City-Every8th.yuv',         1280, 720, 60, 60, 0, 19])
addSeqAttr(['Crew720p60-Every8th',         'every8th/Crew-Every8th.yuv',         1280, 720, 60, 60, 0, 19])
addSeqAttr(['Night720p60-Every8th',        'every8th/Night-Every8th.yuv',        1280, 720, 60, 60, 0, 19])
addSeqAttr(['Raven720p60-Every8th',        'every8th/Raven-Every8th.yuv',        1280, 720, 60, 60, 0, 19])
addSeqAttr(['nuts3_416x240_30-Every8th',   'every8th/nuts3_416x240_30-Every8th.yuv',  416, 240, 30, 30, 0, 38])
addSeqAttr(['nuts5_416x240_30-Every8th',   'every8th/nuts5_416x240_30-Every8th.yuv',  416, 240, 30, 30, 0, 38])
addSeqAttr(['flower4_416x240_30-Every8th',   'every8th/flower4_416x240_30-Every8th.yuv',  416, 240, 30, 30, 0, 38])
addSeqAttr(['keiba3_416x240_30-Every8th',   'every8th/keiba3_416x240_30-Every8th.yuv',  416, 240, 30, 30, 0, 38])


addSeqAttr(['BQSquare_416x240_60', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BQSquare_416x240_60.yuv',   416, 240, 60, 60, 0, 600])
addSeqAttr(['RaceHorses_416x240_30', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/RaceHorses_416x240_30.yuv',   416, 240, 30, 30, 0, 300])
addSeqAttr(['BasketballPass_416x240_50', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BasketballPass_416x240_50.yuv',   416, 240, 50, 50, 0, 500])
addSeqAttr(['BlowingBubbles_416x240_50', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BlowingBubbles_416x240_50.yuv',   416, 240, 50, 50, 0, 500])
addSeqAttr(['BQMall_832x480_60', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BQMall_832x480_60.yuv',   832, 480, 60, 60, 0, 600])
addSeqAttr(['PartyScene_832x480_50', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/PartyScene_832x480_50.yuv',   832, 480, 50, 50, 0, 500])
addSeqAttr(['RaceHorses_832x480_30', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/RaceHorses_832x480_30.yuv',   832, 480, 30, 30, 0, 300])
addSeqAttr(['BasketballDrill_832x480_50', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BasketballDrill_832x480_50.yuv',   832, 480, 50, 50, 0, 500])
addSeqAttr(['Cactus_1920x1080_50', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/Cactus_1920x1080_50.yuv',   1920, 1080, 50, 50, 0, 500])
addSeqAttr(['BQTerrace_1920x1080_60', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BQTerrace_1920x1080_60.yuv',   1920, 1080, 60, 60, 0, 600])
addSeqAttr(['ParkScene_1920x1080_24', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/ParkScene_1920x1080_24.yuv',   1920, 1080, 24, 24, 0, 240])
addSeqAttr(['ChristmasTree_1920x1080_50', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/ChristmasTree_1920x1080_50.yuv',   1920, 1080, 50, 50, 0, 500])
addSeqAttr(['BasketballDrive_1920x1080_50', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BasketballDrive_1920x1080_50.yuv',   1920, 1080, 50, 50, 0, 500])
addSeqAttr(['PeopleOnStreet_2560x1600_30_crop', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/PeopleOnStreet_2560x1600_30_crop.yuv',   2560, 1600, 30, 30, 0, 150])
addSeqAttr(['Traffic_2560x1600_30_crop', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/Traffic_2560x1600_30_crop.yuv',   2560, 1600, 30, 30, 0, 300])
addSeqAttr(['ParkJoy_2560x1600_50_crop', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/ParkJoy_2560x1600_50_crop.yuv',   2560, 1600, 50, 50, 0, 500])
addSeqAttr(['Wisley2_1920x1080_50_full', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/Wisley2_1920x1080_50.yuv',   1920, 1080, 50, 50, 0, 991])
addSeqAttr(['Wisley2_1920x1080_50', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/Wisley2_1920x1080_50.yuv',   1920, 1080, 50, 50, 400, 500])

addSeqAttr(['Kimono1_1920x1080_24',     'mpeg/orig/Kimono1_1920x1080_24.yuv',     1920, 1080, 24, 24, 0, 240])
addSeqAttr(['Traffic_2560x1600_30_crop2', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/Traffic_2560x1600_30_crop.yuv',   2560, 1600, 30, 30, 0, 150])

addSeqAttr(['Steam_Locomotive_2560x1600_60_crop', 'jctvc/steam_locomotive_train.yuv',   2560, 1600, 60, 60, 0, 300])
addSeqAttr(['Nebuta_2560x1600_60_crop', 'jctvc/nebuta_festival.yuv',   2560, 1600, 60, 60, 0, 300])

addSeqAttr(['SteamLocomotiveTrain_2560x1600_60_10bit_crop', 'jctvc/SteamLocomotiveTrain_2560x1600_60_10bit_crop.yuv',   2560, 1600, 60, 60, 0, 300, '420', None, None, 10])
addSeqAttr(['Nebuta_2560x1600_60_10bit_crop', 'jctvc/NebutaFestival_2560x1600_60_10bit_crop.yuv',   2560, 1600, 60, 60, 0, 300, '420', None, None, 10])

addSeqAttr(['BasketballDrillText_832x480_50', 'jctvc/BasketballDrillText_832x480_50.yuv',   832, 480, 50, 50, 0, 500])
addSeqAttr(['ChinaSpeed_1024x768_30', 'jctvc/ChinaSpeed_1024x768_30.yuv',   1024, 768, 30, 30, 0, 500])
addSeqAttr(['SlideEditing_1280x720_30', 'jctvc/SlideEditing_1280x720_30.yuv',   1280, 720, 30, 30, 0, 300])
addSeqAttr(['SlideShow_1280x720_20', 'jctvc/SlideShow_1280x720_20.yuv',   1280, 720, 20, 20, 0, 500])

addSeqAttr(['Kim_1920x1080_24',     'mpeg/orig/Kimono1_1920x1080_24.yuv',     1920, 1080, 24, 24, 0, 240])
addSeqAttr(['PaS_1920x1080_24', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/ParkScene_1920x1080_24.yuv',   1920, 1080, 24, 24, 0, 240])
addSeqAttr(['BQT_1920x1080_60', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BQTerrace_1920x1080_60.yuv',   1920, 1080, 60, 60, 0, 600])
addSeqAttr(['Cac_1920x1080_50', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/Cactus_1920x1080_50.yuv',   1920, 1080, 50, 50, 0, 500])
addSeqAttr(['BaD_1920x1080_50', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BasketballDrive_1920x1080_50.yuv',   1920, 1080, 50, 50, 0, 500])
# 1080p50 and 720p50 test sequences for RSM encoding using 64 mirror picture at the start and 8 mirrored ones at end

addSeqAttr(['BasketBallDrive_1080p50_RSM', 'tandberg/1080p/BasketballDrive500_1920x1080_50_mirror.yuv',   1920, 1080, 50, 50, 0, 572])
addSeqAttr(['Cactus_1080p50_RSM', 'tandberg/1080p/Cactus_1920x1080_50_mirror.yuv',   1920, 1080, 50, 50, 0, 572])
addSeqAttr(['ChristmasTree_1080p50_RSM', 'tandberg/1080p/ChristmasTree500_1920x1080_50_mirror.yuv',   1920, 1080, 50, 50, 0, 572])
addSeqAttr(['CrowdRun_1080p50_RSM', 'tandberg/1080p/CrowdRun1080p50_mirror.yuv',   1920, 1080, 50, 50, 0, 572])
addSeqAttr(['DucksTakeOff_1080p50_RSM', 'tandberg/1080p/DucksTakeOff1080p50_mirror.yuv',   1920, 1080, 50, 50, 0, 572])
addSeqAttr(['InToTree_1080p50_RSM', 'tandberg/1080p/InToTree1080p50_mirror.yuv',   1920, 1080, 50, 50, 0, 572])
addSeqAttr(['OldTownCross_1080p50_RSM', 'tandberg/1080p/OldTownCross1080p50_mirror.yuv',   1920, 1080, 50, 50, 0, 572])
addSeqAttr(['ParkJoy_1080p50_RSM', 'tandberg/1080p/ParkJoy1080p50_mirror.yuv',   1920, 1080, 50, 50, 0, 572])
addSeqAttr(['Wisley_1080p50_RSM', 'tandberg/1080p/Wisley500_1920x1080_50_mirror.yuv',   1920, 1080, 50, 50, 0, 572])

addSeqAttr(['BasketBallDrive_720p50_RSM', 'tandberg/720p/BasketballDrive500_1280x720_50_mirror.yuv',   1280, 720, 50, 50, 0, 572])
addSeqAttr(['Cactus_720p50_RSM', 'tandberg/720p/Cactus_1280x720_50_mirror.yuv',   1280, 720, 50, 50, 0, 572])
addSeqAttr(['ChristmasTree_720p50_RSM', 'tandberg/720p/ChristmasTree500_1280x720_50_mirror.yuv',   1280, 720, 50, 50, 0, 572])
addSeqAttr(['CrowdRun_720p50_RSM', 'tandberg/720p/CrowdRun720p50_mirror.yuv',   1280, 720, 50, 50, 0, 572])
addSeqAttr(['DucksTakeOff_720p50_RSM', 'tandberg/720p/DucksTakeOff720p50_mirror.yuv',   1280, 720, 50, 50, 0, 572])
addSeqAttr(['InToTree_720p50_RSM', 'tandberg/720p/InToTree720p50_mirror.yuv',   1280, 720, 50, 50, 0, 572])
addSeqAttr(['OldTownCross_720p50_RSM', 'tandberg/720p/OldTownCross720p50_mirror.yuv',   1280, 720, 50, 50, 0, 572])
addSeqAttr(['ParkJoy_720p50_RSM', 'tandberg/720p/ParkJoy720p50_mirror.yuv',   1280, 720, 50, 50, 0, 572])
addSeqAttr(['Wisley_720p50_RSM', 'tandberg/720p/Wisley500_1280x720_50_mirror.yuv',   1280, 720, 50, 50, 0, 572])

addSeqAttr(['News_720p50_RSM', 'tandberg/720p/RSM_News.yuv', 1280, 720, 50, 50, 0, 273])
addSeqAttr(['Carol_Singers_720p50_RSM', 'tandberg/720p/RSM_Carol_Singers.yuv', 1280, 720, 50, 50, 0, 337])
addSeqAttr(['Oscars_720p50_RSM', 'tandberg/720p/RSM_Oscars.yuv', 1280, 720, 50, 50, 0, 189])

addSeqAttr(['Xmas_Tree_720p50_RSM', 'tandberg/720p/RSM_Xmas_Tree.yuv', 1280, 720, 50, 50, 0, 181])
addSeqAttr(['Victoria2_720p50_RSM', 'tandberg/720p/Victoria2.yuv', 1280, 720, 50, 50, 0, 597])
addSeqAttr(['American_Football_720p50_RSM', 'tandberg/720p/RSM_American_Football_720.yuv', 1280, 720, 50, 50, 0, 789])

#c65 sequences
addSeqAttr(['cx550_1_1280x720_25',        'HEVC_conversational/cx550_1.yuv', 1280, 720, 25, 25, 0, 300])
addSeqAttr(['cx550_2_1280x720_25',        'HEVC_conversational/cx550_2.yuv', 1280, 720, 25, 25, 0, 300])
addSeqAttr(['cx550_3_1280x720_25',        'HEVC_conversational/cx550_3.yuv', 1280, 720, 25, 25, 0, 300])
addSeqAttr(['er_office_1280x720_25',      'HEVC_conversational/er_office.yuv', 1280, 720, 25, 25, 0, 300])
addSeqAttr(['er_room_1280x720_25',        'HEVC_conversational/er_room.yuv', 1280, 720, 25, 25, 0, 300])
addSeqAttr(['vidyo2_1280x720_30',         'HEVC_conversational/vidyo2_30hz.yuv', 1280, 720, 30, 30, 0, 300])
addSeqAttr(['vidyo3_1280x720_30',         'HEVC_conversational/vidyo3_30hz.yuv', 1280, 720, 30, 30, 0, 300])
addSeqAttr(['vidyo4_1280x720_30',         'HEVC_conversational/vidyo4_30hz.yuv', 1280, 720, 30, 30, 0, 300])
addSeqAttr(['dance1_1280x720_25',         'HEVC_conversational/dance1.yuv', 1280, 720, 25, 25, 0, 300])
addSeqAttr(['fingers1_1280x720_25',       'HEVC_conversational/fingers1.yuv', 1280, 720, 25, 25, 0, 250])
addSeqAttr(['fingers2_1280x720_25',       'HEVC_conversational/fingers2.yuv', 1280, 720, 25, 25, 0, 250])
addSeqAttr(['fingers3_1280x720_25',       'HEVC_conversational/fingers3.yuv', 1280, 720, 25, 25, 0, 250])

#New deblocking sequences
##addSeqAttr(['Shields_ter_720p50', 'additional/new/720p50_shields_ter.yuv', 1280, 720, 50, 50, 2, 500])
##addSeqAttr(['Mobcal_720p50', 'additional/new/720p50_mobcal_ter.yuv',  1280, 720, 50, 50, 2, 500])
##addSeqAttr(['Aspen_1080p30', 'additional/new/Aspen_8bit.yuv', 1920, 1080, 30, 30, 0, 570])
##addSeqAttr(['SnowMnt_1080p30', 'additional/new/SnowMnt_8bit.yuv', 1920, 1080, 30, 30, 0, 570])
##addSeqAttr(['SpeedBag_1080p30', 'additional/new/SpeedBag_8bit.yuv', 1920, 1080, 30, 30, 0, 570])
##addSeqAttr(['RushFieldCuts_1080p30', 'additional/new/RushFieldCuts_8bit.yuv', 1920, 1080, 30, 30, 0, 570])
##addSeqAttr(['StereoWater_1080p_25', 'additional/new/3D_17_LEFT_3.yuv', 1920, 1080, 25, 25, 0, 337])
##addSeqAttr(['StereoFire_1080p_25', 'additional/new/3D_24_LEFT_3.yuv', 1920, 1080, 25, 25, 0, 337])
##addSeqAttr(['BirdsInCage_1080p60', 'additional/new/BirdsInCage_1920x1080_60_8bit_420.yuv', 1920, 1080, 60, 60, 0, 600])
##addSeqAttr(['dinner_1080p30', 'additional/new/dinner_1080p30.yuv', 1920, 1080, 30, 30, 0, 950])
##addSeqAttr(['DucksAndLegs_1080p60', 'additional/new/DucksAndLegs_1920x1080_60_8bit_420.yuv', 1920, 1080, 60, 60, 0, 600])
##addSeqAttr(['EBUGraphics_1080p50', 'additional/new/EBUGraphics_1920x1080_50_8bit_420.yuv', 1920, 1080, 50, 50, 0, 500])
##addSeqAttr(['EBUHorse_1080p50', 'additional/new/EBUHorse_1920x1080_50_8bit_420.yuv', 1920, 1080, 50, 50, 0, 500])
##addSeqAttr(['EBUKidsSoccer_1080p50', 'additional/new/EBUKidsSoccer_1920x1080_50_8bit_420.yuv', 1920, 1080, 50, 50, 0, 500])
##addSeqAttr(['OldTownCross_1080p50', 'additional/new/OldTownCross_1920x1080_50_8bit_420.yuv', 1920, 1080, 50, 50, 0, 500])
##addSeqAttr(['Seeking_1080p50', 'additional/new/Seeking_1920x1080_50_8bit_420.yuv', 1920, 1080, 50, 50, 0, 500])
##addSeqAttr(['sintel_trailer_1080p24', 'additional/new/sintel_trailer_2k_1080p24.yuv', 1920, 1080, 24, 24, 12, 1000])
##addSeqAttr(['VenueVu_1080p30', 'additional/new/VenueVu_1920x1080_30_8bit_440.yuv', 1920, 1080, 30, 30, 0, 300])
##addSeqAttr(['EBULupoCandlelight_1080p60', 'additional/new/EBULupoCandlelight_1920x1080_50_8bit_440.yuv', 1920, 1080, 60, 60, 0, 600])
##addSeqAttr(['EBURainFruits_1080p60', 'additional/new/EBURainFruits_1920x1080_50_8bit_440.yuv', 1920, 1080, 60, 60, 0, 600])
##addSeqAttr(['EBUWaterRocksClose_1080p60', 'additional/new/EBUWaterRocksClose_1920x1080_50_8bit_420.yuv', 1920, 1080, 60, 60, 0, 500])
##addSeqAttr(['JapaneseSingerYadif_1280x720_25', 'additional/JapaneseSingerYadif_1280x720.yuv',   1280, 720, 25, 25, 0, 134])
##addSeqAttr(['JapaneseSingerYadif_1920x1080p_25', 'additional/JapaneseSingerYadif_1920x1080.yuv', 1920, 1080, 25, 25, 0, 134])


#New deblocking sequences short (parts of the sequence)
addSeqAttr(['Shields_ter_720p50', 'additional/new/720p50_shields_ter.yuv', 1280, 720, 50, 50, 2, 200])
addSeqAttr(['Aspen_1080p30', 'additional/new/Aspen_8bit.yuv', 1920, 1080, 30, 30, 160, 200])
addSeqAttr(['SnowMnt_1080p30', 'additional/new/SnowMnt_8bit.yuv', 1920, 1080, 30, 30, 0, 300])
addSeqAttr(['SpeedBag_1080p30', 'additional/new/SpeedBag_8bit.yuv', 1920, 1080, 30, 30, 300, 200])
addSeqAttr(['RushFieldCuts_1080p30', 'additional/new/RushFieldCuts_8bit.yuv', 1920, 1080, 30, 30, 400, 160])
addSeqAttr(['StereoWater_1080p_25', 'additional/new/3D_17_LEFT_3.yuv', 1920, 1080, 25, 25, 0, 150])
addSeqAttr(['StereoFire_1080p_25', 'additional/new/3D_24_LEFT_3.yuv', 1920, 1080, 25, 25, 0, 150])
addSeqAttr(['BirdsInCage_1080p60', 'additional/new/BirdsInCage_1920x1080_60_8bit_420.yuv', 1920, 1080, 60, 60, 200, 100])
addSeqAttr(['dinner_1080p30', 'additional/new/dinner_1080p30.yuv', 1920, 1080, 30, 30, 0, 300])
addSeqAttr(['DucksAndLegs_1080p60', 'additional/new/DucksAndLegs_1920x1080_60_8bit_420.yuv', 1920, 1080, 60, 60, 400, 200])
addSeqAttr(['OldTownCross_1080p50', 'additional/new/OldTownCross_1920x1080_50_8bit_420.yuv', 1920, 1080, 50, 50, 0, 500])
addSeqAttr(['Seeking_1080p50', 'additional/new/Seeking_1920x1080_50_8bit_420.yuv', 1920, 1080, 50, 50, 70, 200])
addSeqAttr(['sintel_trailer_1080p24', 'additional/new/sintel_trailer_2k_1080p24.yuv', 1920, 1080, 24, 24, 350, 200])
addSeqAttr(['EBULupoCandlelight_1080p60', 'additional/new/EBULupoCandlelight_1920x1080_50_8bit_440.yuv', 1920, 1080, 60, 60, 300, 300])
addSeqAttr(['EBURainFruits_1080p60', 'additional/new/EBURainFruits_1920x1080_50_8bit_440.yuv', 1920, 1080, 60, 60, 0, 300])
addSeqAttr(['EBUWaterRocksClose_1080p60', 'additional/new/EBUWaterRocksClose_1920x1080_50_8bit_420.yuv', 1920, 1080, 60, 60, 0, 300])
addSeqAttr(['JapaneseSingerYadif_1280x720_25', 'additional/JapaneseSingerYadif_1280x720.yuv',   1280, 720, 25, 25, 0, 134])
addSeqAttr(['JapaneseSingerYadif_1920x1080p_25', 'additional/JapaneseSingerYadif_1920x1080.yuv', 1920, 1080, 25, 25, 0, 134])
#addSeqAttr(['Riverbed_1080p_25', 'additional/Riverbed_1920x1080_25fps.yuv', 1920, 1080, 25, 25, 0, 250])
#addSeqAttr(['Riverbed_LD_1080p_25', 'additional/Riverbed_1920x1080_25fps.yuv', 1920, 1080, 25, 25, 0, 250])

addSeqAttr(['horse_1920x1080p_30', 'additional/horse_1920x1080_30.yuv', 1920, 1080, 30, 30, 0, 342])
addSeqAttr(['boat_1920x1080p_30', 'additional/boat_1920x1080_30.yuv', 1920, 1080, 30, 30, 0, 274])
addSeqAttr(['snow_1920x1080p_30', 'additional/snow_1920x1080_30.yuv', 1920, 1080, 30, 30, 0, 573])

#addSeqAttr(['WestWindEasy_1920x1080_30', 'additional/WestWindEasy_p1920x1080_30fps_420pl_300fr.yuv', 1920, 1080, 30, 30, 0, 300])
addSeqAttr(['ControlledBurn_1920x1080_30', 'additional/ControlledBurn_p1920x1080_30fps_420pl_300fr.yuv', 1920, 1080, 30, 30, 0, 300])

#addSeqAttr(['WestWindEasy_LD_1920x1080_30', 'additional/WestWindEasy_p1920x1080_30fps_420pl_300fr.yuv', 1920, 1080, 30, 30, 0, 300])
addSeqAttr(['ControlledBurn_LD_1920x1080_30', 'additional/ControlledBurn_p1920x1080_30fps_420pl_300fr.yuv', 1920, 1080, 30, 30, 0, 300])
#addSeqAttr(['ChinaSpeed_LD_1024x768_30', 'jctvc/ChinaSpeed_1024x768_30.yuv',   1024, 768, 30, 30, 0, 500])
addSeqAttr(['SlideEditing_LD_1280x720_30', 'jctvc/SlideEditing_1280x720_30.yuv',   1280, 720, 30, 30, 0, 300])
#addSeqAttr(['DucksTakeOff_1080p_50', 'additional/ducks_take_off_1080p50.yuv', 1920, 1080, 50, 50, 0, 500])
#addSeqAttr(['DucksTakeOff_LD_1080p_50', 'additional/ducks_take_off_1080p50.yuv', 1920, 1080, 50, 50, 0, 500])
addSeqAttr(['RedKayak_1280x720_30', 'additional/RedKayak_720p30.yuv',   1280, 720, 30, 30, 0, 570])
#addSeqAttr(['RedKayak_1080p_30', 'additional/RedKayak_1080p30.yuv', 1920, 1080, 30, 30, 0, 570])

addSeqAttr(['Riverbed_1080p_25', 'additional/CE1/riverbed_1080p25.yuv', 1920, 1080, 25, 25, 0, 250])
addSeqAttr(['Riverbed_LD_1080p_25', 'additional/CE1/riverbed_1080p25.yuv', 1920, 1080, 25, 25, 0, 250])
addSeqAttr(['WestWindEasy_1920x1080_30', 'additional/CE1/WestWindEasy_p1920x1080_30fps_420pl_300fr.yuv', 1920, 1080, 30, 30, 0, 300])
addSeqAttr(['WestWindEasy_LD_1920x1080_30', 'additional/CE1/WestWindEasy_p1920x1080_30fps_420pl_300fr.yuv', 1920, 1080, 30, 30, 0, 300])
addSeqAttr(['ChinaSpeed_LD_1024x768_30', 'jctvc/ChinaSpeed_1024x768_30.yuv',   1024, 768, 30, 30, 0, 500])
addSeqAttr(['DucksTakeOff_1080p_50', 'additional/CE1/ducks_take_off_1080p50.yuv', 1920, 1080, 50, 50, 0, 500])
addSeqAttr(['DucksTakeOff_LD_1080p_50', 'additional/CE1/ducks_take_off_1080p50.yuv', 1920, 1080, 50, 50, 0, 500])
addSeqAttr(['DucksTakeOff_1080p_25', 'additional/CE1/ducks_take_off_1080p25.yuv', 1920, 1080, 25, 25, 0, 250])
addSeqAttr(['DucksTakeOff_LD_1080p_25', 'additional/CE1/ducks_take_off_1080p25.yuv', 1920, 1080, 25, 25, 0, 250])
addSeqAttr(['RedKayak_1080p_30', 'additional/CE1/red_kayak_1080p.yuv', 1920, 1080, 30, 30, 0, 570])
addSeqAttr(['RedKayak_1080p_30_300fr', 'additional/CE1/red_kayak_1080p.yuv', 1920, 1080, 30, 30, 0, 300])
addSeqAttr(['RedKayak_LD_1080p_30', 'additional/CE1/red_kayak_1080p.yuv', 1920, 1080, 30, 30, 0, 570])
addSeqAttr(['Riverbed_1080p_25_100', 'additional/CE1/riverbed_1080p25.yuv', 1920, 1080, 25, 25, 0, 100])
addSeqAttr(['SprinklerYadif_1280x720_25', 'additional/SprinklerYadif_1280x720.yuv',   1280, 720, 25, 25, 0, 85])
addSeqAttr(['SprinklerYadif_1080p_25', 'additional/SprinklerYadif_1920x1080.yuv', 1920, 1080, 25, 25, 0, 85])
addSeqAttr(['Spincalendar_720p_50', 'additional/Spincalendar_1280x720_550.yuv', 1280, 720, 50, 50, 0, 550])
addSeqAttr(['Spincalendar_LD_720p_50', 'additional/Spincalendar_1280x720_550.yuv', 1280, 720, 50, 50, 0, 550])
addSeqAttr(['ski_1920x1080_24',     'additional/new/ski4.yuv',     1920, 1080, 24, 24, 0, 240])
addSeqAttr(['canoe_1920x1080_24',     'additional/new/canoe.yuv',     1920, 1080, 24, 24, 0, 240])
addSeqAttr(['sand_1920x1080_24',     'additional/new/sand.yuv',     1920, 1080, 24, 24, 0, 240])

#SHVC sequences
addSeqAttr(['Cactus_1280x720_50_zerophase_0.9pi', 'jctvc/scalable/Cactus_1280x720_50_zerophase_0.9pi.yuv',   1280, 720, 50, 50, 0, 500])
addSeqAttr(['BQTerrace_1280x720_60_zerophase_0.9pi', 'jctvc/scalable/BQTerrace_1280x720_60_zerophase_0.9pi.yuv',   1280, 720, 60, 60, 0, 600])
addSeqAttr(['ParkScene_1280x720_24_zerophase_0.9pi', 'jctvc/scalable/ParkScene_1280x720_24_zerophase_0.9pi.yuv',   1280, 720, 24, 24, 0, 240])
addSeqAttr(['BasketballDrive_1280x720_50_zerophase_0.9pi', 'jctvc/scalable/BasketballDrive_1280x720_50_zerophase_0.9pi.yuv',   1280, 720, 50, 50, 0, 500])
addSeqAttr(['Kimono1_1280x720_24_zerophase_0.9pi',     'jctvc/scalable/Kimono1_1280x720_24_zerophase_0.9pi.yuv',     1280, 720, 24, 24, 0, 240])
addSeqAttr(['Kim_1280x720_24',     'jctvc/scalable/Kimono1_1280x720_24_zerophase_0.9pi.yuv',     1280, 720, 24, 24, 0, 240])
addSeqAttr(['Cac_1280x720_50', 'jctvc/scalable/Cactus_1280x720_50_zerophase_0.9pi.yuv',   1280, 720, 50, 50, 0, 500])
addSeqAttr(['BQT_1280x720_60', 'jctvc/scalable/BQTerrace_1280x720_60_zerophase_0.9pi.yuv',   1280, 720, 60, 60, 0, 600])
addSeqAttr(['PaS_1280x720_24', 'jctvc/scalable/ParkScene_1280x720_24_zerophase_0.9pi.yuv',   1280, 720, 24, 24, 0, 240])
addSeqAttr(['BaD_1280x720_50', 'jctvc/scalable/BasketballDrive_1280x720_50_zerophase_0.9pi.yuv',   1280, 720, 50, 50, 0, 500])

addSeqAttr(['Cactus_960x540_50_zerophase_0.9pi', 'jctvc/scalable/Cactus_960x540_50_zerophase_0.9pi.yuv',   960, 540, 50, 50, 0, 500])
addSeqAttr(['BQTerrace_960x540_60_zerophase_0.9pi', 'jctvc/scalable/BQTerrace_960x540_60_zerophase_0.9pi.yuv',   960, 540, 60, 60, 0, 600])
addSeqAttr(['ParkScene_960x540_24_zerophase_0.9pi', 'jctvc/scalable/ParkScene_960x540_24_zerophase_0.9pi.yuv',   960, 540, 24, 24, 0, 240])
addSeqAttr(['BasketballDrive_960x540_50_zerophase_0.9pi', 'jctvc/scalable/BasketballDrive_960x540_50_zerophase_0.9pi.yuv',   960, 540, 50, 50, 0, 500])
addSeqAttr(['Kimono1_960x540_24_zerophase_0.9pi',     'jctvc/scalable/Kimono1_960x540_24_zerophase_0.9pi.yuv',     960, 540, 24, 24, 0, 240])
addSeqAttr(['Kim_960x540_24',     'jctvc/scalable/Kimono1_960x540_24_zerophase_0.9pi.yuv',     960, 540, 24, 24, 0, 240])
addSeqAttr(['Cac_960x540_50', 'jctvc/scalable/Cactus_960x540_50_zerophase_0.9pi.yuv',   960, 540, 50, 50, 0, 500])
addSeqAttr(['BQT_960x540_60', 'jctvc/scalable/BQTerrace_960x540_60_zerophase_0.9pi.yuv',   960, 540, 60, 60, 0, 600])
addSeqAttr(['PaS_960x540_24', 'jctvc/scalable/ParkScene_960x540_24_zerophase_0.9pi.yuv',   960, 540, 24, 24, 0, 240])
addSeqAttr(['BaD_960x540_50', 'jctvc/scalable/BasketballDrive_960x540_50_zerophase_0.9pi.yuv',   960, 540, 50, 50, 0, 500])

addSeqAttr(['Cactus_640x360_50_zerophase_0.9pi', 'jctvc/scalable/Cactus_640x360_50_zerophase_0.9pi.yuv',   640, 360, 50, 50, 0, 500])
addSeqAttr(['BQTerrace_640x360_60_zerophase_0.9pi', 'jctvc/scalable/BQTerrace_640x360_60_zerophase_0.9pi.yuv',   640, 360, 60, 60, 0, 600])
addSeqAttr(['ParkScene_640x360_24_zerophase_0.9pi', 'jctvc/scalable/ParkScene_640x360_24_zerophase_0.9pi.yuv',   640, 360, 24, 24, 0, 240])
addSeqAttr(['BasketballDrive_640x360_50_zerophase_0.9pi', 'jctvc/scalable/BasketballDrive_640x360_50_zerophase_0.9pi.yuv',   640, 360, 50, 50, 0, 500])
addSeqAttr(['Kimono1_640x360_24_zerophase_0.9pi',     'jctvc/scalable/Kimono1_640x360_24_zerophase_0.9pi.yuv',     640, 360, 24, 24, 0, 240])

#addSeqAttr(['Traffic_2560x1600_30_crop2', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/Traffic_2560x1600_30_crop.yuv',   2560, 1600, 30, 30, 0, 150])
#addSeqAttr(['PeopleOnStreet_2560x1600_30_crop', 'MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/PeopleOnStreet_2560x1600_30_crop.yuv',   2560, 1600, 30, 30, 0, 150])
addSeqAttr(['Traffic_1280x800_30_crop_zerophase_0.9pi', 'jctvc/scalable/Traffic_1280x800_30_crop_zerophase_0.9pi.yuv',   1280, 800, 30, 30, 0, 150])
addSeqAttr(['PeopleOnStreet_1280x800_30_crop_zerophase_0.9pi', 'jctvc/scalable/PeopleOnStreet_1280x800_30_crop_zerophase_0.9pi.yuv',   1280, 800, 30, 30, 0, 150])

addSeqAttr(['Johnny_640x360_60_zerophase_0.9pi',     'jctvc/scalable/Johnny_640x360_60_zerophase_0.9pi.yuv',     640, 360, 60, 60, 0, 600])
addSeqAttr(['KristenAndSara_640x360_60_zerophase_0.9pi',     'jctvc/scalable/KristenAndSara_640x360_60_zerophase_0.9pi.yuv',     640, 360, 60, 60, 0, 600])
addSeqAttr(['FourPeople_640x360_60_zerophase_0.9pi',     'jctvc/scalable/FourPeople_640x360_60_zerophase_0.9pi.yuv',     640, 360, 60, 60, 0, 600])

addSeqAttr(['cx550_1_640x360_25_zerophase_0.9pi',     'jctvc/scalable/cx550_1_640x360_25_zerophase_0.9pi.yuv',     640, 360, 25, 25, 0, 300])
addSeqAttr(['cx550_2_640x360_25_zerophase_0.9pi',     'jctvc/scalable/cx550_2_640x360_25_zerophase_0.9pi.yuv',     640, 360, 25, 25, 0, 300])
addSeqAttr(['cx550_3_640x360_25_zerophase_0.9pi',     'jctvc/scalable/cx550_3_640x360_25_zerophase_0.9pi.yuv',     640, 360, 25, 25, 0, 300])
addSeqAttr(['vidyo4_640x360_30_zerophase_0.9pi',     'jctvc/scalable/vidyo4_640x360_30_zerophase_0.9pi.yuv',     640, 360, 30, 30, 0, 300])
addSeqAttr(['vidyo3_640x360_30_zerophase_0.9pi',     'jctvc/scalable/vidyo3_640x360_30_zerophase_0.9pi.yuv',     640, 360, 30, 30, 0, 300])
addSeqAttr(['vidyo2_640x360_30_zerophase_0.9pi',     'jctvc/scalable/vidyo2_640x360_30_zerophase_0.9pi.yuv',     640, 360, 30, 30, 0, 300])
addSeqAttr(['er_office_640x360_25_zerophase_0.9pi',     'jctvc/scalable/er_office_640x360_25_zerophase_0.9pi.yuv',     640, 360, 25, 25, 0, 300])
addSeqAttr(['er_room_640x360_25_zerophase_0.9pi',     'jctvc/scalable/er_room_640x360_25_zerophase_0.9pi.yuv',     640, 360, 25, 25, 0, 300])


# CA bilin
addSeqAttr(['Cactus_960x540_50_CA_bilin', 'jctvc/scalable/Cactus_960x540_50_CA_bilin.yuv',   960, 540, 50, 50, 0, 500])
addSeqAttr(['Cactus_1280x720_50_CA_bilin', 'jctvc/scalable/Cactus_1280x720_50_CA_bilin.yuv',   1280, 720, 50, 50, 0, 500])
addSeqAttr(['BQTerrace_960x540_60_CA_bilin', 'jctvc/scalable/BQTerrace_960x540_60_CA_bilin.yuv',   960, 540, 60, 60, 0, 600])
addSeqAttr(['BQTerrace_1280x720_60_CA_bilin', 'jctvc/scalable/BQTerrace_1280x720_60_CA_bilin.yuv', 1280, 720, 60, 60, 0, 600])
addSeqAttr(['BasketballDrive_960x540_50_CA_bilin', 'jctvc/scalable/BasketballDrive_960x540_50_CA_bilin.yuv',   960, 540, 50, 50, 0, 500])
addSeqAttr(['BasketballDrive_1280x720_50_CA_bilin', 'jctvc/scalable/BasketballDrive_1280x720_50_CA_bilin.yuv', 1280, 720, 50, 50, 0, 500])
addSeqAttr(['Kimono1_960x540_24_CA_bilin', 'jctvc/scalable/Kimono1_960x540_24_CA_bilin.yuv',   960, 540, 24, 24, 0, 240])
addSeqAttr(['Kimono1_1280x720_24_CA_bilin', 'jctvc/scalable/Kimono1_1280x720_24_CA_bilin.yuv', 1280, 720, 24, 24, 0, 240])
addSeqAttr(['ParkScene_960x540_24_CA_bilin', 'jctvc/scalable/ParkScene_960x540_24_CA_bilin.yuv',   960, 540, 24, 24, 0, 240])
addSeqAttr(['ParkScene_1280x720_24_CA_bilin', 'jctvc/scalable/ParkScene_1280x720_24_CA_bilin.yuv', 1280, 720, 24, 24, 0, 240])
addSeqAttr(['Traffic_1280x800_30_crop_CA_bilin', 'jctvc/scalable/Traffic_1280x800_30_crop_CA_bilin.yuv',   1280, 800, 30, 30, 0, 150])
addSeqAttr(['PeopleOnStreet_1280x800_30_crop_CA_bilin', 'jctvc/scalable/PeopleOnStreet_1280x800_30_crop_CA_bilin.yuv',   1280, 800, 30, 30, 0, 150])

# strong LP
addSeqAttr(['Cactus_960x540_50_STRONG_LP', 'jctvc/scalable/Cactus_960x540_50_STRONG_LP.yuv',   960, 540, 50, 50, 0, 500])
addSeqAttr(['Cactus_1280x720_50_STRONG_LP', 'jctvc/scalable/Cactus_1280x720_50_STRONG_LP.yuv', 1280, 720, 50, 50, 0, 500])
addSeqAttr(['BQTerrace_960x540_60_STRONG_LP', 'jctvc/scalable/BQTerrace_960x540_60_STRONG_LP.yuv',   960, 540, 60, 60, 0, 600])
addSeqAttr(['BQTerrace_1280x720_60_STRONG_LP', 'jctvc/scalable/BQTerrace_1280x720_60_STRONG_LP.yuv', 1280, 720, 60, 60, 0, 600])
addSeqAttr(['BasketballDrive_960x540_50_STRONG_LP', 'jctvc/scalable/BasketballDrive_960x540_50_STRONG_LP.yuv',   960, 540, 50, 50, 0, 500])
addSeqAttr(['BasketballDrive_1280x720_50_STRONG_LP', 'jctvc/scalable/BasketballDrive_1280x720_50_STRONG_LP.yuv', 1280, 720, 50, 50, 0, 500])
addSeqAttr(['Kimono1_960x540_24_STRONG_LP', 'jctvc/scalable/Kimono1_960x540_24_STRONG_LP.yuv',   960, 540, 24, 24, 0, 240])
addSeqAttr(['Kimono1_1280x720_24_STRONG_LP', 'jctvc/scalable/Kimono1_1280x720_24_STRONG_LP.yuv', 1280, 720, 24, 24, 0, 240])
addSeqAttr(['ParkScene_960x540_24_STRONG_LP', 'jctvc/scalable/ParkScene_960x540_24_STRONG_LP.yuv',   960, 540, 24, 24, 0, 240])
addSeqAttr(['ParkScene_1280x720_24_STRONG_LP', 'jctvc/scalable/ParkScene_1280x720_24_STRONG_LP.yuv', 1280, 720, 24, 24, 0, 240])
addSeqAttr(['Traffic_1280x800_30_crop_STRONG_LP', 'jctvc/scalable/Traffic_1280x800_30_crop_STRONG_LP.yuv',   1280, 800, 30, 30, 0, 150])
addSeqAttr(['PeopleOnStreet_1280x800_30_crop_STRONG_LP', 'jctvc/scalable/PeopleOnStreet_1280x800_30_crop_STRONG_LP.yuv',   1280, 800, 30, 30, 0, 150])



addSeqAttr(['BasketBallDrive_320p50_RSM', 'tandberg/320p/BasketballDrive500_640x320_50.yuv',   640, 320, 50, 50, 0, 65])
addSeqAttr(['Cactus_320p50_RSM',          'tandberg/320p/Cactus_640x320_50.yuv',   640, 320, 50, 50, 0, 65])
addSeqAttr(['ChristmasTree_320p50_RSM',   'tandberg/320p/ChristmasTree500_640x320_50.yuv',   640, 320, 50, 50, 0, 65])
addSeqAttr(['CrowdRun_320p50_RSM',        'tandberg/320p/CrowdRun320p50.yuv',   640, 320, 50, 50, 0, 65])
addSeqAttr(['DucksTakeOff_320p50_RSM',    'tandberg/320p/DucksTakeOff320p50.yuv',   640, 320, 50, 50, 0, 65])
addSeqAttr(['InToTree_320p50_RSM',        'tandberg/320p/InToTree320p50.yuv',   640, 320, 50, 50, 0, 65])
addSeqAttr(['OldTownCross_320p50_RSM',    'tandberg/320p/OldTownCross320p50.yuv',   640, 320, 50, 50, 0, 65])
addSeqAttr(['ParkJoy_320p50_RSM',         'tandberg/320p/ParkJoy320p50.yuv',   640, 320, 50, 50, 0, 65])
addSeqAttr(['Wisley_320p50_RSM',          'tandberg/320p/Wisley500_640x320_50.yuv',   640, 320, 50, 50, 0, 65])

addSeqAttr(['Dancers_640x320', 'tandberg/1088i/crop/Dancers_640x320.yuv',   640, 320, 30, 30, 0, 65])
addSeqAttr(['RSM2_Ceremony_640x320', 'tandberg/1088i/crop/RSM2_Ceremony_640x320.yuv',   640, 320, 30, 30, 0, 65])
addSeqAttr(['RSM2_Indian_640x320', 'tandberg/1088i/crop/RSM2_Indian_640x320.yuv',   640, 320, 30, 30, 0, 49])
addSeqAttr(['RSM2_Sky_Fade_640x320', 'tandberg/1088i/crop/RSM2_Sky_Fade_640x320.yuv',   640, 320, 30, 30, 0, 65])
addSeqAttr(['RSM_Crossfade_640x320', 'tandberg/1088i/crop/RSM_Crossfade_640x320.yuv',   640, 320, 30, 30, 0, 65])
addSeqAttr(['RSM_Cygnet_640x320', 'tandberg/1088i/crop/RSM_Cygnet_640x320.yuv',   640, 320, 30, 30, 0, 65])
addSeqAttr(['Talking_Head_640x320', 'tandberg/1088i/crop/Talking_Head_640x320.yuv',   640, 320, 30, 30, 0, 65])

addSeqAttr(['Dancers1088i', 'tandberg/1088i/Dancers1088i.yuv',   1920, 1088, 30, 30, 0, 249])
addSeqAttr(['RSM2_Ceremony', 'tandberg/1088i/RSM2_Ceremony.yuv',   1920, 1088, 30, 30, 0, 97])
addSeqAttr(['RSM2_Indian', 'tandberg/1088i/RSM2_Indian.yuv',   1920, 1088, 30, 30, 0, 49])
addSeqAttr(['RSM2_Sky_Fade', 'tandberg/1088i/RSM2_Sky_Fade.yuv',   1920, 1088, 30, 30, 0, 73])
addSeqAttr(['RSM_Crossfade', 'tandberg/1088i/RSM_Crossfade.yuv',   1920, 1088, 30, 30, 0, 105])
addSeqAttr(['RSM_Cygnet', 'tandberg/1088i/RSM_Cygnet.yuv',   1920, 1088, 30, 30, 0, 113])
addSeqAttr(['Talking_Head', 'tandberg/1088i/Talking_Head.yuv',   1920, 1088, 30, 30, 0, 249])

# screen content

#removed from screen content test set at SanJose meeting 2014
#sc_socialNetworkMap_1920x1080_60_8bit_444.yuv

#RGB
# note only 300 frames of flying grapics is used in SCC
addSeqAttr(['sc_flyingGraphics_1920x1080_60_8bit_444_rgb', 'screen_content/sc_flyingGraphics_1920x1080_60_8bit_rgb.rgb',   1920, 1080, 60, 60, 0, 300, '444', None, None, 8])
addSeqAttr(['sc_desktop_1920x1080_60_8bit_444_rgb', 'screen_content/sc_desktop_1920x1080_60_8bit_rgb.rgb',   1920, 1080, 60, 60, 0, 600, '444', None, None, 8])
addSeqAttr(['sc_console_1920x1080_60_8bit_444_rgb', 'screen_content/sc_console_1920x1080_60_8bit_rgb.rgb',   1920, 1080, 60, 60, 0, 600, '444', None, None, 8])
addSeqAttr(['sc_web_browsing_1280x720_30_300_8bit_444_rgb', 'screen_content/sc_web_browsing_1280x720_30_8bit_300_rgb.rgb',   1280, 720, 30, 30, 0, 300, '444', None, None, 8])
addSeqAttr(['sc_map_1280x720_60_8bit_444_rgb', 'screen_content/sc_map_1280x720_60_8bit.rgb',   1280, 720, 60, 60, 0, 600, '444', None, None, 8])
addSeqAttr(['sc_programming_1280x720_60_8bit_444_rgb', 'screen_content/sc_programming_1280x720_60_8bit.rgb',   1280, 720, 60, 60, 0, 600, '444', None, None, 8])
addSeqAttr(['sc_SlideShow_1280x720_20_8bit_500_444_rgb', 'screen_content/sc_SlideShow_1280x720_20_8bit_500.rgb',   1280, 720, 20, 20, 0, 500, '444', None, None, 8])
addSeqAttr(['sc_robot_1280x720_30_8bit_300_444_rgb', 'screen_content/sc_robot_1280x720_30_8bit_300.rgb',   1280, 720, 30, 30, 0, 300, '444', None, None, 8])

addSeqAttr(['MissionControlClip2_2560x1440_60p_8b444_rgb', 'screen_content/MissionControlClip2_2560x1440_60p_8b444.rgb',   2560, 1440, 60, 60, 120, 300, '444', None, None, 8])
addSeqAttr(['MissionControlClip3_1920x1080_60p_8b444_rgb', 'screen_content/MissionControlClip3_1920x1080_60p_8b444.rgb',   1920, 1080, 60, 60, 0, 600, '444', None, None, 8])
addSeqAttr(['Basketball_Screen_2560x1440_60p_8b444_rgb', 'screen_content/Basketball_Screen_2560x1440_60p_8b444.rgb',   2560, 1440, 60, 60, 322, 300, '444', None, None, 8])

#YUV
addSeqAttr(['sc_flyingGraphics_1920x1080_60_8bit_444_yuv', 'screen_content/sc_flyingGraphics_1920x1080_60_8bit_444.yuv',   1920, 1080, 60, 60, 0, 300, '444', None, None, 8])
addSeqAttr(['sc_desktop_1920x1080_60_8bit_444_yuv', 'screen_content/sc_desktop_1920x1080_60_8bit_444.yuv',   1920, 1080, 60, 60, 0, 600, '444', None, None, 8])
addSeqAttr(['sc_console_1920x1080_60_8bit_444_yuv', 'screen_content/sc_console_1920x1080_60_8bit_444.yuv',   1920, 1080, 60, 60, 0, 600, '444', None, None, 8])
addSeqAttr(['sc_web_browsing_1280x720_30_300_8bit_444_yuv', 'screen_content/sc_web_browsing_1280x720_30_8bit_300_444_r1.yuv',   1280, 720, 30, 30, 0, 300, '444', None, None, 8])
addSeqAttr(['sc_map_1280x720_60_8bit_444_yuv', 'screen_content/sc_map_1280x720_60_8bit_444.yuv',   1280, 720, 60, 60, 0, 600, '444', None, None, 8])
addSeqAttr(['sc_programming_1280x720_60_8bit_444_yuv', 'screen_content/sc_programming_1280x720_60_8bit_444.yuv',   1280, 720, 60, 60, 0, 600, '444', None, None, 8])
addSeqAttr(['sc_SlideShow_1280x720_20_8bit_500_444_yuv', 'screen_content/sc_SlideShow_1280x720_20_8bit_500_444.yuv',   1280, 720, 20, 20, 0, 500, '444', None, None, 8])
addSeqAttr(['sc_robot_1280x720_30_8bit_300_444_yuv', 'screen_content/sc_robot_1280x720_30_8bit_300_444.yuv',   1280, 720, 30, 30, 0, 300, '444', None, None, 8])

addSeqAttr(['MissionControlClip2_2560x1440_60p_8b444_yuv', 'screen_content/MissionControlClip2_2560x1440_60p_8b444.yuv',   2560, 1440, 60, 60, 120, 300, '444', None, None, 8])
addSeqAttr(['MissionControlClip3_1920x1080_60p_8b444_yuv', 'screen_content/MissionControlClip3_1920x1080_60p_8b444.yuv',   1920, 1080, 60, 60, 0, 600, '444', None, None, 8])
addSeqAttr(['Basketball_Screen_2560x1440_60p_8b444_yuv', 'screen_content/Basketball_Screen_2560x1440_60p_8b444.yuv',   2560, 1440, 60, 60, 322, 300, '444', None, None, 8])


#natural content part of screen content test set
# note only 120 frames used in SCC
addSeqAttr(['Kimono1_1920x1080_24_10bit_444_yuv', 'rext/Kimono1_1920x1080_24_10bit_444.yuv',   1920, 1080, 24, 24, 0, 120, '444', None, None, 10])
addSeqAttr(['Kimono1_1920x1080_24_10bit_444_rgb', 'rext/Kimono1_1920x1080_24_10bit_444.rgb',   1920, 1080, 24, 24, 0, 120, '444', None, None, 10])

# note only 250 frames used in SCC
addSeqAttr(['EBURainFruits_1920x1080_50_10bit_444_yuv', 'rext/EBURainFruits_1920x1080_50_10bit_444.yuv',   1920, 1080, 50, 50, 0, 250, '444', None, None, 10])
addSeqAttr(['EBURainFruits_1920x1080_50_10bit_444_rgb', 'rext/EBURainFruits_1920x1080_50_10bit_444.rgb',   1920, 1080, 50, 50, 0, 250, '444', None, None, 10])

# Test sequences for text over video
addSeqAttr(['crowd_run_text_1280x720_250Frames_25hz_rgbp','screen_content/test_text_over_video/crowd_run_text_1280x720_250Frames_25hz_rgbp.rgb', 1280, 720, 25, 25, 0, 250, '444', None, None, 8])
addSeqAttr(['crowd_run_blacktextbox_1280x720_250Frames_25hz_rgbp','screen_content/test_text_over_video/crowd_run_blacktextbox_1280x720_250Frames_25hz_rgbp.rgb', 1280, 720, 25, 25, 0, 250, '444', None, None, 8])
addSeqAttr(['crowd_run_transptextbox_1280x720_250Frames_25hz_rgbp','screen_content/test_text_over_video/crowd_run_transptextbox_1280x720_250Frames_25hz_rgbp.rgb', 1280, 720, 25, 25, 0, 250, '444', None, None, 8])
addSeqAttr(['BasketballDrive_text_1280x720_250Frames_25hz_rgbp', 'screen_content/test_text_over_video/BasketballDrive_text_1280x720_250Frames_25hz_rgbp.rgb', 1280, 720, 25, 25, 0, 250, '444', None, None, 8])
addSeqAttr(['sc_robot_text_1280x720_30_8bit_300_rgbp','screen_content/test_text_over_video/sc_robot_text_1280x720_30_8bit_300_rgbp.rgb', 1280, 720, 30, 30, 0, 300, '444', None, None, 8])

# Test sequences for MPEG HDR CfE

# HDR
# Class A
addSeqAttr(['FireEater2Clip4000r1_1920x1080p_25_hf_709_ct2020_444', 'HDR/CfE/openEXR/FireEater2Clip4000r1_1920x1080p_25_hf_709_ct2020_444/FireEater2Clip4000r1_1920x1080p_25_hf_709_ct2020_444_%05d.exr', 1920, 1080, 25, 25, 0, 200, '444', None, None, 16])
addSeqAttr(['Tibul2Clip4000r1_1920x1080p_30_hf_709_ct2020_444', 'HDR/CfE/openEXR/Tibul2Clip4000r1_1920x1080p_30_hf_709_ct2020_444/Tibul2Clip4000r1_1920x1080p_30_hf_709_ct2020_444_%05d.exr', 1920, 1080, 30, 30, 0, 240, '444', None, None, 16])
addSeqAttr(['Market3Clip4000r2_1920x1080p_50_hf_709_ct2020_444', 'HDR/CfE/openEXR/Market3Clip4000r2_1920x1080p_50_hf_709_ct2020_444/Market3Clip4000r2_1920x1080p_50_hf_709_ct2020_444_%05d.exr', 1920, 1080, 50, 50, 0, 400, '444', None, None, 16])

# Class B
addSeqAttr(['AutoWeldingClip4000_1920x1080p_24_12_P3_ct2020', 'HDR/Dolby/Dolby_MPEG_HDR_WCG_AhG_Content/AutoWelding/AutoWeldingClip4000_1920x1080p_24_12_P3_ct2020_%03d.tif', 1920, 1080, 24, 24, 0, 427, '444', None, None, 16])
addSeqAttr(['BikeSparklersClip4000_1920x1080p_24_12_P3_ct2020', 'HDR/Dolby/Dolby_MPEG_HDR_WCG_AhG_Content/BikeSparklers/BikeSparklersClip4000_1920x1080p_24_12_P3_ct2020_%03d.tif', 1920, 1080, 24, 24, 0, 480, '444', None, None, 16])

# Class B-nofade: Removes the first second so that the black fade is avoided
addSeqAttr(['AutoWeldingClip4000_1920x1080p_24_12_P3_ct2020_nofade', 'HDR/Dolby/Dolby_MPEG_HDR_WCG_AhG_Content/AutoWelding/AutoWeldingClip4000_1920x1080p_24_12_P3_ct2020_%03d.tif', 1920, 1080, 24, 24, 24, 403, '444', None, None, 16])
addSeqAttr(['BikeSparklersClip4000_1920x1080p_24_12_P3_ct2020_nofade', 'HDR/Dolby/Dolby_MPEG_HDR_WCG_AhG_Content/BikeSparklers/BikeSparklersClip4000_1920x1080p_24_12_P3_ct2020_%03d.tif', 1920, 1080, 24, 24, 24, 456, '444', None, None, 16])

# Class C
addSeqAttr(['ShowGirl2TeaserClip4000_1920x1080p_25_12_P3_ct2020', 'HDR/Stuttgart_MPEG/mpeg_share/ShowGirl2Teaser/ShowGirl2TeaserClip4000_1920x1080p_25_12_P3_ct2020_444i_%03d.tif', 1920, 1080, 25, 25, 0, 339, '444', None, None, 16])

# Class D
addSeqAttr(['StEM_MagicHourFountainToTable_1920x1080p_24_12_P3', 'HDR/StEM_technicolor/StEM-2014/StEM_2014_4000nit_24fps_1920x1080_PQ_P3rgb_16b/StEM_2014_4000nit_24fps_1920x1080_PQ_P3rgb_16b.%07d.tiff', 1920, 1080, 24, 24, 3527, 361, '444', None, None, 16])
addSeqAttr(['StEM_WarmNightTorchToTable_1920x1080p_24_12_P3', 'HDR/StEM_technicolor/StEM-2014/StEM_2014_4000nit_24fps_1920x1080_PQ_P3rgb_16b/StEM_2014_4000nit_24fps_1920x1080_PQ_P3rgb_16b.%07d.tiff', 1920, 1080, 24, 24, 6280, 361, '444', None, None, 16])

# Class G
addSeqAttr(['BalloonFestival_1920x1080p_24_hf_709_ct2020_444', 'HDR/CfE/openEXR/BalloonFestival_1920x1080p_24_hf_709_ct2020_444/BalloonFestival_1920x1080p_24_hf_709_ct2020_444_%05d.exr', 1920, 1080, 24, 24, 0, 240, '444', None, None, 16])

# Class A'
addSeqAttr(['FireEater2Clip4000r1_1920x1080p_25_hf_709_444', 'HDR/CfE/openEXR/FireEater2Clip4000r1_1920x1080p_25_hf_709_444/FireEater2Clip4000r1_1920x1080p_25_hf_709_444_%05d.exr', 1920, 1080, 25, 25, 0, 200, '444', None, None, 16])
addSeqAttr(['Tibul2Clip4000r1_1920x1080p_30_hf_709_444', 'HDR/CfE/openEXR/Tibul2Clip4000r1_1920x1080p_30_hf_709_444/Tibul2Clip4000r1_1920x1080p_30_hf_709_444_%05d.exr', 1920, 1080, 30, 30, 0, 240, '444', None, None, 16])
addSeqAttr(['Market3Clip4000r2_1920x1080p_50_hf_709_444', 'HDR/CfE/openEXR/Market3Clip4000r2_1920x1080p_50_hf_709_444/Market3Clip4000r2_1920x1080p_50_hf_709_444_%05d.exr', 1920, 1080, 50, 50, 0, 400, '444', None, None, 16])

# Class A' old grading
addSeqAttr(['FireEater2_1920x1080p_25_hf_709', 'HDR/old_technicolor_grading/FireEater2_1920x1080p_25_hf_709/FireEater2_1920x1080p_25_hf_709_%05d.exr', 1920, 1080, 25, 25, 0, 200, '444', None, None, 16])
addSeqAttr(['Tibul2_1920x1080p_30_hf_709', 'HDR/old_technicolor_grading/Tibul2_1920x1080p_30_hf_709/Tibul2_1920x1080p_30_hf_709_%05d.exr', 1920, 1080, 30, 30, 0, 240, '444', None, None, 16])
addSeqAttr(['Market3_1920x1080p_50_hf_709', 'HDR/old_technicolor_grading/Market3_1920x1080p_50_hf_709/Market3_1920x1080p_50_hf_709_%05d.exr', 1920, 1080, 50, 50, 0, 400, '444', None, None, 16])

# SDR
# Class AASA
addSeqAttr(['FireEater2Clip4000r1_AG_1920x1080p_25_10_709', 'HDR/CfE/SDR/FireEater2Clip4000r1_1920x1080p_25_10_709/FireEater2Clip4000r1_1920x1080p_25_10_709.yuv', 1920, 1080, 25, 25, 0, 200, '420', None, None, 10])
addSeqAttr(['Tibul2Clip4000r1_AG_1920x1080p_30_10_709', 'HDR/CfE/SDR/Tibul2Clip4000r1_1920x1080p_30_10_709/Tibul2Clip4000r1_1920x1080p_30_10_709.yuv', 1920, 1080, 30, 30, 0, 240, '420', None, None, 10])
addSeqAttr(['Market3Clip4000r2_AG_1920x1080p_50_10_709', 'HDR/CfE/SDR/Market3Clip4000r2_1920x1080p_50_10_709/Market3Clip4000r2_1920x1080p_50_10_709.yuv', 1920, 1080, 50, 50, 0, 400, '420', None, None, 10])

# Class GGSA
addSeqAttr(['BalloonFestival_AG_1920x1080p_24_10_709', 'HDR/CfE/SDR/BalloonFestival_1920x1080p_24_10_709/BalloonFestival_1920x1080p_24_10_709.yuv', 1920, 1080, 24, 24, 0, 240, '420', None, None, 10])

# Class AASC
addSeqAttr(['FireEater2Clip4000r1_CG_1920x1080p_25_10_709', 'HDR/CfE/SDR/FireEater2Clip4000r1_SDR_DLB_1920x1080p_25_10_709/FireEater2Clip4000r1_SDR_DLB_1920x1080p_25_10_709.yuv', 1920, 1080, 25, 25, 0, 200, '420', None, None, 10])
addSeqAttr(['Tibul2Clip4000r1_CG_1920x1080p_30_10_709', 'HDR/CfE/SDR/Tibul2Clip4000r1_SDR_DLB_1920x1080p_30_10_709/Tibul2Clip4000r1_SDR_DLB_1920x1080p_30_10_709.yuv', 1920, 1080, 30, 30, 0, 240, '420', None, None, 10])
addSeqAttr(['Market3Clip4000r2_CG_1920x1080p_50_10_709', 'HDR/CfE/SDR/Market3Clip4000r2_SDR_DLB_1920x1080p_50_10_709/Market3Clip4000r2_SDR_DLB_1920x1080p_50_10_709.yuv', 1920, 1080, 50, 50, 0, 400, '420', None, None, 10])

# Class BBSC
addSeqAttr(['AutoWeldingClip4000_CG_1920x1080p_24_10_709', 'HDR/Dolby/graded_SDR_classB/AutoWeldingClip4000_1920x1080p_24_10_709.yuv', 1920, 1080, 24, 24, 0, 427, '420', None, None, 10])
addSeqAttr(['BikeSparklersClip4000_CG_1920x1080p_24_10_709', 'HDR/Dolby/graded_SDR_classB/BikeSparklersClip4000_1920x1080p_24_10_709.yuv', 1920, 1080, 24, 24, 0, 480, '420', None, None, 10])

# Class DDSC
addSeqAttr(['StEM_MagicHourFountainToTable_CG_1920x1080p_24_10_709_444', 'HDR/StEM_technicolor/StEM-2014/StEM_2014_100nit_24fps_1920x1080_gam24_709rgb_16b/StEM_2014_100nit_24fps_1920x1080_gam24_709rgb_16b.%07d.tiff', 1920, 1080, 24, 24, 3527, 361, '444', None, None, 16])
addSeqAttr(['StEM_WarmNightTorchToTable_CG_1920x1080p_24_10_709_444', 'HDR/StEM_technicolor/StEM-2014/StEM_2014_100nit_24fps_1920x1080_gam24_709rgb_16b/StEM_2014_100nit_24fps_1920x1080_gam24_709rgb_16b.%07d.tiff', 1920, 1080, 24, 24, 6280, 361, '444', None, None, 16])

# Class GGSC
addSeqAttr(['BalloonFestival_CG_1920x1080p_24_10_709', 'HDR/CfE/SDR/BalloonFestival_SDR_DLB_1920x1080p_24_10_709/BalloonFestival_SDR_DLB_1920x1080p_24_10_709.yuv', 1920, 1080, 24, 24, 0, 240, '420', None, None, 10])


# Sequences removed from the MPEG HDR CfE
addSeqAttr(['BalloonClip4000r1_1920x1080p_25_hf_709_ct2020_444', 'HDR/CfE/openEXR/BalloonClip4000r1_1920x1080p_25_hf_709_ct2020_444/BalloonClip4000r1_1920x1080p_25_hf_709_ct2020_444_%05d.exr', 1920, 1080, 25, 25, 0, 200, '444', None, None, 16])
addSeqAttr(['BalloonClip4000r1_1920x1080p_25_hf_709_444', 'HDR/CfE/openEXR/BalloonClip4000r1_1920x1080p_25_hf_709_444/BalloonClip4000r1_1920x1080p_25_hf_709_444_%05d.exr', 1920, 1080, 25, 25, 0, 200, '444', None, None, 16])
addSeqAttr(['Typewriter_b1500_1920x1080p_24_hf_2020_ct2020_444i', 'HDR/CfE/openEXR/Typewriter_b1500_1920x1080p_24_hf_2020_ct2020_444i/Typewriter_b1500_1920x1080p_24_hf_2020_ct2020_444i_%03d.exr', 1920, 1080, 24, 24, 0, 240, '444', None, None, 16])
addSeqAttr(['WalkPath_1920x1080p_24_hf_709_ct2020_444', 'HDR/CfE/openEXR/WalkPath_1920x1080p_24_hf_709_ct2020_444/WalkPath_1920x1080p_24_hf_709_ct2020_444_%05d.exr', 1920, 1080, 24, 24, 0, 240, '444', None, None, 16])
addSeqAttr(['WalkPath_1920x1080p_24_10_709', 'HDR/CfE/SDR/WalkPath_1920x1080p_24_10_709/WalkPath_1920x1080p_24_10_709.yuv', 1920, 1080, 24, 24, 0, 240, '420', None, None, 10])


######################################################################################################################

### test set definitions
########################

#Test_originals = [[]]

RH832 = [[seqAttr['RaceHorses_832x480_30']]]

RSM_basket = [[seqAttr['BasketBallDrive_320p50_RSM']]]
RSM_320p = [[seqAttr['BasketBallDrive_320p50_RSM']],[seqAttr['Cactus_320p50_RSM']],[seqAttr['ChristmasTree_320p50_RSM']],[seqAttr['CrowdRun_320p50_RSM']],[seqAttr['DucksTakeOff_320p50_RSM']],[seqAttr['InToTree_320p50_RSM']],[seqAttr['OldTownCross_320p50_RSM']],[seqAttr['ParkJoy_320p50_RSM']],[seqAttr['Wisley_320p50_RSM']]]

RSM_dancers_320i = [[seqAttr['Dancers_640x320']]]
RSM_1080i_crop_to_320i = [[seqAttr['Dancers_640x320']],[seqAttr['RSM2_Ceremony_640x320']],[seqAttr['RSM2_Indian_640x320']],[seqAttr['RSM2_Sky_Fade_640x320']],[seqAttr['RSM_Crossfade_640x320']],[seqAttr['RSM_Cygnet_640x320']],[seqAttr['Talking_Head_640x320']]]
RSM_1080i = [[seqAttr['Dancers1088i']],[seqAttr['RSM2_Ceremony']],[seqAttr['RSM2_Indian']],[seqAttr['RSM2_Sky_Fade']],[seqAttr['RSM_Crossfade']],[seqAttr['RSM_Cygnet']],[seqAttr['Talking_Head']]]

c65_perf = [[seqAttr['cx550_1_1280x720_25']],[seqAttr['cx550_2_1280x720_25']],[seqAttr['cx550_3_1280x720_25']],[seqAttr['er_office_1280x720_25']],[seqAttr['er_room_1280x720_25']],[seqAttr['vidyo2_1280x720_30']],[seqAttr['vidyo3_1280x720_30']],[seqAttr['vidyo4_1280x720_30']]]
c65_speed = [[seqAttr['fingers1_1280x720_25']],[seqAttr['fingers2_1280x720_25']],[seqAttr['fingers3_1280x720_25']],[seqAttr['dance1_1280x720_25']]]


#SHVC (SHM1.0)

SHVC_EL = [[seqAttr['Traffic_2560x1600_30_crop2']],[seqAttr['PeopleOnStreet_2560x1600_30_crop']],[seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']]]
SHVC_EL_ClassB = [[seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']]]
SHVC_BASE_1280_ZEROPHASE_09PI = [[seqAttr['Traffic_1280x800_30_crop_zerophase_0.9pi']], [seqAttr['PeopleOnStreet_1280x800_30_crop_zerophase_0.9pi']],[seqAttr['Kimono1_1280x720_24_zerophase_0.9pi']], [seqAttr['ParkScene_1280x720_24_zerophase_0.9pi']], [seqAttr['Cactus_1280x720_50_zerophase_0.9pi']], [seqAttr['BasketballDrive_1280x720_50_zerophase_0.9pi']], [seqAttr['BQTerrace_1280x720_60_zerophase_0.9pi']]]
SHVC_BASE_960_ZEROPHASE_09PI = [[seqAttr['Kimono1_960x540_24_zerophase_0.9pi']], [seqAttr['ParkScene_960x540_24_zerophase_0.9pi']], [seqAttr['Cactus_960x540_50_zerophase_0.9pi']], [seqAttr['BasketballDrive_960x540_50_zerophase_0.9pi']], [seqAttr['BQTerrace_960x540_60_zerophase_0.9pi']]]

SHVC_SPATIAL_2X = [[seqAttr['Traffic_1280x800_30_crop_zerophase_0.9pi'], seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_1280x800_30_crop_zerophase_0.9pi'], seqAttr['PeopleOnStreet_2560x1600_30_crop']], [seqAttr['Kimono1_960x540_24_zerophase_0.9pi'], seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_960x540_24_zerophase_0.9pi'], seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_960x540_50_zerophase_0.9pi'], seqAttr['Cactus_1920x1080_50']], [seqAttr['BasketballDrive_960x540_50_zerophase_0.9pi'], seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['BQTerrace_960x540_60_zerophase_0.9pi'], seqAttr['BQTerrace_1920x1080_60']]]

SHVC_SPATIAL_1m5X = [[seqAttr['Kimono1_1280x720_24_zerophase_0.9pi'], seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1280x720_24_zerophase_0.9pi'], seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1280x720_50_zerophase_0.9pi'], seqAttr['Cactus_1920x1080_50']], [seqAttr['BasketballDrive_1280x720_50_zerophase_0.9pi'], seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['BQTerrace_1280x720_60_zerophase_0.9pi'], seqAttr['BQTerrace_1920x1080_60']]]

SHVC_SNR = [[seqAttr['Traffic_2560x1600_30_crop2'],seqAttr['Traffic_2560x1600_30_crop2']],[seqAttr['PeopleOnStreet_2560x1600_30_crop'],seqAttr['PeopleOnStreet_2560x1600_30_crop']],[seqAttr['Kimono1_1920x1080_24'],seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24'],seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50'],seqAttr['Cactus_1920x1080_50']], [seqAttr['BasketballDrive_1920x1080_50'],seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60'],seqAttr['BQTerrace_1920x1080_60']]]

SHVC_SPATIAL_1m5X_KIMONO = [[seqAttr['Kimono1_1280x720_24_zerophase_0.9pi'], seqAttr['Kimono1_1920x1080_24']]]
SHVC_SPATIAL_2X_KIMONO = [[seqAttr['Kimono1_960x540_24_zerophase_0.9pi'], seqAttr['Kimono1_1920x1080_24']]]

# not part of any test conditions
SHVC_SPATIAL_2X_CLASS_E = [[seqAttr['Johnny_640x360_60_zerophase_0.9pi'], seqAttr['Johnny_1280x720_60']], [seqAttr['KristenAndSara_640x360_60_zerophase_0.9pi'], seqAttr['KristenAndSara_1280x720_60']], [seqAttr['FourPeople_640x360_60_zerophase_0.9pi'], seqAttr['FourPeople_1280x720_60']]]

c65_perf_SPATIAL_2X = [[seqAttr['cx550_1_640x360_25_zerophase_0.9pi'], seqAttr['cx550_1_1280x720_25']], [seqAttr['cx550_2_640x360_25_zerophase_0.9pi'], seqAttr['cx550_2_1280x720_25']],[seqAttr['cx550_3_640x360_25_zerophase_0.9pi'], seqAttr['cx550_3_1280x720_25']], [seqAttr['er_office_640x360_25_zerophase_0.9pi'], seqAttr['er_office_1280x720_25']],[seqAttr['er_room_640x360_25_zerophase_0.9pi'], seqAttr['er_room_1280x720_25']], [seqAttr['vidyo2_640x360_30_zerophase_0.9pi'], seqAttr['vidyo2_1280x720_30']],[seqAttr['vidyo3_640x360_30_zerophase_0.9pi'], seqAttr['vidyo3_1280x720_30']],[seqAttr['vidyo4_640x360_30_zerophase_0.9pi'], seqAttr['vidyo4_1280x720_30']]]

SHVC_SPATIAL_2X_STRONG_LP =  [[seqAttr['Traffic_1280x800_30_crop_STRONG_LP'], seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_1280x800_30_crop_STRONG_LP'], seqAttr['PeopleOnStreet_2560x1600_30_crop']], [seqAttr['Kimono1_960x540_24_STRONG_LP'], seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_960x540_24_STRONG_LP'], seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_960x540_50_STRONG_LP'], seqAttr['Cactus_1920x1080_50']], [seqAttr['BasketballDrive_960x540_50_STRONG_LP'], seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['BQTerrace_960x540_60_STRONG_LP'], seqAttr['BQTerrace_1920x1080_60']]]
SHVC_SPATIAL_2X_BILIN =  [[seqAttr['Traffic_1280x800_30_crop_CA_bilin'], seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_1280x800_30_crop_CA_bilin'], seqAttr['PeopleOnStreet_2560x1600_30_crop']], [seqAttr['Kimono1_960x540_24_CA_bilin'], seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_960x540_24_CA_bilin'], seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_960x540_50_CA_bilin'], seqAttr['Cactus_1920x1080_50']], [seqAttr['BasketballDrive_960x540_50_CA_bilin'], seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['BQTerrace_960x540_60_CA_bilin'], seqAttr['BQTerrace_1920x1080_60']]]

SHVC_SPATIAL_2X_STRONG_LP_CLASS_B =  [[seqAttr['Kimono1_960x540_24_STRONG_LP'], seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_960x540_24_STRONG_LP'], seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_960x540_50_STRONG_LP'], seqAttr['Cactus_1920x1080_50']], [seqAttr['BasketballDrive_960x540_50_STRONG_LP'], seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['BQTerrace_960x540_60_STRONG_LP'], seqAttr['BQTerrace_1920x1080_60']]]
SHVC_SPATIAL_2X_BILIN_CLASS_B =  [[seqAttr['Kimono1_960x540_24_CA_bilin'], seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_960x540_24_CA_bilin'], seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_960x540_50_CA_bilin'], seqAttr['Cactus_1920x1080_50']], [seqAttr['BasketballDrive_960x540_50_CA_bilin'], seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['BQTerrace_960x540_60_CA_bilin'], seqAttr['BQTerrace_1920x1080_60']]]

SHVC_SPATIAL_1m5X_STRONG_LP =  [[seqAttr['Kimono1_1280x720_24_STRONG_LP'], seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1280x720_24_STRONG_LP'], seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1280x720_50_STRONG_LP'], seqAttr['Cactus_1920x1080_50']], [seqAttr['BasketballDrive_1280x720_50_STRONG_LP'], seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['BQTerrace_1280x720_60_STRONG_LP'], seqAttr['BQTerrace_1920x1080_60']]]
SHVC_SPATIAL_1m5X_BILIN =  [[seqAttr['Kimono1_1280x720_24_CA_bilin'], seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1280x720_24_CA_bilin'], seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1280x720_50_CA_bilin'], seqAttr['Cactus_1920x1080_50']], [seqAttr['BasketballDrive_1280x720_50_CA_bilin'], seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['BQTerrace_1280x720_60_CA_bilin'], seqAttr['BQTerrace_1920x1080_60']]]

SHVC_SPATIAL_2X_STRONG_LP_CACTUS = [[seqAttr['Cactus_960x540_50_STRONG_LP'], seqAttr['Cactus_1920x1080_50']]]
SHVC_SPATIAL_2X_BILIN_CACTUS = [[seqAttr['Cactus_960x540_50_CA_bilin'], seqAttr['Cactus_1920x1080_50']]]

SHVC_SPATIAL_1m5_STRONG_LP_CACTUS = [[seqAttr['Cactus_1280x720_50_STRONG_LP'], seqAttr['Cactus_1920x1080_50']]]
SHVC_SPATIAL_1m5X_BILIN_CACTUS = [[seqAttr['Cactus_1280x720_50_CA_bilin'], seqAttr['Cactus_1920x1080_50']]]

# for guided transcoding two layers
SHVC_SPATIAL_1m5X_TRN = [[seqAttr['Kimono1_1920x1080_24'], seqAttr['Kimono1_1280x720_24_zerophase_0.9pi']], [seqAttr['ParkScene_1920x1080_24'], seqAttr['ParkScene_1280x720_24_zerophase_0.9pi']], [seqAttr['Cactus_1920x1080_50'], seqAttr['Cactus_1280x720_50_zerophase_0.9pi']], [seqAttr['BasketballDrive_1920x1080_50'], seqAttr['BasketballDrive_1280x720_50_zerophase_0.9pi']], [seqAttr['BQTerrace_1920x1080_60'], seqAttr['BQTerrace_1280x720_60_zerophase_0.9pi']] ]
SHVC_SPATIAL_2X_TRN = [[seqAttr['Traffic_2560x1600_30_crop2'], seqAttr['Traffic_1280x800_30_crop_zerophase_0.9pi']], [seqAttr['PeopleOnStreet_2560x1600_30_crop'], seqAttr['PeopleOnStreet_1280x800_30_crop_zerophase_0.9pi']], [seqAttr['Kimono1_1920x1080_24'], seqAttr['Kimono1_960x540_24_zerophase_0.9pi']], [seqAttr['ParkScene_1920x1080_24'], seqAttr['ParkScene_960x540_24_zerophase_0.9pi']], [seqAttr['Cactus_1920x1080_50'], seqAttr['Cactus_960x540_50_zerophase_0.9pi']], [seqAttr['BasketballDrive_1920x1080_50'], seqAttr['BasketballDrive_960x540_50_zerophase_0.9pi']], [seqAttr['BQTerrace_1920x1080_60'], seqAttr['BQTerrace_960x540_60_zerophase_0.9pi']] ]
SHVC_SPATIAL_1m5X_KIMONO = [[seqAttr['Kimono1_1280x720_24_zerophase_0.9pi'], seqAttr['Kimono1_1920x1080_24']]]
SHVC_SPATIAL_1m5X_KIMONO_TRN = [[seqAttr['Kimono1_1920x1080_24'], seqAttr['Kimono1_1280x720_24_zerophase_0.9pi']]]
SHVC_SPATIAL_2X_KIMONO = [[seqAttr['Kimono1_960x540_24_zerophase_0.9pi'], seqAttr['Kimono1_1920x1080_24']]]
SHVC_SPATIAL_2X_KIMONO_TRN = [[seqAttr['Kimono1_1920x1080_24'], seqAttr['Kimono1_960x540_24_zerophase_0.9pi']]]

SHVC_SPATIAL_2X_HD_TRN = [[seqAttr['Kimono1_1920x1080_24'], seqAttr['Kimono1_960x540_24_zerophase_0.9pi']], [seqAttr['ParkScene_1920x1080_24'], seqAttr['ParkScene_960x540_24_zerophase_0.9pi']], [seqAttr['Cactus_1920x1080_50'], seqAttr['Cactus_960x540_50_zerophase_0.9pi']], [seqAttr['BasketballDrive_1920x1080_50'], seqAttr['BasketballDrive_960x540_50_zerophase_0.9pi']], [seqAttr['BQTerrace_1920x1080_60'], seqAttr['BQTerrace_960x540_60_zerophase_0.9pi']] ]
SHVC_SNR_HD_TRN = [[seqAttr['Kimono1_1920x1080_24'],seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24'],seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50'],seqAttr['Cactus_1920x1080_50']], [seqAttr['BasketballDrive_1920x1080_50'],seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60'],seqAttr['BQTerrace_1920x1080_60']]]
SHVC_SPATIAL_3X_HD_TRN = [[seqAttr['Kimono1_1920x1080_24'], seqAttr['Kimono1_640x360_24_zerophase_0.9pi']], [seqAttr['ParkScene_1920x1080_24'], seqAttr['ParkScene_640x360_24_zerophase_0.9pi']], [seqAttr['Cactus_1920x1080_50'], seqAttr['Cactus_640x360_50_zerophase_0.9pi']], [seqAttr['BasketballDrive_1920x1080_50'], seqAttr['BasketballDrive_640x360_50_zerophase_0.9pi']], [seqAttr['BQTerrace_1920x1080_60'], seqAttr['BQTerrace_640x360_60_zerophase_0.9pi']] ]


# for guided transcoding four layers
# 1080p, 1080p, 720p, 540p
# note, short filenames are used
SHVC_4LAYERS_TRN = [[seqAttr['Kim_1920x1080_24'], seqAttr['Kim_1920x1080_24'], seqAttr['Kim_1280x720_24'], seqAttr['Kim_960x540_24']],[seqAttr['PaS_1920x1080_24'], seqAttr['PaS_1920x1080_24'], seqAttr['PaS_1280x720_24'], seqAttr['PaS_960x540_24']],[seqAttr['Cac_1920x1080_50'], seqAttr['Cac_1920x1080_50'], seqAttr['Cac_1280x720_50'], seqAttr['Cac_960x540_50']],[seqAttr['BaD_1920x1080_50'], seqAttr['BaD_1920x1080_50'], seqAttr['BaD_1280x720_50'], seqAttr['BaD_960x540_50']],[seqAttr['BQT_1920x1080_60'], seqAttr['BQT_1920x1080_60'], seqAttr['BQT_1280x720_60'], seqAttr['BQT_960x540_60']]]
SHVC_4LAYERS_KIMONO_TRN = [[seqAttr['Kim_1920x1080_24'], seqAttr['Kim_1920x1080_24'], seqAttr['Kim_1280x720_24'], seqAttr['Kim_960x540_24']]]


# QCIF is not part of the VCEG test sequences
vcegQCIF = [ [seqAttr['ContainerQCIF15']], [seqAttr['ForemanQCIF15']], [seqAttr['SilentQCIF15']] ]

# VCEG test sequences according to VCEG-AJ10r1
vcegCIF  = [ [seqAttr['ParisCIF30']], [seqAttr['ForemanCIF30']], [seqAttr['MobileCIF30']], [seqAttr['TempeteCIF30']] ]
vceg832 = [ [seqAttr['nuts5_832x480_30']], [seqAttr['flower4_832x480_30']], [seqAttr['keiba3_832x480_30']] ]
vceg720p = [ [seqAttr['Janine1_8SVGA25']],  [seqAttr['BigShips720p60']], [seqAttr['City720p60']], [seqAttr['Crew720p60']], [seqAttr['Night720p60']], [seqAttr['Jets720p60']], [seqAttr['Raven720p60']] ]
vceg1080p = [ [seqAttr['CrowdRun1080p60']], [seqAttr['ParkJoy1080p60']], [seqAttr['sunflower1080p25']], [seqAttr['toys_and_calendar1080p25']], [seqAttr['Traffic_1920x1080_p30']] ]
vcegall = [ [seqAttr['ParisCIF30']], [seqAttr['ForemanCIF30']], [seqAttr['MobileCIF30']], [seqAttr['TempeteCIF30']],  [seqAttr['nuts5_832x480_30']], [seqAttr['flower4_832x480_30']], [seqAttr['keiba3_832x480_30']], [seqAttr['Janine1_8SVGA25']],  [seqAttr['BigShips720p60']], [seqAttr['City720p60']], [seqAttr['Crew720p60']], [seqAttr['Night720p60']], [seqAttr['Jets720p60']], [seqAttr['Raven720p60']], [seqAttr['CrowdRun1080p60']], [seqAttr['ParkJoy1080p60']], [seqAttr['sunflower1080p25']], [seqAttr['toys_and_calendar1080p25']], [seqAttr['Traffic_1920x1080_p30']] ]
vcegvidyo =[ [seqAttr['Vidyo1_720p60']],[seqAttr['Vidyo2_720p60']], [seqAttr['Vidyo3_720p60']], [seqAttr['Vidyo4_720p60']] ]

vcegtntm = [ [seqAttr['ParisCIF30']], [seqAttr['ForemanCIF30']], [seqAttr['MobileCIF30']], [seqAttr['TempeteCIF30']],  [seqAttr['nuts5_832x480_30']], [seqAttr['flower4_832x480_30']], [seqAttr['keiba3_832x480_30']], [seqAttr['Janine1_8SVGA25']],  [seqAttr['BigShips720p60']], [seqAttr['City720p60']], [seqAttr['Crew720p60']], [seqAttr['Night720p60']], [seqAttr['Jets720p60']], [seqAttr['Raven720p60']], [seqAttr['CrowdRun1080p60']], [seqAttr['ParkJoy1080p60']], [seqAttr['sunflower1080p25']], [seqAttr['toys_and_calendar1080p25']], [seqAttr['Traffic_1920x1080_p30']], [seqAttr['Vidyo1_720p60']], [seqAttr['Vidyo2_720p60']], [seqAttr['Vidyo3_720p60']], [seqAttr['Vidyo4_720p60']] ]

# MPEG test sequences according to w10553 Call for Evidence May 2009
mpeg416 = [ [seqAttr['nuts5_416x240_30']], [seqAttr['flower4_416x240_30']], [seqAttr['keiba3_416x240_30']] ]
mpeg832 = [ [seqAttr['nuts5_832x480_30']], [seqAttr['flower4_832x480_30']], [seqAttr['keiba3_832x480_30']] ]
mpeg1080p = [ [seqAttr['Kimono1_1080p24']], [seqAttr['ParkScene_1080p24']], [seqAttr['Tennis_1080p24']] ]
mpeg2560p = [ [seqAttr['ParkJoy_2560x1600_crop_p50']], [seqAttr['PeopleOnStreet_2560x1600_crop_p30']], [seqAttr['Traffic_2560x1600_crop_p30']] ]
mpegbcd = [ [seqAttr['nuts5_416x240_30']], [seqAttr['flower4_416x240_30']], [seqAttr['keiba3_416x240_30']], [seqAttr['nuts5_832x480_30']], [seqAttr['flower4_832x480_30']], [seqAttr['keiba3_832x480_30']], [seqAttr['Kimono1_1080p24']], [seqAttr['ParkScene_1080p24']], [seqAttr['Tennis_1080p24']] ]
mpegall = [ [seqAttr['nuts5_416x240_30']], [seqAttr['flower4_416x240_30']], [seqAttr['keiba3_416x240_30']], [seqAttr['nuts5_832x480_30']], [seqAttr['flower4_832x480_30']], [seqAttr['keiba3_832x480_30']], [seqAttr['Kimono1_1080p24']], [seqAttr['ParkScene_1080p24']], [seqAttr['Tennis_1080p24']], [seqAttr['ParkJoy_2560x1600_crop_p50']], [seqAttr['PeopleOnStreet_2560x1600_crop_p30']], [seqAttr['Traffic_2560x1600_crop_p30']] ]

# MPEG test sequence candidates for MPEG Call for Proposal as made available 2009-06-23
MPEG_CfP_everything_available = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['ChristmasTree_1920x1080_50']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']], [seqAttr['ParkJoy_2560x1600_50_crop']], [seqAttr['Wisley2_1920x1080_50_full']] ]

# MPEG test sequence candidates for MPEG Call for Proposal as of MPEG document N10722 (draft call for proposals)
# Compared to MPEG_CfP_everything_available, the following changes were made:
# - Christmas tree sequence removed since it is not listed in N10722
# - Only the first 500 frames used from Wisley
MPEG_CfP = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']], [seqAttr['ParkJoy_2560x1600_50_crop']], [seqAttr['Wisley2_1920x1080_50']] ]

MPEG_wisley = [ [seqAttr['Wisley2_1920x1080_50']] ]

MPEG_vidyo = [ [seqAttr['Vidyo1_720p60']],[seqAttr['Vidyo3_720p60']],[seqAttr['Vidyo4_720p60']]]
MPEG_kimono = [[seqAttr['Kimono1_1920x1080_24']]]

# MPEG test sequences for draft VCEG CfP and draft MPEG cfP call according to VCEG C-152 and MPEG N10926 November 6 2009
# HM test sequences including two 10 bit sequences
VCEGMPEG_CfP = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']],[seqAttr['Vidyo1_720p60']],[seqAttr['Vidyo3_720p60']],[seqAttr['Vidyo4_720p60']], [seqAttr['SteamLocomotiveTrain_2560x1600_60_10bit_crop']], [seqAttr['Nebuta_2560x1600_60_10bit_crop']] ]
VCEGMPEG_CS1_CfP = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']], [seqAttr['SteamLocomotiveTrain_2560x1600_60_10bit_crop']], [seqAttr['Nebuta_2560x1600_60_10bit_crop']] ]
VCEGMPEG_CS2_CfP = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Vidyo1_720p60']],[seqAttr['Vidyo3_720p60']],[seqAttr['Vidyo4_720p60']] ]

VCEGMPEG_ClassA = [ [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']], [seqAttr['SteamLocomotiveTrain_2560x1600_60_10bit_crop']], [seqAttr['Nebuta_2560x1600_60_10bit_crop']]]
VCEGMPEG_ClassB = [ [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']]]
VCEGMPEG_ClassC = [ [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']]]
VCEGMPEG_ClassD = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']]]
VCEGMPEG_ClassE = [ [seqAttr['Vidyo1_720p60']],[seqAttr['Vidyo3_720p60']],[seqAttr['Vidyo4_720p60']]]
VCEGMPEG_ClassEnew = [[seqAttr['FourPeople_1280x720_60']],[seqAttr['Johnny_1280x720_60']],[seqAttr['KristenAndSara_1280x720_60']]]

VCEGMPEG_ClassA10bit = [ [seqAttr['SteamLocomotiveTrain_2560x1600_60_10bit_crop']], [seqAttr['Nebuta_2560x1600_60_10bit_crop']]]
VCEGMPEG_ClassA8bit = [ [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']]]

#HM5 common test conditions test sequences
JCTVC = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']],[seqAttr['Vidyo1_720p60']],[seqAttr['Vidyo3_720p60']],[seqAttr['Vidyo4_720p60']]]
JCTVC_RA = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']] ]
JCTVC_LD = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Vidyo1_720p60']],[seqAttr['Vidyo3_720p60']],[seqAttr['Vidyo4_720p60']] ]
JCTVC_RA10bit = [ [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']], [seqAttr['SteamLocomotiveTrain_2560x1600_60_10bit_crop']], [seqAttr['Nebuta_2560x1600_60_10bit_crop']] ]

JCTVC_ALL = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']],[seqAttr['Vidyo1_720p60']],[seqAttr['Vidyo3_720p60']],[seqAttr['Vidyo4_720p60']],[seqAttr['BasketballDrillText_832x480_50']], [seqAttr['ChinaSpeed_1024x768_30']], [seqAttr['SlideEditing_1280x720_30']], [seqAttr['SlideShow_1280x720_20']]]
JCTVC_RA_ALL = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']],[seqAttr['BasketballDrillText_832x480_50']], [seqAttr['ChinaSpeed_1024x768_30']], [seqAttr['SlideEditing_1280x720_30']], [seqAttr['SlideShow_1280x720_20']] ]
JCTVC_LD_ALL = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Vidyo1_720p60']],[seqAttr['Vidyo3_720p60']],[seqAttr['Vidyo4_720p60']],[seqAttr['BasketballDrillText_832x480_50']], [seqAttr['ChinaSpeed_1024x768_30']], [seqAttr['SlideEditing_1280x720_30']], [seqAttr['SlideShow_1280x720_20']] ]

#HM6 common test contition including new Vidyo sequences
JCTVC_HM6_ALL = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']], [seqAttr['SteamLocomotiveTrain_2560x1600_60_10bit_crop']], [seqAttr['Nebuta_2560x1600_60_10bit_crop']],[seqAttr['FourPeople_1280x720_60']],[seqAttr['Johnny_1280x720_60']],[seqAttr['KristenAndSara_1280x720_60']],[seqAttr['BasketballDrillText_832x480_50']], [seqAttr['ChinaSpeed_1024x768_30']], [seqAttr['SlideEditing_1280x720_30']], [seqAttr['SlideShow_1280x720_20']]]
JCTVC_LD_HM6_ALL = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['FourPeople_1280x720_60']],[seqAttr['Johnny_1280x720_60']],[seqAttr['KristenAndSara_1280x720_60']],[seqAttr['BasketballDrillText_832x480_50']], [seqAttr['ChinaSpeed_1024x768_30']], [seqAttr['SlideEditing_1280x720_30']], [seqAttr['SlideShow_1280x720_20']] ]
JCTVC_RA_HM6_ALL = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']], [seqAttr['SteamLocomotiveTrain_2560x1600_60_10bit_crop']], [seqAttr['Nebuta_2560x1600_60_10bit_crop']],[seqAttr['BasketballDrillText_832x480_50']], [seqAttr['ChinaSpeed_1024x768_30']], [seqAttr['SlideEditing_1280x720_30']], [seqAttr['SlideShow_1280x720_20']]]

# optional
VCEGMPEG_ClassF = [ [seqAttr['BasketballDrillText_832x480_50']], [seqAttr['ChinaSpeed_1024x768_30']], [seqAttr['SlideEditing_1280x720_30']], [seqAttr['SlideShow_1280x720_20']]]

# all including optional
VCEGMPEG_CfP_all = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']],[seqAttr['Vidyo1_720p60']],[seqAttr['Vidyo3_720p60']],[seqAttr['Vidyo4_720p60']], [seqAttr['SteamLocomotiveTrain_2560x1600_60_10bit_crop']], [seqAttr['Nebuta_2560x1600_60_10bit_crop']], [seqAttr['BasketballDrillText_832x480_50']], [seqAttr['ChinaSpeed_1024x768_30']], [seqAttr['SlideEditing_1280x720_30']], [seqAttr['SlideShow_1280x720_20']] ]
VCEGMPEG_CS1_CfP_all = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']], [seqAttr['SteamLocomotiveTrain_2560x1600_60_10bit_crop']], [seqAttr['Nebuta_2560x1600_60_10bit_crop']], [seqAttr['BasketballDrillText_832x480_50']], [seqAttr['ChinaSpeed_1024x768_30']], [seqAttr['SlideEditing_1280x720_30']], [seqAttr['SlideShow_1280x720_20']] ]
VCEGMPEG_CS2_CfP_all = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Vidyo1_720p60']],[seqAttr['Vidyo3_720p60']],[seqAttr['Vidyo4_720p60']],[seqAttr['BasketballDrillText_832x480_50']], [seqAttr['ChinaSpeed_1024x768_30']], [seqAttr['SlideEditing_1280x720_30']], [seqAttr['SlideShow_1280x720_20']] ]


# Deblocking improvements
VCEGMPEG_CS1_CfP_SQ = [[seqAttr['BQMall_832x480_60']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['PartyScene_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']]]
VCEGMPEG_CS2_CfP_SQ = [[seqAttr['BQMall_832x480_60']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['PartyScene_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']],[seqAttr['Vidyo3_720p60']],[seqAttr['Vidyo4_720p60']]]
VCEGMPEG_CS2_CfP_SQVidyo = [[seqAttr['Vidyo3_720p60']],[seqAttr['Vidyo4_720p60']]]

VCEGMPEG_CS1_CfP_SQsdir = [[seqAttr['BQMall_RAHE_832x480_60']], [seqAttr['BasketballDrill_RAHE_832x480_50']], [seqAttr['PartyScene_RAHE_832x480_50']], [seqAttr['Kimono1_RAHE_1920x1080_24']], [seqAttr['ParkScene_RAHE_1920x1080_24']]]
VCEGMPEG_CS2_CfP_SQsdir = [[seqAttr['BQMall_LDLC_832x480_60']], [seqAttr['BasketballDrill_LDLC_832x480_50']], [seqAttr['PartyScene_LDLC_832x480_50']], [seqAttr['Kimono1_LDLC_1920x1080_24']], [seqAttr['ParkScene_LDLC_1920x1080_24']],[seqAttr['Vidyo3_LDLC_720p60']],[seqAttr['Vidyo4_LDLC_720p60']]]
VCEGMPEG_CS2_CfP_SQVidyosdir = [[seqAttr['Vidyo3_LDHE_720p60']],[seqAttr['Vidyo4_LDHE_720p60']]]

PartySceneRAHE = [[seqAttr['PartyScene_RAHE_832x480_50']]]
PartySceneLDLC = [[seqAttr['PartyScene_LDLC_832x480_50']]]

RH416 = [[seqAttr['RaceHorses_416x240_30']]]
Nebuta_2560x1600_60_10bit_crop = [[seqAttr['Nebuta_2560x1600_60_10bit_crop']]]
SteamLocomotiveTrain_2560x1600_60_10bit_crop = [[seqAttr['SteamLocomotiveTrain_2560x1600_60_10bit_crop']]]

CE1_and_more = [ [seqAttr['RedKayak_1080p_30']], [seqAttr['DucksTakeOff_1080p_25']], [seqAttr['WestWindEasy_1920x1080_30']], [seqAttr['Riverbed_1080p_25']],  [seqAttr['ChinaSpeed_1024x768_30']], [seqAttr['ControlledBurn_1920x1080_30']], [seqAttr['SprinklerYadif_1080p_25']], [seqAttr['SprinklerYadif_1280x720_25']], [seqAttr['JapaneseSingerYadif_1280x720_25']], [seqAttr['JapaneseSingerYadif_1920x1080p_25']], [seqAttr['sintel_trailer_1080p24']], [seqAttr['EBULupoCandlelight_1080p60']], [seqAttr['EBURainFruits_1080p60']], [seqAttr['EBUWaterRocksClose_1080p60']], [seqAttr['Shields_ter_720p50']],  [seqAttr['Aspen_1080p30']], [seqAttr['SnowMnt_1080p30']],  [seqAttr['SpeedBag_1080p30']],  [seqAttr['RushFieldCuts_1080p30']], [seqAttr['StereoWater_1080p_25']], [seqAttr['StereoFire_1080p_25']], [seqAttr['BirdsInCage_1080p60']],  [seqAttr['dinner_1080p30']],  [seqAttr['DucksAndLegs_1080p60']],  [seqAttr['Seeking_1080p50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']],  [seqAttr['Johnny_1280x720_60']], [seqAttr['KristenAndSara_1280x720_60']], [seqAttr['BasketballDrillText_832x480_50']], [seqAttr['BQMall_832x480_60']],  [seqAttr['PartyScene_832x480_50']], [seqAttr['fingers2_1280x720_25']], [seqAttr['ski_1920x1080_24']], [seqAttr['canoe_1920x1080_24']], [seqAttr['sand_1920x1080_24']]  ]
CE1_and_more2 = [ [seqAttr['RedKayak_1080p_30']], [seqAttr['DucksTakeOff_1080p_25']], [seqAttr['WestWindEasy_1920x1080_30']], [seqAttr['Riverbed_1080p_25']],  [seqAttr['ChinaSpeed_1024x768_30']], [seqAttr['ControlledBurn_1920x1080_30']], [seqAttr['SprinklerYadif_1080p_25']], [seqAttr['SprinklerYadif_1280x720_25']], [seqAttr['JapaneseSingerYadif_1280x720_25']], [seqAttr['JapaneseSingerYadif_1920x1080p_25']], [seqAttr['sintel_trailer_1080p24']], [seqAttr['EBULupoCandlelight_1080p60']], [seqAttr['EBURainFruits_1080p60']], [seqAttr['EBUWaterRocksClose_1080p60']], [seqAttr['Shields_ter_720p50']],  [seqAttr['Aspen_1080p30']], [seqAttr['SnowMnt_1080p30']],  [seqAttr['SpeedBag_1080p30']],  [seqAttr['RushFieldCuts_1080p30']], [seqAttr['StereoWater_1080p_25']], [seqAttr['StereoFire_1080p_25']], [seqAttr['BirdsInCage_1080p60']],  [seqAttr['dinner_1080p30']],  [seqAttr['DucksAndLegs_1080p60']],  [seqAttr['Seeking_1080p50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']],  [seqAttr['Johnny_1280x720_60']], [seqAttr['KristenAndSara_1280x720_60']], [seqAttr['BasketballDrillText_832x480_50']], [seqAttr['BQMall_832x480_60']],  [seqAttr['PartyScene_832x480_50']], [seqAttr['fingers2_1280x720_25']], [seqAttr['ski_1920x1080_24']], [seqAttr['canoe_1920x1080_24']], [seqAttr['sand_1920x1080_24']],[seqAttr['horse_1920x1080p_30']],[seqAttr['boat_1920x1080p_30']],[seqAttr['snow_1920x1080p_30']],[seqAttr['BQTerrace_1920x1080_60']]  ]
CE1_test = [ [seqAttr['RedKayak_1080p_30']], [seqAttr['RedKayak_1080p_30_300fr']], [seqAttr['DucksTakeOff_1080p_50']], [seqAttr['DucksTakeOff_1080p_25']], [seqAttr['WestWindEasy_1920x1080_30']], [seqAttr['Riverbed_1080p_25']],  [seqAttr['ChinaSpeed_1024x768_30']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']],  [seqAttr['Johnny_1280x720_60']], [seqAttr['KristenAndSara_1280x720_60']], [seqAttr['BasketballDrillText_832x480_50']], [seqAttr['BQMall_832x480_60']],  [seqAttr['PartyScene_832x480_50']], [seqAttr['SprinklerYadif_1080p_25']], [seqAttr['SprinklerYadif_1280x720_25']]  ]
CE1_test_short = [ [seqAttr['RedKayak_1080p_30']], [seqAttr['RedKayak_1080p_30_300fr']], [seqAttr['DucksTakeOff_1080p_50']], [seqAttr['DucksTakeOff_1080p_25']], [seqAttr['WestWindEasy_1920x1080_30']], [seqAttr['Riverbed_1080p_25']],  [seqAttr['ChinaSpeed_1024x768_30']], [seqAttr['SprinklerYadif_1080p_25']], [seqAttr['SprinklerYadif_1280x720_25']]  ]

Deblock_set = [ [seqAttr['RedKayak_1080p_30']], [seqAttr['DucksTakeOff_1080p_25']], [seqAttr['WestWindEasy_1920x1080_30']], [seqAttr['Riverbed_1080p_25']],  [seqAttr['ControlledBurn_1920x1080_30']], [seqAttr['SprinklerYadif_1080p_25']], [seqAttr['EBULupoCandlelight_1080p60']], [seqAttr['StereoFire_1080p_25']], [seqAttr['dinner_1080p30']],  [seqAttr['Kimono1_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']],  [seqAttr['Johnny_1280x720_60']], [seqAttr['KristenAndSara_1280x720_60']], [seqAttr['BasketballDrillText_832x480_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['fingers2_1280x720_25']], [seqAttr['ski_1920x1080_24']], [seqAttr['canoe_1920x1080_24']], [seqAttr['sand_1920x1080_24']]]
Deblock_set2 = [ [seqAttr['RedKayak_1080p_30']], [seqAttr['DucksTakeOff_1080p_25']], [seqAttr['WestWindEasy_1920x1080_30']], [seqAttr['Riverbed_1080p_25']],  [seqAttr['ControlledBurn_1920x1080_30']], [seqAttr['SprinklerYadif_1080p_25']], [seqAttr['EBULupoCandlelight_1080p60']], [seqAttr['StereoFire_1080p_25']], [seqAttr['dinner_1080p30']],  [seqAttr['Kimono1_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']],  [seqAttr['Johnny_1280x720_60']], [seqAttr['KristenAndSara_1280x720_60']], [seqAttr['BasketballDrillText_832x480_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['fingers2_1280x720_25']], [seqAttr['ski_1920x1080_24']], [seqAttr['canoe_1920x1080_24']], [seqAttr['sand_1920x1080_24']],[seqAttr['horse_1920x1080p_30']],[seqAttr['boat_1920x1080p_30']],[seqAttr['snow_1920x1080p_30']]]


# B110 also using Tennis
VCEGMPEG_CfPB110 = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']],[seqAttr['Vidyo1_720p60']],[seqAttr['Vidyo3_720p60']],[seqAttr['Vidyo4_720p60']],[seqAttr['Tennis_1080p24']] ]
VCEGMPEG_CS1_CfPB110 = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Traffic_2560x1600_30_crop2']], [seqAttr['PeopleOnStreet_2560x1600_30_crop']],[seqAttr['Tennis_1080p24']] ]
VCEGMPEG_CS2_CfPB110 = [ [seqAttr['BQSquare_416x240_60']], [seqAttr['RaceHorses_416x240_30']], [seqAttr['BasketballPass_416x240_50']], [seqAttr['BlowingBubbles_416x240_50']], [seqAttr['BQMall_832x480_60']], [seqAttr['PartyScene_832x480_50']], [seqAttr['RaceHorses_832x480_30']], [seqAttr['BasketballDrill_832x480_50']], [seqAttr['Kimono1_1920x1080_24']], [seqAttr['ParkScene_1920x1080_24']], [seqAttr['Cactus_1920x1080_50']], [seqAttr['BQTerrace_1920x1080_60']], [seqAttr['BasketballDrive_1920x1080_50']], [seqAttr['Vidyo1_720p60']],[seqAttr['Vidyo3_720p60']],[seqAttr['Vidyo4_720p60']],[seqAttr['Tennis_1080p24']] ]

# 1080p50 and 720p50 test sequences for RSM encoding using 64 mirror picture at the start and 8 mirrored ones at end
RSM_720p50 = [ [seqAttr['BasketBallDrive_720p50_RSM']], [seqAttr['Cactus_720p50_RSM']], [seqAttr['ChristmasTree_720p50_RSM']], [seqAttr['CrowdRun_720p50_RSM']], [seqAttr['DucksTakeOff_720p50_RSM']], [seqAttr['InToTree_720p50_RSM']], [seqAttr['OldTownCross_720p50_RSM']], [seqAttr['ParkJoy_720p50_RSM']], [seqAttr['Wisley_720p50_RSM']] ]
RSM_1080p50 = [ [seqAttr['BasketBallDrive_1080p50_RSM']], [seqAttr['Cactus_1080p50_RSM']], [seqAttr['ChristmasTree_1080p50_RSM']], [seqAttr['CrowdRun_1080p50_RSM']], [seqAttr['DucksTakeOff_1080p50_RSM']], [seqAttr['InToTree_1080p50_RSM']], [seqAttr['OldTownCross_1080p50_RSM']], [seqAttr['ParkJoy_1080p50_RSM']], [seqAttr['Wisley_1080p50_RSM']] ]

# 720p50 test sequences for RSM, JM and TMuC encoding, categorized in "easy" and "hard" tasks.
RSM_720p50_easy = [ [seqAttr['News_720p50_RSM']], [seqAttr['Carol_Singers_720p50_RSM']], [seqAttr['Oscars_720p50_RSM']] ]
RSM_720p50_hard = [ [seqAttr['Xmas_Tree_720p50_RSM']], [seqAttr['Victoria2_720p50_RSM']], [seqAttr['American_Football_720p50_RSM']] ]
RSM_720p50_HEVC_vs_AVC = [ [seqAttr['News_720p50_RSM']], [seqAttr['Carol_Singers_720p50_RSM']], [seqAttr['Oscars_720p50_RSM']], [seqAttr['Xmas_Tree_720p50_RSM']], [seqAttr['Victoria2_720p50_RSM']], [seqAttr['American_Football_720p50_RSM']] ]
RSM_720p50_news = [ [seqAttr['News_720p50_RSM']] ]
RSM_720p50_singers  = [ [seqAttr['Carol_Singers_720p50_RSM']] ]
RSM_720p50_oscars  = [ [seqAttr['Oscars_720p50_RSM']] ]
RSM_720p50_xmas  = [ [seqAttr['Xmas_Tree_720p50_RSM']] ]
RSM_720p50_vic  = [ [seqAttr['Victoria2_720p50_RSM']] ]
RSM_720p50_ame  = [ [seqAttr['American_Football_720p50_RSM']] ]

# Tandberg TV sequences
tandberg720p = [ [seqAttr['RSM_ABC_Football720p60']], [seqAttr['RSM_American_Football_720p60']], [seqAttr['RSM_Confetti720p60']],  [seqAttr['RSM_Fairground720p60']],  [seqAttr['RSM_Horse_Racing720p60']],  [seqAttr['RSM_Pop_Idol720p60']] ]

# Synthetic
syntheticCIF = [ [seqAttr['dcgrotCIF']]]

# SVC verification test sequences
svc_vt_SBB1 = [ [seqAttr['CrowdRunQVGA12.5'],seqAttr['CrowdRunQVGA12.5']], [seqAttr['SeekingQVGA12.5'],seqAttr['SeekingQVGA12.5']], [seqAttr['CrewQVGA15'],seqAttr['CrewQVGA15']], [seqAttr['SoccerQVGA15'],seqAttr['SoccerQVGA15']] ]
svc_vt_SBB2 = [ [seqAttr['CrowdRunQVGA12.5'],seqAttr['CrowdRunVGA25']], [seqAttr['SeekingQVGA12.5'],seqAttr['SeekingVGA25']], [seqAttr['CrewQVGA15'],seqAttr['CrewVGA30']], [seqAttr['SoccerQVGA15'],seqAttr['SoccerVGA30']] ]
svc_vt_SHB1 = [ [seqAttr['AlohaWave720p50'], seqAttr['AlohaWave1080p50']], [seqAttr['CrowdRun720p50'], seqAttr['CrowdRun1080p50']], [seqAttr['Seeking720p50'], seqAttr['Seeking1080p50']], [seqAttr['Umbrella720p50'], seqAttr['Umbrella1080p50']] ]

svc_vt_SBB2_BL = [ [seqAttr['CrowdRunQVGA12.5']], [seqAttr['SeekingQVGA12.5']], [seqAttr['CrewQVGA15']], [seqAttr['SoccerQVGA15']] ]
svc_vt_SBB2_EL = [ [seqAttr['CrowdRunVGA25']], [seqAttr['SeekingVGA25']], [seqAttr['CrewVGA30']], [seqAttr['SoccerVGA30']] ]
svc_vt_SHB1_BL = [ [seqAttr['AlohaWave720p50']], [seqAttr['CrowdRun720p50']], [seqAttr['Seeking720p50']], [seqAttr['Umbrella720p50']] ]
svc_vt_SHB1_EL = [ [seqAttr['AlohaWave1080p50']], [seqAttr['CrowdRun1080p50']], [seqAttr['Seeking1080p50']], [seqAttr['Umbrella1080p50']] ]

# SVC for video conferencing tests
svc_conf_vidyo_a = [ [seqAttr['Vidyo1_176x144_30'], seqAttr['Vidyo1_640x360_30'], seqAttr['Vidyo1_1280x720_30']],
                     [seqAttr['Vidyo2_176x144_30'], seqAttr['Vidyo2_640x360_30'], seqAttr['Vidyo2_1280x720_30']],
                     [seqAttr['Vidyo3_176x144_30'], seqAttr['Vidyo3_640x360_30'], seqAttr['Vidyo3_1280x720_30']],
                     [seqAttr['Vidyo4_176x144_30'], seqAttr['Vidyo4_640x360_30'], seqAttr['Vidyo4_1280x720_30']] ]
svc_conf_vidyo_b = [ [seqAttr['Vidyo1_256x144_30'], seqAttr['Vidyo1_640x360_30'], seqAttr['Vidyo1_1280x720_30']],
                     [seqAttr['Vidyo2_256x144_30'], seqAttr['Vidyo2_640x360_30'], seqAttr['Vidyo2_1280x720_30']],
                     [seqAttr['Vidyo3_256x144_30'], seqAttr['Vidyo3_640x360_30'], seqAttr['Vidyo3_1280x720_30']],
                     [seqAttr['Vidyo4_256x144_30'], seqAttr['Vidyo4_640x360_30'], seqAttr['Vidyo4_1280x720_30']] ]
svc_conf_face1_a = [ [seqAttr['Pico1_176x144_25'], seqAttr['Pico1_640x360_25'], seqAttr['Pico1_1280x720_25']],
                     [seqAttr['Pico2_176x144_25'], seqAttr['Pico2_640x360_25'], seqAttr['Pico2_1280x720_25']] ]
svc_conf_face1_b = [ [seqAttr['Pico1_256x144_25'], seqAttr['Pico1_640x360_25'], seqAttr['Pico1_1280x720_25']],
                     [seqAttr['Pico2_256x144_25'], seqAttr['Pico2_640x360_25'], seqAttr['Pico2_1280x720_25']] ]
svc_conf_face2_a = [ [seqAttr['c1mroom08_176x144_25'],   seqAttr['c1mroom08_640x360_25'],   seqAttr['c1mroom08_1280x720_25'],   seqAttr['c1mroom08_1920x1080_25']],
                     [seqAttr['c2hoffice04_176x144_25'], seqAttr['c2hoffice04_640x360_25'], seqAttr['c2hoffice04_1280x720_25'], seqAttr['c2hoffice04_1920x1080_25']],
                     [seqAttr['c2office04_176x144_25'],  seqAttr['c2office04_640x360_25'],  seqAttr['c2office04_1280x720_25'],  seqAttr['c2office04_1920x1080_25']] ]
svc_conf_face2_b = [ [seqAttr['c1mroom08_256x144_25'],   seqAttr['c1mroom08_640x360_25'],   seqAttr['c1mroom08_1280x720_25'],   seqAttr['c1mroom08_1920x1080_25']],
                     [seqAttr['c2hoffice04_256x144_25'], seqAttr['c2hoffice04_640x360_25'], seqAttr['c2hoffice04_1280x720_25'], seqAttr['c2hoffice04_1920x1080_25']],
                     [seqAttr['c2office04_256x144_25'],  seqAttr['c2office04_640x360_25'],  seqAttr['c2office04_1280x720_25'],  seqAttr['c2office04_1920x1080_25']] ]
svc_conf_face2_c = [ [seqAttr['c1mroom08_176x144_25'],   seqAttr['c1mroom08_640x360_25'],   seqAttr['c1mroom08_1920x1080_25']],
                     [seqAttr['c2hoffice04_176x144_25'], seqAttr['c2hoffice04_640x360_25'], seqAttr['c2hoffice04_1920x1080_25']],
                     [seqAttr['c2office04_176x144_25'],  seqAttr['c2office04_640x360_25'],  seqAttr['c2office04_1920x1080_25']] ]
svc_conf_face2_d = [ [seqAttr['c1mroom08_256x144_25'],   seqAttr['c1mroom08_640x360_25'],   seqAttr['c1mroom08_1920x1080_25']],
                     [seqAttr['c2hoffice04_256x144_25'], seqAttr['c2hoffice04_640x360_25'], seqAttr['c2hoffice04_1920x1080_25']],
                     [seqAttr['c2office04_256x144_25'],  seqAttr['c2office04_640x360_25'],  seqAttr['c2office04_1920x1080_25']] ]
svc_conf_vidyo_BP = [ [seqAttr['Vidyo1_176x144_30']],  [seqAttr['Vidyo2_176x144_30']],  [seqAttr['Vidyo3_176x144_30']],  [seqAttr['Vidyo4_176x144_30']],
                      [seqAttr['Vidyo1_256x144_30']],  [seqAttr['Vidyo2_256x144_30']],  [seqAttr['Vidyo3_256x144_30']],  [seqAttr['Vidyo4_256x144_30']] ]
svc_conf_vidyo_HP = [ [seqAttr['Vidyo1_640x360_30']],  [seqAttr['Vidyo2_640x360_30']],  [seqAttr['Vidyo3_640x360_30']],  [seqAttr['Vidyo4_640x360_30']],
                      [seqAttr['Vidyo1_1280x720_30']], [seqAttr['Vidyo2_1280x720_30']], [seqAttr['Vidyo3_1280x720_30']], [seqAttr['Vidyo4_1280x720_30']] ]
svc_conf_face1_BP = [ [seqAttr['Pico1_176x144_25']],  [seqAttr['Pico2_176x144_25']],
                      [seqAttr['Pico1_256x144_25']],  [seqAttr['Pico2_256x144_25']] ]
svc_conf_face1_HP = [ [seqAttr['Pico1_640x360_25']],  [seqAttr['Pico2_640x360_25']],
                      [seqAttr['Pico1_1280x720_25']], [seqAttr['Pico2_1280x720_25']] ]
svc_conf_face2_BP = [ [seqAttr['c1mroom08_176x144_25']],   [seqAttr['c2hoffice04_176x144_25']],   [seqAttr['c2office04_176x144_25']],
                      [seqAttr['c1mroom08_256x144_25']],   [seqAttr['c2hoffice04_256x144_25']],   [seqAttr['c2office04_256x144_25']] ]
svc_conf_face2_HP = [ [seqAttr['c1mroom08_640x360_25']],   [seqAttr['c2hoffice04_640x360_25']],   [seqAttr['c2office04_640x360_25']],
                      [seqAttr['c1mroom08_1280x720_25']],  [seqAttr['c2hoffice04_1280x720_25']],  [seqAttr['c2office04_1280x720_25']],
                      [seqAttr['c1mroom08_1920x1080_25']], [seqAttr['c2hoffice04_1920x1080_25']], [seqAttr['c2office04_1920x1080_25']] ]

# and some sequences with frames missing
every8th = [ [seqAttr['BigShips720p60-Every8th']], [seqAttr['City720p60-Every8th']], [seqAttr['Crew720p60-Every8th']], [seqAttr['Night720p60-Every8th']], [seqAttr['Raven720p60-Every8th']], [seqAttr['nuts5_416x240_30-Every8th']], [seqAttr['flower4_416x240_30-Every8th']], [seqAttr['keiba3_416x240_30-Every8th']] ]

# c65 test sequences
c65_perf = [[seqAttr['cx550_1_1280x720_25']],[seqAttr['cx550_2_1280x720_25']],[seqAttr['cx550_3_1280x720_25']],[seqAttr['er_office_1280x720_25']],[seqAttr['er_room_1280x720_25']],[seqAttr['vidyo2_1280x720_30']],[seqAttr['vidyo3_1280x720_30']],[seqAttr['vidyo4_1280x720_30']],[seqAttr['FourPeople_1280x720_60']],[seqAttr['Johnny_1280x720_60']],[seqAttr['KristenAndSara_1280x720_60']],[seqAttr['SlideEditing_1280x720_30']]]
c65_speed = [[seqAttr['fingers1_1280x720_25']],[seqAttr['fingers2_1280x720_25']],[seqAttr['fingers3_1280x720_25']]]


# screen content
sc_map_yuv = [[ seqAttr['sc_map_1280x720_60_8bit_444_yuv']]]
sc_map_rgb = [[ seqAttr['sc_map_1280x720_60_8bit_444_rgb']]]

sc_yuv = [[seqAttr['sc_flyingGraphics_1920x1080_60_8bit_444_yuv']], [seqAttr['sc_desktop_1920x1080_60_8bit_444_yuv']], [seqAttr['sc_console_1920x1080_60_8bit_444_yuv']], [seqAttr['sc_web_browsing_1280x720_30_300_8bit_444_yuv']], [seqAttr['sc_map_1280x720_60_8bit_444_yuv']], [seqAttr['sc_programming_1280x720_60_8bit_444_yuv']], [seqAttr['sc_SlideShow_1280x720_20_8bit_500_444_yuv']], [seqAttr['Basketball_Screen_2560x1440_60p_8b444_yuv']], [seqAttr['MissionControlClip2_2560x1440_60p_8b444_yuv']], [seqAttr['MissionControlClip3_1920x1080_60p_8b444_yuv']], [seqAttr['sc_robot_1280x720_30_8bit_300_444_yuv']]]
sc_rgb = [[seqAttr['sc_flyingGraphics_1920x1080_60_8bit_444_rgb']], [seqAttr['sc_desktop_1920x1080_60_8bit_444_rgb']], [seqAttr['sc_console_1920x1080_60_8bit_444_rgb']], [seqAttr['sc_web_browsing_1280x720_30_300_8bit_444_rgb']], [seqAttr['sc_map_1280x720_60_8bit_444_rgb']], [seqAttr['sc_programming_1280x720_60_8bit_444_rgb']], [seqAttr['sc_SlideShow_1280x720_20_8bit_500_444_rgb']], [seqAttr['Basketball_Screen_2560x1440_60p_8b444_rgb']], [seqAttr['MissionControlClip2_2560x1440_60p_8b444_rgb']], [seqAttr['MissionControlClip3_1920x1080_60p_8b444_rgb']], [seqAttr['sc_robot_1280x720_30_8bit_300_444_rgb']]]

#sc_all_no_normal = [[seqAttr['sc_flyingGraphics_1920x1080_60_8bit_444_rgb']], [seqAttr['sc_desktop_1920x1080_60_8bit_444_rgb']], [seqAttr['sc_console_1920x1080_60_8bit_444_rgb']], [seqAttr['sc_web_browsing_1280x720_30_300_8bit_444_rgb']], [seqAttr['sc_map_1280x720_60_8bit_444_rgb']], [seqAttr['sc_programming_1280x720_60_8bit_444_rgb']], [seqAttr['sc_SlideShow_1280x720_20_8bit_500_444_rgb']], [seqAttr['Basketball_Screen_2560x1440_60p_8b444_rgb']], [seqAttr['MissionControlClip2_2560x1440_60p_8b444_rgb']], [seqAttr['MissionControlClip3_1920x1080_60p_8b444_rgb']], [seqAttr['sc_robot_1280x720_30_8bit_300_444_rgb']], [seqAttr['sc_flyingGraphics_1920x1080_60_8bit_444_yuv']], [seqAttr['sc_desktop_1920x1080_60_8bit_444_yuv']], [seqAttr['sc_console_1920x1080_60_8bit_444_yuv']], [seqAttr['sc_web_browsing_1280x720_30_300_8bit_444_yuv']], [seqAttr['sc_map_1280x720_60_8bit_444_yuv']], [seqAttr['sc_programming_1280x720_60_8bit_444_yuv']], [seqAttr['sc_SlideShow_1280x720_20_8bit_500_444_yuv']], [seqAttr['Basketball_Screen_2560x1440_60p_8b444_yuv']], [seqAttr['MissionControlClip2_2560x1440_60p_8b444_yuv']], [seqAttr['MissionControlClip3_1920x1080_60p_8b444_yuv']], [seqAttr['sc_robot_1280x720_30_8bit_300_444_yuv']]]

sc_normal_yuv = [[seqAttr['EBURainFruits_1920x1080_50_10bit_444_yuv']], [seqAttr['Kimono1_1920x1080_24_10bit_444_yuv']]]
sc_normal_rgb = [[seqAttr['EBURainFruits_1920x1080_50_10bit_444_rgb']], [seqAttr['Kimono1_1920x1080_24_10bit_444_rgb']]]

sc_all_yuv = [[seqAttr['sc_flyingGraphics_1920x1080_60_8bit_444_yuv']], [seqAttr['sc_desktop_1920x1080_60_8bit_444_yuv']], [seqAttr['sc_console_1920x1080_60_8bit_444_yuv']], [seqAttr['sc_web_browsing_1280x720_30_300_8bit_444_yuv']], [seqAttr['sc_map_1280x720_60_8bit_444_yuv']], [seqAttr['sc_programming_1280x720_60_8bit_444_yuv']], [seqAttr['sc_SlideShow_1280x720_20_8bit_500_444_yuv']], [seqAttr['Basketball_Screen_2560x1440_60p_8b444_yuv']], [seqAttr['MissionControlClip2_2560x1440_60p_8b444_yuv']], [seqAttr['MissionControlClip3_1920x1080_60p_8b444_yuv']], [seqAttr['sc_robot_1280x720_30_8bit_300_444_yuv']], [seqAttr['EBURainFruits_1920x1080_50_10bit_444_yuv']], [seqAttr['Kimono1_1920x1080_24_10bit_444_yuv']]]
sc_all_rgb = [[seqAttr['sc_flyingGraphics_1920x1080_60_8bit_444_rgb']], [seqAttr['sc_desktop_1920x1080_60_8bit_444_rgb']], [seqAttr['sc_console_1920x1080_60_8bit_444_rgb']], [seqAttr['sc_web_browsing_1280x720_30_300_8bit_444_rgb']], [seqAttr['sc_map_1280x720_60_8bit_444_rgb']], [seqAttr['sc_programming_1280x720_60_8bit_444_rgb']], [seqAttr['sc_SlideShow_1280x720_20_8bit_500_444_rgb']], [seqAttr['Basketball_Screen_2560x1440_60p_8b444_rgb']], [seqAttr['MissionControlClip2_2560x1440_60p_8b444_rgb']], [seqAttr['MissionControlClip3_1920x1080_60p_8b444_rgb']], [seqAttr['sc_robot_1280x720_30_8bit_300_444_rgb']], [seqAttr['EBURainFruits_1920x1080_50_10bit_444_rgb']], [seqAttr['Kimono1_1920x1080_24_10bit_444_rgb']] ]

sc_normal_EBURainFruits_yuv = [[seqAttr['EBURainFruits_1920x1080_50_10bit_444_yuv']]]

#sc_all = [[seqAttr['sc_flyingGraphics_1920x1080_60_8bit_444_rgb']], [seqAttr['sc_desktop_1920x1080_60_8bit_444_rgb']], [seqAttr['sc_console_1920x1080_60_8bit_444_rgb']], [seqAttr['sc_web_browsing_1280x720_30_300_8bit_444_rgb']], [seqAttr['sc_map_1280x720_60_8bit_444_rgb']], [seqAttr['sc_programming_1280x720_60_8bit_444_rgb']], [seqAttr['sc_SlideShow_1280x720_20_8bit_500_444_rgb']], [seqAttr['Basketball_Screen_2560x1440_60p_8b444_rgb']], [seqAttr['MissionControlClip2_2560x1440_60p_8b444_rgb']], [seqAttr['MissionControlClip3_1920x1080_60p_8b444_rgb']], [seqAttr['sc_robot_1280x720_30_8bit_300_444_rgb']], [seqAttr['EBURainFruits_1920x1080_50_10bit_444_rgb']], [seqAttr['Kimono1_1920x1080_24_10bit_444_rgb']], [seqAttr['sc_flyingGraphics_1920x1080_60_8bit_444_yuv']], [seqAttr['sc_desktop_1920x1080_60_8bit_444_yuv']], [seqAttr['sc_console_1920x1080_60_8bit_444_yuv']], [seqAttr['sc_web_browsing_1280x720_30_300_8bit_444_yuv']], [seqAttr['sc_map_1280x720_60_8bit_444_yuv']], [seqAttr['sc_programming_1280x720_60_8bit_444_yuv']], [seqAttr['sc_SlideShow_1280x720_20_8bit_500_444_yuv']], [seqAttr['Basketball_Screen_2560x1440_60p_8b444_yuv']], [seqAttr['MissionControlClip2_2560x1440_60p_8b444_yuv']], [seqAttr['MissionControlClip3_1920x1080_60p_8b444_yuv']], [seqAttr['sc_robot_1280x720_30_8bit_300_444_yuv']], [seqAttr['EBURainFruits_1920x1080_50_10bit_444_yuv']], [seqAttr['Kimono1_1920x1080_24_10bit_444_yuv']]]

sc_text_over_video = [[seqAttr['crowd_run_text_1280x720_250Frames_25hz_rgbp']],[seqAttr['crowd_run_blacktextbox_1280x720_250Frames_25hz_rgbp']],[seqAttr['crowd_run_transptextbox_1280x720_250Frames_25hz_rgbp']],[seqAttr['BasketballDrive_text_1280x720_250Frames_25hz_rgbp']],[seqAttr['sc_robot_text_1280x720_30_8bit_300_rgbp']]]

#
# MPEG_HDR_CfE_class_A_old  = [ [seqAttr['BalloonClip4000r1_1920x1080p_25_hf_709_444']],
#                              [seqAttr['FireEater2Clip4000r1_1920x1080p_25_hf_709_444']],
#                              [seqAttr['Tibul2Clip4000r1_1920x1080p_30_hf_709_444']],
#                              [seqAttr['Market3Clip4000r2_1920x1080p_50_hf_709_444']]]
# MPEG_HDR_CfE_class_Aprim = [ [seqAttr['BalloonClip4000r1_1920x1080p_25_hf_709_ct2020_444']],
#                              [seqAttr['FireEater2Clip4000r1_1920x1080p_25_hf_709_ct2020_444']],
#                              [seqAttr['Tibul2Clip4000r1_1920x1080p_30_hf_709_ct2020_444']],
#                              [seqAttr['Market3Clip4000r2_1920x1080p_50_hf_709_ct2020_444']]]

# MPEG_HDR_CfE_class_F = [ [seqAttr['Typewriter_b1500_1920x1080p_24_hf_2020_ct2020_444i']]]


MPEG_HDR_CfE_class_A = [[seqAttr['FireEater2Clip4000r1_1920x1080p_25_hf_709_ct2020_444']],
                        [seqAttr['Tibul2Clip4000r1_1920x1080p_30_hf_709_ct2020_444']],
                        [seqAttr['Market3Clip4000r2_1920x1080p_50_hf_709_ct2020_444']]]

MPEG_HDR_CfE_class_B = [[seqAttr['AutoWeldingClip4000_1920x1080p_24_12_P3_ct2020']],
                        [seqAttr['BikeSparklersClip4000_1920x1080p_24_12_P3_ct2020']]]

MPEG_HDR_CfE_class_B_nofade = [[seqAttr['AutoWeldingClip4000_1920x1080p_24_12_P3_ct2020_nofade']],
                        [seqAttr['BikeSparklersClip4000_1920x1080p_24_12_P3_ct2020_nofade']]]

MPEG_HDR_CfE_class_C = [[seqAttr['ShowGirl2TeaserClip4000_1920x1080p_25_12_P3_ct2020']]]

MPEG_HDR_CfE_class_D = [[seqAttr['StEM_MagicHourFountainToTable_1920x1080p_24_12_P3']],
                        [seqAttr['StEM_WarmNightTorchToTable_1920x1080p_24_12_P3']]]

MPEG_HDR_CfE_class_G = [[seqAttr['BalloonFestival_1920x1080p_24_hf_709_ct2020_444']]]

MPEG_HDR_CfE_class_A_prime = [[seqAttr['FireEater2Clip4000r1_1920x1080p_25_hf_709_444']],
                              [seqAttr['Tibul2Clip4000r1_1920x1080p_30_hf_709_444']],
                              [seqAttr['Market3Clip4000r2_1920x1080p_50_hf_709_444']]]

MPEG_HDR_CfE_class_AASA = [[seqAttr['FireEater2Clip4000r1_AG_1920x1080p_25_10_709']],
                           [seqAttr['Tibul2Clip4000r1_AG_1920x1080p_30_10_709']],
                           [seqAttr['Market3Clip4000r2_AG_1920x1080p_50_10_709']]]

MPEG_HDR_CfE_class_GGSA = [[seqAttr['BalloonFestival_AG_1920x1080p_24_10_709']]]

MPEG_HDR_CfE_class_AASC = [[seqAttr['FireEater2Clip4000r1_CG_1920x1080p_25_10_709']],
                           [seqAttr['Tibul2Clip4000r1_CG_1920x1080p_30_10_709']],
                           [seqAttr['Market3Clip4000r2_CG_1920x1080p_50_10_709']]]

MPEG_HDR_CfE_class_BBSC = [[seqAttr['AutoWeldingClip4000_CG_1920x1080p_24_10_709']],
                           [seqAttr['BikeSparklersClip4000_CG_1920x1080p_24_10_709']]]

MPEG_HDR_CfE_class_DDSC = [[seqAttr['StEM_MagicHourFountainToTable_CG_1920x1080p_24_10_709_444']],
                           [seqAttr['StEM_WarmNightTorchToTable_CG_1920x1080p_24_10_709_444']]]

MPEG_HDR_CfE_class_GGSC = [[seqAttr['BalloonFestival_CG_1920x1080p_24_10_709']]]

MPEG_HDR_CfE_HDR = []; [MPEG_HDR_CfE_HDR.extend(s) for s in \
                        [MPEG_HDR_CfE_class_A, MPEG_HDR_CfE_class_B, MPEG_HDR_CfE_class_C, MPEG_HDR_CfE_class_D, MPEG_HDR_CfE_class_G,
                         MPEG_HDR_CfE_class_A_prime]]

MPEG_HDR_CfE_HDR_nofade = []; [MPEG_HDR_CfE_HDR_nofade.extend(s) for s in \
                        [MPEG_HDR_CfE_class_A, MPEG_HDR_CfE_class_B_nofade, MPEG_HDR_CfE_class_C, MPEG_HDR_CfE_class_D, MPEG_HDR_CfE_class_G,
                         MPEG_HDR_CfE_class_A_prime]]

MPEG_HDR_CfE_all = MPEG_HDR_CfE_HDR # Deprecated name, but kept for backwards compatibility

MPEG_HDR_OLD_GRADE = [[seqAttr['FireEater2_1920x1080p_25_hf_709']],[seqAttr['Tibul2_1920x1080p_30_hf_709']],[seqAttr['Market3_1920x1080p_50_hf_709']]]
MPEG_HDR_709_PROBLEM_SEQUENCES = []; [MPEG_HDR_709_PROBLEM_SEQUENCES.extend(s) for s in \
                        [MPEG_HDR_OLD_GRADE, [[seqAttr['BalloonClip4000r1_1920x1080p_25_hf_709_444']]]]]

MPEG_HDR_CfE_SDR = []; [MPEG_HDR_CfE_SDR.extend(s) for s in \
                        [MPEG_HDR_CfE_class_AASA,                                                   MPEG_HDR_CfE_class_GGSA,
                         MPEG_HDR_CfE_class_AASC, MPEG_HDR_CfE_class_BBSC, MPEG_HDR_CfE_class_DDSC, MPEG_HDR_CfE_class_GGSC]]

MPEG_HDR_CfE_SHVC = [];
[MPEG_HDR_CfE_SHVC.extend(s) for s in [[[sdr[0],hdr[0]]] for sdr, hdr in zip(MPEG_HDR_CfE_class_AASA, MPEG_HDR_CfE_class_A)]]
[MPEG_HDR_CfE_SHVC.extend(s) for s in [[[sdr[0],hdr[0]]] for sdr, hdr in zip(MPEG_HDR_CfE_class_GGSA, MPEG_HDR_CfE_class_G)]]
[MPEG_HDR_CfE_SHVC.extend(s) for s in [[[sdr[0],hdr[0]]] for sdr, hdr in zip(MPEG_HDR_CfE_class_AASC, MPEG_HDR_CfE_class_A)]]
[MPEG_HDR_CfE_SHVC.extend(s) for s in [[[sdr[0],hdr[0]]] for sdr, hdr in zip(MPEG_HDR_CfE_class_BBSC, MPEG_HDR_CfE_class_B)]]
[MPEG_HDR_CfE_SHVC.extend(s) for s in [[[sdr[0],hdr[0]]] for sdr, hdr in zip(MPEG_HDR_CfE_class_DDSC, MPEG_HDR_CfE_class_D)]]
[MPEG_HDR_CfE_SHVC.extend(s) for s in [[[sdr[0],hdr[0]]] for sdr, hdr in zip(MPEG_HDR_CfE_class_GGSC, MPEG_HDR_CfE_class_G)]]


