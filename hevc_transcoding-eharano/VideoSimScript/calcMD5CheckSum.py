import sys, os, subprocess

# Script for calculating md5sums for one file or all files in a folder. A md5-file is created
# with the md5sums listed together with the filenames


def calcMD5ForFile(fn, dest=None):
    md5sumfn = fn.rsplit('.')[0] + '.md5'
    if dest:
        md5sumfn = dest+os.sep+md5sumfn.split(os.sep)[-1]
    md5sumf = open(md5sumfn,'w')

    md5sumString = subprocess.check_output(["md5sum", fn])

    # Nicify
    md5sumString = "%s  %s" % (md5sumString.split()[0],fn.split(os.sep)[-1])

    print md5sumString
    md5sumf.write(md5sumString+'\n')

    md5sumf.close()


def calcMD5ForFolder(fldr, dest=None):
    md5sumfn = fldr + '.md5'
    if dest:
        md5sumfn = dest+os.sep+md5sumfn.split(os.sep)[-1]
    md5sumf = open(md5sumfn,'w')

    for f in os.listdir(fldr):
        fullpath = fldr+os.sep+f
        md5sumString = subprocess.check_output(["md5sum", fullpath])

        # Nicify
        md5sumString = "%s  %s" % (md5sumString.split()[0],f) 

        print md5sumString
        md5sumf.write(md5sumString+'\n')                              

    md5sumf.close()

def calcMD5ForCatenatedFile(exp, dest=None):
    md5sumfn = exp.rsplit('.')[0].replace('*','cat') + '.md5'
    #md5sumfn = exp.rsplit(os.sep,1)[0] + '_cat.md5'
    if dest:
        md5sumfn = dest+os.sep+md5sumfn.split(os.sep)[-1]

    import glob
    files = glob.glob(exp)
    args = ['cat']; args.extend(files)

    if files:
        p1           = subprocess.Popen(args, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
        md5sumString = subprocess.check_output(["md5sum"],stdin=p1.stdout)

        # Nicify
        md5sumString = "%s  %s" % (md5sumString.split()[0],exp.split(os.sep)[-1])

        md5sumf = open(md5sumfn, 'w')
        print md5sumString
        md5sumf.write(md5sumString+'\n')
        md5sumf.close()


def createMD5List(path, dest=None):
    base, ext = os.path.splitext(path)
    base = base.rsplit(os.sep,1)[0]
    md5listfn = base +'.md5'
    if dest:
        md5listfn = dest+os.sep+md5listfn.split(os.sep)[-1]
    md5listf = open(md5listfn,'w')
    for p in sorted(os.listdir(base)):
        filepath = base+os.sep+p
        if filepath.endswith('.md5'):
            f = open(filepath, 'r')
            md5listf.writelines(f.readlines())
            f.close()
    md5listf.close()


def calcMD5Sum(path, dest=None):
    if os.path.isdir(path):
        calcMD5ForFolder(path,dest)
    elif os.path.isfile(path):
        calcMD5ForFile(path,dest)
        #createMD5List(path,dest)
    elif '*' in path:
        calcMD5ForCatenatedFile(path,dest)
        #createMD5List(path,dest)
        #createMD5List(path.rsplit(os.sep,1)[0],dest)
    elif os.path.isdir(path.rsplit(os.sep,1)[0]): # Calculating md5sum for the files folder
        calcMD5ForFolder(path.rsplit(os.sep,1)[0],dest)
    else:
        raise Exception("%s is not an accurate file or a folder" % path)

def main(argv):
    assert(len(argv)>1)
    argv = ['', 'test/test_*.exr']
    calcMD5Sum(argv[1])


if __name__ == '__main__':
    main(sys.argv)
