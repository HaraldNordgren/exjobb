import sys, os
from pyExcelerator import *


class Md5ExcelPlotter:
    def __init__(self):
        self.workbook = Workbook()

    def collectMd5ForHDR(self, settingsFile, xlsFile, jobsdir):
        sequences = getSequencesFromSettingsFile(settingsFile)

        worksheet = [self.workbook.add_sheet("Cat. 1 proposal"),
                     self.workbook.add_sheet("Cat. 3 proposal")]

        worksheet[0].write(2, 2, "Bitstreams")
        worksheet[0].write(2, 3, "Reconstructed YCbCr after HM decoding")
        worksheet[0].write(2, 4, "EXR BT.2020 of the reconstructed video")
        worksheet[0].write(2, 5, "Tiff of the reconstructed video")
        worksheet[0].write(2, 6, "Sim2 of the reconstructed video")

        worksheet[1].write(2, 2, "Bitstreams")
        worksheet[1].write(2, 3, "Reconstructed YCbCr after HM decoding")
        worksheet[1].write(2, 4, "EXR BT.2020 of the reconstructed video")
        worksheet[1].write(2, 5, "Tiff of the reconstructed video")
        worksheet[1].write(2, 6, "Sim2 of the reconstructed video")

        offsetX = 0
        offsetY = 3

        numQPs = 4
        order = [0,1,2,3,4,5,6,7,8,9,10,11,12,13]
        skipAt = [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        sortedSequences = [sequences[order[i]] for i, _ in enumerate(sequences)]
        sortedSequences = [sortedSequences[0:9], sortedSequences[9:12]]

        for sheetID in [0,1]:
            y = 0
            for seq in sortedSequences[sheetID]:
                y += skipAt[y]
                qpCount = -1
                for folder in sorted(os.listdir(jobsdir)):
                    if seq in folder:

                        qp = folder.split(seq+'qp')[-1]
                        qpCount += 1

                        # Filename
                        if not qpCount:
                            worksheet[sheetID].write(offsetY+y*numQPs+qpCount, offsetX, seq.replace('StEM_', '').split('_')[0])

                        # QP
                        worksheet[sheetID].write(offsetY+y*numQPs+qpCount, offsetX+1, qp)

                        # Bitstream checksum
                        fn = '%s%s%s%s%s.md5' % (jobsdir, os.sep, folder, os.sep, folder)
                        md5checksum = getFirstWordInFile(fn)
                        worksheet[sheetID].write(offsetY+y*numQPs+qpCount, offsetX+2, md5checksum)

                        # Dec checksum
                        fn = '%s%s%s%sdec_%s_L0.md5' % (jobsdir, os.sep, folder, os.sep, folder)
                        md5checksum = getFirstWordInFile(fn)
                        worksheet[sheetID].write(offsetY+y*numQPs+qpCount, offsetX+3, md5checksum)

                        # Exr checksum
                        fn = '%s%s%s%spst_%s_cat.md5' % (jobsdir, os.sep, folder, os.sep, folder)
                        md5checksum = getFirstWordInFile(fn)
                        worksheet[sheetID].write(offsetY+y*numQPs+qpCount, offsetX+4, md5checksum)

                        # Tiff checksum
                        fn = '%s%s%s%stif_%s_cat.md5' % (jobsdir, os.sep, folder, os.sep, folder)
                        md5checksum = getFirstWordInFile(fn)
                        worksheet[sheetID].write(offsetY+y*numQPs+qpCount, offsetX+5, md5checksum)

                        # Sim2 avi checksum
                        fn = '%s%s%s%ssim2_%s.md5' % (jobsdir, os.sep, folder, os.sep, folder)
                        md5checksum = getFirstWordInFile(fn)
                        worksheet[sheetID].write(offsetY+y*numQPs+qpCount, offsetX+6, md5checksum)


                y += 1

        self.workbook.save(xlsFile)
        print "Md5 checksums written to " + xlsFile


    def collectMd5ForHDRanchor(self, settingsFile, xlsFile, jobsdir):
        sequences = getSequencesFromSettingsFile(settingsFile)

        worksheet = self.workbook.add_sheet("Cat. 1 anchors")

        worksheet.write(2, 3, "Conversion from source TIFF to EXR BT.2020")
        worksheet.write(2, 4, "Conversion from source (EXR or TIFF) to YCbCr 10b PQ")
        worksheet.write(2, 5, "Conversion from source (EXR or TIFF) to YCbCr 10b PQ then to EXR BT.2020")
        worksheet.write(2, 6, "Conversion from source (EXR or TIFF) to logY'u'v' SIM2")
        worksheet.write(2, 7, "QP I Slice")
        worksheet.write(2, 8, "Bitstreams")
        worksheet.write(2, 9, "Reconstructed YCbCr after HM decoding")
        worksheet.write(2, 10, "Conversion from reconstructed YCbCr to EXR BT.2020")
        worksheet.write(2, 11, "Conversion from reconstructed YCbCr to EXR BT.2020 then to logY'u'v' SIM2")

        offsetX = 2
        offsetY = 3
        y = 0

        numQPs = 4
        order = [0,2,1,3,4,5,6,7,8,9,10,11,12,13]
        skipAt = [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        sortedSequences = [sequences[order[i]] for i, _ in enumerate(sequences)]

        for seq in sortedSequences:
            y += skipAt[y]
            qpCount = -1
            for folder in sorted(os.listdir(jobsdir)):
                if seq in folder:

                    qp = folder.split(seq+'qp')[-1]
                    qpCount += 1

                    # Filename
                    if not qpCount:
                        worksheet.write(offsetY+y*numQPs+qpCount, offsetX, seq.replace('StEM_', '').split('_')[0])

                    # BT2020ref checksum
                    fn = '%s%s%s%sb2020ref_%s_cat.md5' % (jobsdir, os.sep, folder, os.sep, folder)
                    md5checksum = getFirstWordInFile(fn)
                    if not qpCount:
                        worksheet.write(offsetY+y*numQPs+qpCount, offsetX+1, md5checksum)

                    # Preprocessing checksum
                    fn = '%s%s%s%spre_%s.md5' % (jobsdir, os.sep, folder, os.sep, folder)
                    md5checksum = getFirstWordInFile(fn)
                    if not qpCount:
                        worksheet.write(offsetY+y*numQPs+qpCount, offsetX+2, md5checksum)

                    # QP
                    worksheet.write(offsetY+y*numQPs+qpCount, offsetX+5, qp)

                    # Bitstream checksum
                    fn = '%s%s%s%s%s.md5' % (jobsdir, os.sep, folder, os.sep, folder)
                    md5checksum = getFirstWordInFile(fn)
                    worksheet.write(offsetY+y*numQPs+qpCount, offsetX+6, md5checksum)

                    # Dec checksum
                    fn = '%s%s%s%sdec_%s_L0.md5' % (jobsdir, os.sep, folder, os.sep, folder)
                    md5checksum = getFirstWordInFile(fn)
                    worksheet.write(offsetY+y*numQPs+qpCount, offsetX+7, md5checksum)

                    # Postprocessing checksum
                    fn = '%s%s%s%spst_%s_cat.md5' % (jobsdir, os.sep, folder, os.sep, folder)
                    md5checksum = getFirstWordInFile(fn)
                    worksheet.write(offsetY+y*numQPs+qpCount, offsetX+8, md5checksum)

                    # Rec checksum
                    fn = '%s%s%s%srec_%s_L0.md5' % (jobsdir, os.sep, folder, os.sep, folder)
                    md5checksum = getFirstWordInFile(fn)
                    worksheet.write(offsetY+y*numQPs+qpCount, offsetX+10, md5checksum)


            y += 1

        self.workbook.save(xlsFile)
        print "Md5 checksums written to " + xlsFile


    def collectMd5ForHDRSHVC(self, settingsFileBL, settingsFileBLEL1, settingsFileBLEL2, xlsFile, jobsdirBL, jobsdirBLEL1, jobsdirBLEL2):
        sequencesBL  = getSequencesFromSettingsFile(settingsFileBL)
        sequencesBLEL1 = getSequencesFromSettingsFile(settingsFileBLEL1)
        sequencesBLEL2 = getSequencesFromSettingsFile(settingsFileBLEL2)

        worksheet = self.workbook.add_sheet("Cat. 2 SHVC anchors")

        worksheet.write(1, 2, "QPI BL")
        worksheet.write(1, 3, "QPI EL")
        worksheet.write(1, 4, "YCbCr (uncompressed)")
        worksheet.write(1, 5, "Bitstreams SDR single layer SHM7.0")
        worksheet.write(1, 6, "Bitstreams BL+EL HDR SHM7.0")
        worksheet.write(1, 7, "Reconstructed SDR YCbCr after SHM decoding")
        worksheet.write(1, 8, "Reconstructed HDR YCbCr after SHM decoding")


        numQPs = 8
        order = [0,1,2,4,5,6,7,3,8,9,10,11,12,13]
        skipAt = [0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        #skipAt = [1,1,1,1,1,1,1,1,1,2,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        sortedSequencesBL  = [sequencesBL[order[i]] for i, _ in enumerate(sequencesBL)]
        sortedSequencesBLEL1 = [sequencesBLEL1[order[i]] for i, _ in enumerate(sequencesBLEL1)]
        sortedSequencesBLEL2 = [sequencesBLEL2[order[i]] for i, _ in enumerate(sequencesBLEL2)]

        # Bitstreams SDR single layer
        offsetY = 2
        offsetX = 1

        y = 0
        for seq in []:#sortedSequencesBL:
            y += skipAt[y]
            qpCount = -1
            for folder in sorted(os.listdir(jobsdirBL)):
                if seq in folder:
                    qpCount += 1

                    # Filename
                    if not qpCount:
                        worksheet.write(offsetY+y*numQPs+qpCount, offsetX, seq.replace('StEM_', '').split('_')[0])

                    # Bitstream checksum
                    fn = '%s%s%s%s%s.md5' % (jobsdirBL, os.sep, folder, os.sep, folder)
                    md5checksum = getFirstWordInFile(fn)
                    worksheet.write(offsetY+y*numQPs+qpCount, offsetX+4, md5checksum)
            y += 2

        # BL+EL1
        offsetY = 2
        offsetX = 2

        y = 0
        for pattern in ['_AG_', '_CG_']:
            for seq in sortedSequencesBLEL1:
                y += skipAt[y]
                qpCount = -1
                seqWritten = False
                for folder in sorted(os.listdir(jobsdirBLEL1)):
                    if seq in folder and pattern in folder:
                        seqWritten = True

                        (_,qpBL,qpEL) = [s[:2] for s in folder.split('qp')]
                        qpCount += 1

                        # Filename
                        if not qpCount:
                            worksheet.write(offsetY+y*numQPs+qpCount, offsetX-1, seq.replace('StEM_', '').split('_')[0]+pattern[:3])

                        # QP
                        worksheet.write(offsetY+y*numQPs+qpCount, offsetX+0, qpBL)
                        worksheet.write(offsetY+y*numQPs+qpCount, offsetX+1, qpEL)

                        # Bitstream checksum
                        fn = '%s%s%s%s%s.md5' % (jobsdirBLEL1, os.sep, folder, os.sep, folder)
                        md5checksum = getFirstWordInFile(fn)
                        worksheet.write(offsetY+y*numQPs+qpCount, offsetX+4, md5checksum)

                        # Dec checksum L0
                        fn = '%s%s%s%sdec_%s_L0.md5' % (jobsdirBLEL1, os.sep, folder, os.sep, folder)
                        md5checksum = getFirstWordInFile(fn)
                        worksheet.write(offsetY+y*numQPs+qpCount, offsetX+5, md5checksum)

                        # Dec checksum L1
                        fn = '%s%s%s%sdec_%s_L1.md5' % (jobsdirBLEL1, os.sep, folder, os.sep, folder)
                        md5checksum = getFirstWordInFile(fn)
                        worksheet.write(offsetY+y*numQPs+qpCount, offsetX+6, md5checksum)
                if seqWritten:
                    y += 1

        # BL+EL2
        offsetY = 6
        offsetX = 2

        y = 0
        for seq in sortedSequencesBLEL2:
            y += skipAt[y]
            qpCount = -1
            for folder in sorted(os.listdir(jobsdirBLEL2)):
                if seq in folder:

                    (_,qpBL,qpEL) = [s[:2] for s in folder.split('qp')]
                    qpCount += 1

                    # QP
                    worksheet.write(offsetY+y*numQPs+qpCount, offsetX+0, qpBL)
                    worksheet.write(offsetY+y*numQPs+qpCount, offsetX+1, qpEL)

                    # Bitstream checksum
                    fn = '%s%s%s%s%s.md5' % (jobsdirBLEL2, os.sep, folder, os.sep, folder)
                    md5checksum = getFirstWordInFile(fn)
                    worksheet.write(offsetY+y*numQPs+qpCount, offsetX+4, md5checksum)
            y += 2


        # for layer, sortedSequences in [('BL',sortedSequencesBL), ('BLEL1',sortedSequencesEL1), ('BLEL2',sortedSequencesEL2)]:
        #     if layer in ['BL','BLEL1']:
        #         offsetY = 2
        #     else: # EL2
        #         offsetY = 6
        #
        #     if layer in ['BLEL1','BLEL2']:
        #         offsetX = 2
        #     else: # BL
        #         offsetX = 1
        #
        #     y = 0
        #     for seq in sortedSequences:
        #         y += skipAt[y]
        #         qpCount = -1
        #         for folder in sorted(os.listdir(jobsdirBL)):
        #             if seq in folder:
        #
        #                 (_,qpBL,qpEL) = [s[:2] for s in folder.split('qp')]
        #                 qpCount += 1
        #
        #                 # Filename
        #                 if layer == 'BL' and not qpCount:
        #                     worksheet.write(offsetY+y*numQPs+qpCount, offsetX, seq.replace('StEM_', '').split('_')[0])
        #
        #                 # QP
        #                 if layer in ['BLEL1','BLEL2']:
        #                     worksheet.write(offsetY+y*numQPs+qpCount, offsetX+0, qpBL)
        #                     worksheet.write(offsetY+y*numQPs+qpCount, offsetX+1, qpEL)
        #
        #                 # Bitstream checksum
        #                 if layer in ['BL','EL1']:
        #                     fn = '%s%s%s%s%s.md5' % (jobsdir, os.sep, folder, os.sep, folder)
        #                 else: # EL2
        #                     fn = '%s%s%s%s%s.md5' % (jobsdirEL2, os.sep, folder, os.sep, folder)
        #                 md5checksum = getFirstWordInFile(fn)
        #                 worksheet.write(offsetY+y*numQPs+qpCount, offsetX+4, fn)
        #
        #         y += 2

        self.workbook.save(xlsFile)
        print "Md5 checksums written to " + xlsFile



def getSequencesFromSettingsFile(settingsFile):

    #assert(os.path.isfile(settingsFile), "Settings file %s does not exist")

    if settingsFile and os.path.isfile(settingsFile):
        sf = open(settingsFile,'r')
        lines = sf.readlines()
        sf.close()
    else:
        lines = []

    testSequences = []
    for line in lines:
        if line.startswith("testSequences"):
            testSequences = [x[0] for x in eval(line.split('=',1)[1])]

    return testSequences


def getFirstWordInFile(fn):
    if os.path.isfile(fn):
        f = open(fn,'r')
        line = f.readline()
        f.close()
        return line.split(' ')[0]
    else:
        return 'N/A'

def main(argv):
    assert len(argv) > 1, "Unexpected number of parameters. Syntax: %s [simset] settingsFile [settingsFile2 settingsFile3]" % argv[0]

    if argv[1].upper() in ['HDR','HDRanchor','HDRSHVC']:
        simset = argv[1].upper()
        settingsFile = argv[2]
    else:
        simset = 'HDR' # HDR proposal
        settingsFile = argv[1]

    if simset in ['HDR','HDRanchor']:
        jobsdir = 'jobs'+os.sep+settingsFile.split(os.sep)[-1].replace('-L0.txt', '')
        md5ep   = Md5ExcelPlotter()
        xlsFile = settingsFile.rsplit(os.sep,1)[0]+os.sep+'md5_' + settingsFile.rsplit(os.sep,1)[-1].replace('.txt', '.xls')
        if simset == 'HDR':
            md5ep.collectMd5ForHDR(settingsFile, xlsFile, jobsdir)
        else:
            md5ep.collectMd5ForHDRanchor(settingsFile, xlsFile, jobsdir)
    elif simset == 'HDRSHVC':
        md5ep   = Md5ExcelPlotter()
        settingsFileBLEL1 = argv[3]
        settingsFileBLEL2 = None
        jobsdirBL = 'jobs'+os.sep+settingsFile.split(os.sep)[-1].replace('-L0.txt', '')
        jobsdirBLEL1 = 'jobs'+os.sep+settingsFileBLEL1.split(os.sep)[-1].replace('-L1.txt', '')
        jobsdirBLEL2 = None
        if len(argv) > 4:
            settingsFileBLEL2 = argv[4]
            jobsdirBLEL2 = 'jobs'+os.sep+settingsFileBLEL2.split(os.sep)[-1].replace('-L1.txt', '')
        xlsFile = settingsFile.rsplit(os.sep,1)[0]+os.sep+'md5_' + settingsFile.rsplit(os.sep,1)[-1].replace('.txt', '_SHVC.xls')
        md5ep.collectMd5ForHDRSHVC(settingsFile, settingsFileBLEL1, settingsFileBLEL2, xlsFile, jobsdirBL, jobsdirBLEL1, jobsdirBLEL2)

if __name__ == '__main__':
    main (sys.argv)







