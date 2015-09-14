import sys, os

def main(argv):
    assert(len(argv)==3)
    
    fn1 = argv[1]
    fn2 = argv[2]

    print "File 1: %s" % fn1
    print "File 2: %s" % fn2

    f1 = open(fn1, 'r')
    f2 = open(fn2, 'r')

    f1lines = f1.readlines(); f1.close()
    f2lines = f2.readlines(); f2.close()

    nbrMismatch = 0
    for num, f1line in enumerate(f1lines):
        try:
            f2line = f2lines[num]
        except:
            break # The files have different number of rows

        f1linemd5 = f1line.split()[0]
        f2linemd5 = f2line.split()[0]

        if f1linemd5 == f2linemd5:
            print "%4d %s %s Match" % (num, f1linemd5, f2linemd5)
        else:
            nbrMismatch+=1
            print "%4d %s %s Mismatch!!!!!!!!!!!!!!!!!!!!!!!!!!" % (num, f1linemd5, f2linemd5)

    if nbrMismatch:
        print "WARNING: %s mismatches found!" % nbrMismatch
    else:
        print "No mismatches found"

    if not len(f1lines) == len(f2lines):
        print "WARNING: File 1 and 2 have different number of rows!"

if __name__ == '__main__':
    main(sys.argv)
