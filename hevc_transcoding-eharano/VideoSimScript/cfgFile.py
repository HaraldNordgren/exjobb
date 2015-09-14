""" cfgFile config file reader/writer

    Supports text config files with line-based key-value pairs with comments (e.g. "key = value # comment")
    Default key-value delimiter is '='. Default value-comment delimiter is '#'.

>>> Usage examples

1a) Open config file (alternatives)
import cfgFile
c = cfgFile.cfgFile('config.txt',None) # read config file 'config.txt' assuming whitespace as key-value delimiter
c = cfgFile.cfgFile('config.txt','=')  # read config file 'config.txt' assuming '=' as key-value delimiter
c = cfgFile.cfgFile('config.txt')      # read config file 'config.txt' assuming '=' as key-value delimiter (default)

1b) Create empty cfgFile object (alternatives)
import cfgFile
c = cfgFile.cfgFile(None,None) # assuming whitespace as key-value delimiter
c = cfgFile.cfgFile(None,'=')  # assuming '=' as key-value delimiter
c = cfgFile.cfgFile()          # assuming '=' as key-value delimiter (default)

2a) Set/get values for certain key
c['key'] = 1                                # supports both numeric and string values
c['removecod'] = [1,1,1,1]                  # supports vectors
c['embedded'] = [[1,2,3],1,['hello',[1,2]]] # including support for cascaded vectors
print c['key']                              # attention: c['key'] does not support ambiguous keys (multiple key-value pairs with identical keys)
print c['removecod']
print c['embedded']
c.appendEntry('key', 'value')               # alternative to "c['key'] = value"
print c.countEntries('key')                 # returns the number of entries with given key

2b) Delete key-value pairs
c.removeEntries('key')                      # deletes all entries with given key

2c) Add lines with key-value pairs from argument list.
    Call the script using "python script.py key1=value1 key2=value2".
    Make sure to use no whitespaces within key-value pairs.
import cfgFile, sys
c = cfgFile.cfgFile()
c.appendLines(sys.argv[1:])
print c['key1']
print c['key2']

2d) Update key values according to input file
c.update(InputFile)

3) Write config file
c.writeCfgFile('newconfig.txt')             # write config to given file name
c.writeCfgFile()                            # default: same file the config was read from

"""
import os

def isfloat(s):
    try:
        float(s)
        return True
    except ValueError:
        return False

def isint(s):
    if isfloat(s):
        if int(float(s)) == float(s):
            return True
    return False

class cfgFileException(Exception): pass

class cfgFile:

    def __init__(self, inputFile=None, keyValDelim="=", commDelim='#', vecValDelim='[,]', writeKeyValSpaces=True):
        self._list = []
        self._inputFile         = inputFile
        self._keyValDelim       = keyValDelim
        self._commDelim         = commDelim
        self._writeKeyValSpaces = writeKeyValSpaces
        assert len(vecValDelim) == 3
        self._vecValDelim = vecValDelim
        if self._inputFile:
            self.readCfgFile(self._inputFile)

    def _findIndex(self, key):
        if not key:
            return None
        key = key.strip()
        idx = None
        for i in range(len(self._list)):
            if self._list[i][0] == key:
                if idx != None:
                    raise cfgFileException, "no support for amgiuous keys ('%s')" % key
                idx = i
        return idx

    def __getitem__(self, key):
        i = self._findIndex(key)
        if i != None:
            return self._list[i][1]
        else:
            raise cfgFileException, "key not found: " + str(key)

    def __setitem__(self, key, val):
        i = self._findIndex(key)
        if i != None:
            self._list[i][0] = key
            self._list[i][1] = val
        else:
            self.appendEntry(key, val)

    def appendEntry(self, key, val):
#        print "appending %s -> %s" % (key, val)
        self._list.append([key, val, ''])

    def updateEntry(self, key, val):
        if not key:
            return None
        key = key.strip()
        idx = None
        for i in range(len(self._list)):
            if self._list[i][0] == key:
                self._list[i][1] = val


    def removeEntries(self, key):
        if not key:
            return None
        key = key.strip()
        idx = None
        for i in range(len(self._list)-1, -1, -1):
            if self._list[i][0] == key:
                dummy = self._list.pop(i)

    def countEntries(self, key):
        if not key:
            return None
        key = key.strip()
        cnt = 0
        for i in range(len(self._list)):
            if self._list[i][0] == key:
                cnt = cnt + 1
        return cnt

    def hasEntries(self, key):
        return self.countEntries(key) > 0

    def _typify(self, val):
        if not val:
            return val
        elif type(val) == type(''):
            val = val.strip()
            if (val[0] == "'" and val[-1] == "'") or (val[0] == '"' and val[-1] == '"'):
                return eval("'%s'" % val[1:-1])
            elif val[0] == self._vecValDelim[0] and val[-1] == self._vecValDelim[2]:
                val = val[1:-1]
#                print "parsing vector: " + val
                splitval = []
                qtecnt1 = False
                qtecnt2 = False
                veccnt = 0
                i = 0
                while i < len(val):
#                    print "checking '%s' (%d,%d,%d)" % (val[i], veccnt, qtecnt1, qtecnt2)
                    if val[i] == "'" and (not qtecnt2):
                        qtecnt1 = not qtecnt1
                    elif val[i] == '"' and (not qtecnt1):
                        qtecnt2 = not qtecnt2
                    elif (not qtecnt1) and (not qtecnt2):
                        if val[i] == self._vecValDelim[0]:
                            veccnt = veccnt + 1
                        elif val[i] == self._vecValDelim[2]:
                            veccnt = veccnt - 1
                            if veccnt < 0:
                                raise cfgFileException, "illegal config file format: " + val
                        elif (val[i] == self._vecValDelim[1]) and (not veccnt):
#                            print "adding to splitval: '%s' -> '%s'" % (val[0:i], val[i+1:])
                            splitval.append(self._typify(val[0:i]))
                            val = val[i+1:]
                            i = -1
                    i = i + 1
                if qtecnt1 or qtecnt2:
                    raise cfgFileException, "unclosed quotes detected"
                if veccnt:
                    raise cfgFileException, "unclosed vector detected"
#                print "final adding to splitval: '%s'" % val
                splitval.append(self._typify(val))
                return splitval
            else:
                if isint(val):
                    return int(float(val))
                elif isfloat(val):
                    return float(val)
        return val

    def appendLines(self, lines):
        for line in lines:
            key = ''
            val = ''
            comm = ''
            line = line.replace('\r','').replace('\n','')
            splitline = line.split(self._commDelim, 1)
            commLJust = len(splitline[0])==0
            if splitline:
                keyVal = splitline[0].strip()
                if len(splitline) > 1:
                    assert len(splitline)==2
                    comm = self._commDelim + splitline[1]
                splitkeyval = keyVal.split(self._keyValDelim, 1)
#                print "splitkeyval: " + str(splitkeyval)
                if splitkeyval:
                    key = splitkeyval[0].strip()
                    if len(splitkeyval) > 1:
                        assert len(splitkeyval)==2
                        val = self._typify(splitkeyval[1].strip())
#                        print val
            self._list.append([key, val, comm, commLJust])
#            print "appending %s -> %s (%s)" % (key, str(val), comm)

    def readCfgFile(self, inputFile):
        self._inputFile = inputFile
        f = open(self._inputFile, 'r')
        self._list = []
        for line in f:
            self.appendLines([line])
        f.close()

    def getSize(self):
        return len(self._list)

    def getKey(self,num):
        return self._list[num][0]

    # updates configuration according to an input file
    def update(self, inputFile):
        isFile = os.path.isfile(inputFile)
        if isFile:
          confSeq = cfgFile(inputFile, self._keyValDelim, self._commDelim, self._vecValDelim)
          num = confSeq.getSize()

          for i in range(num):
            key = confSeq.getKey(i)
            if key!='':
              self[key]=confSeq[key]

    def writeCfgFile(self, outputFile=None):
        if not outputFile:
            if self._inputFile:
                outputFile = self._inputFile
            else:
                raise cfgFileException, "no file name specified"
        if self._keyValDelim:
            keyValDelim = ' ' + self._keyValDelim.strip() + ' '
        else:
            keyValDelim = ' '
        maxKeyLen = 0
        maxValLen = 0
        for element in self._list:
            maxKeyLen = max(maxKeyLen, len(element[0]))
            if len(element[2]) > 0:
                maxValLen = max(min(maxKeyLen+3, maxValLen), len(str(element[1])))
        f = open(outputFile, 'w')

        for element in self._list:
            commLJust = True
            if len(element) > 3:
                commLJust = element[3]
            if element[0]!='':
                if self._writeKeyValSpaces:
                    f.write(element[0].ljust(maxKeyLen) + keyValDelim + str(element[1]).ljust(maxValLen) + " " + element[2] + "\n")
                else:
                    f.write(element[0] + keyValDelim.strip() + str(element[1]).ljust(maxKeyLen-len(element[0])+maxValLen+len(keyValDelim)-len(keyValDelim.strip())+1) + element[2] + "\n")
            elif not commLJust:
                f.write("".ljust(maxKeyLen+len(keyValDelim)+maxValLen) + " " + element[2] + "\n")
            else:
                f.write(element[2] + "\n")
        f.close()


