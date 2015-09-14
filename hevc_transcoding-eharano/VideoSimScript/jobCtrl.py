import os, tempfile, re, sys, getopt, stat, subprocess, posixpath, platform, math, shutil

""" jobCtrl classes for (LSF) job management

    Keep track of jobs that have been started on the LSF cluster.
    Delete all running jobs of a certain group.
    Submit jobs to be run either on local machine or on LSF cluster.
    Submit jobs either from Windows or Linux.

    For jobs to be considered as a group of jobs, all of them have to be started
    from the same working directory. The script creates a "job root" subdirectory
    ("jobs/" if not otherwise specified), under which it saves information to keep
    track of the jobs. For each job, associated data (such as stdout and stderr)
    are kept in a specific subdirectory under the "job root" directory.
    
>>> Usage example: Submitting jobs from within a Python script (to be executed on the LSF cluster)

import jobCtrl
jc = jobCtrl.jobCtrl()
job1 = jc.submitJob( "command1 (e.g. encoder call)" )
job2 = jc.submitJob( "command2 (e.g. decoder call)", None, job1 )  # job2 is not started until job1 is done
jc.printStatus()

>>> Type of job can be modified by changing only the jobCtrl object instantiation (second line in example above)

jc = jobCtrl.jobCtrl(jobCtrl.lsfJob)        # same as "jc = jobCtrl.jobCtrl()"
jc = jobCtrl.jobCtrl(jobCtrl.simpleJob)     # will start all jobs on local machine (works on either Windows or Linux)
jc = jobCtrl.jobCtrl(jobCtrl.remoteLsfJob)  # will start all jobs on LSF remotely from within Python script that run on Windows machine
                                            # this requires installation of "putty.exe" and "plink.exe" with profile "lsflingate" to access LSF cluster
                                            # functionality is available only if script is executed under project area (\\rndsmb.rnd.ki.sw.ericsson.se\Projects)

>>> Adapt behaviour of jobCtrl

jc = jobCtrl.jobCtrl(jobCtrl.lsfJob,"jobroot")        # specify "jobroot" directory to keep track of jobs (default: None -> "jobs")
jc = jobCtrl.jobCtrl(jobCtrl.lsfJob,None,True)        # be verbose (default: False)

>>> Configuration for jobs to be run on LSF cluster (jobType=jobCtrl.lsfJob or jobType=jobCtrl.remoteLsfJob), platform, queue and email behaviour can be adapted

jc = jobCtrl.jobCtrl(jobCtrl.lsfJob,None,False,'linux')                   # specify target platform ('linux' (default) or 'solaris', or more specifically 'intel' (linux) or 'amd' (linux))
jc = jobCtrl.jobCtrl(jobCtrl.lsfJob,None,False,'linux','sim_24h')         # specify target queue ('sim' (default), 'sim_24h', 'sim_high', 'max15min', 'max30min')
jc = jobCtrl.jobCtrl(jobCtrl.lsfJob,None,False,'linux','sim_24h',False)   # True (default): send script output as email. False: store script output in job directory, send mail with links.

>>> "bqueues -u er3" (on linux command line) gives information about load of the queues (sim, sim_24h, etc.)

>>> Getting job status / killing all running jobs from Linux command line (executed in the directory where the jobs were started)

python jobCtrl.py --status
python jobCtrl.py --killAll

>> These commands can also be used remotely - e.g. executed in Windows to kill jobs on LSF cluster
   To do so, pass option "-x" to jobCtrl.py.

python jobCtrl.py -x --killAll

"""

class jobException(Exception): pass
class jobStatusFileException(Exception): pass

class job:
    (_NOTSUBMITTED, _SUBMITTED, _RUNNING, _DONE, _KILLED, _ERROR, _UNKNOWN) = range(7)
    _statusNames = {_NOTSUBMITTED : 'notsubmitted',\
                    _SUBMITTED    : 'submitted',\
                    _RUNNING      : 'running',\
                    _DONE         : 'done',\
                    _KILLED       : 'killed',\
                    _ERROR        : 'error',\
                    _UNKNOWN      : 'unknown'}

    def __init__(self, jobName=None, verbose=False):
        """creates directory for job specific files"""
        self._verbose = verbose
        curDir = os.getcwd()
        if jobName:
            self._jobDir = os.path.join(curDir, jobName)
            self._jobDirRelative = jobName
            self._statusFile = os.path.join(self._jobDir, self._statusFileName)
            if os.path.isdir(self._jobDir):
                if self._verbose: print "job dir exists: trying to read status file " + self._statusFile
                self._readStatusFile()
            else:
                os.mkdir(self._jobDir)
                if self._verbose: print "new job: created job dir " + str(self._jobDir)
                self._status = self._NOTSUBMITTED
                self._writeStatusFile()
        else:
            self._jobDir = tempfile.mkdtemp(prefix='',suffix='',dir=curDir)
            self._jobDirRelative = os.path.basename(self._jobDir)
            if self._verbose: print "new job: created job dir " + str(self._jobDir)
            self._statusFile = os.path.join(self._jobDir, self._statusFileName)
            self._status = self._NOTSUBMITTED
            self._writeStatusFile()
        self._jobScriptFile = os.path.join(self._jobDir, self._jobScriptFileName)
        self._stdOutFile = os.path.join(self._jobDir, self._stdOutFileName)
        self._stdErrFile = os.path.join(self._jobDir, self._stdErrFileName)
        
    def submit(self, command, tmpMegaBytes=0, *refJobs):
        """submit job to job system. tmpMegaBytes specifies required disk space on /tmp dir. refJobs specifies jobs to wait for"""
        
        #hej
        #print "******** job submitted, %f megabytes requested" % tmpMegaBytes
        
        if self.isSubmitted():
            raise jobException, "job was already submitted"
        f = open(self._jobScriptFile,'wb')
        f.write(self._scriptPrefix)
        f.write(command+'\n')
        f.write(self._scriptPostfix)
        f.close()
        os.chmod(self._jobScriptFile, stat.S_IREAD|stat.S_IWRITE|stat.S_IEXEC)
        self._submitScriptFile(tmpMegaBytes, *refJobs)
        
    def kill(self):
        """kill job if submitted or running"""
#        self._updateStatus()
        if (not self.isSubmitted()) or self.isEnded():
            if self._verbose: print "WARNING - job not running: " + self._jobDir
            return
        self._kill()
        self._status = self._KILLED
        self._writeStatusFile()
        print "job killed: " + self._jobDir

    def isSubmitted(self,update=True):
        if update:
            self._updateStatus()        
        return self._status >= self._SUBMITTED
    
    def isRunning(self,update=True):
        if update:
            self._updateStatus()        
        return self._status == self._RUNNING

    def isDone(self,update=True):
        if update:
            self._updateStatus()        
        return self._status == self._DONE

    def isEnded(self,update=True):
        if update:
            self._updateStatus()        
        return self._status >= self._DONE
#        return self._status == self._DONE or self._status == self._KILLED or self._status == self._ERROR

    def getStatus(self,update=True):
        if update:
            self._updateStatus()        
        return os.path.basename(self._jobDir) + ": " + self._statusNames[self._status]

    def getJobDir(self):
        return self._jobDir

    def getJobDirRelative(self):
        return self._jobDirRelative

    def getDestinationCwd(self):
        return os.getcwd()

    def getDestinationJobDir(self):
        cwd = os.getcwd()
        os.chdir(self.getJobDir())
        result = self.getDestinationCwd()
        os.chdir(cwd)
        return result
        
    def getDestinationPlatform(self):
        return platform.system().lower()


class simpleJob(job):
    _scriptPrefix  = ""
    _scriptPostfix = ""
    _statusFileName    = 'simpleJobStatus.txt'
    _jobScriptFileName = 'simpleJobScript.bat'
    _stdOutFileName    = 'simpleJobStdOut.txt'
    _stdErrFileName    = 'simpleJobStdErr.txt'
    _localTmpDir = ''
    
    def __init__(self, jobName=None, verbose=False):
        job.__init__(self, jobName, verbose)
        if not self._localTmpDir:
            self._localTmpDir = tempfile.mkdtemp("_simpleJob")
            self._writeStatusFile()
        
    def _submitScriptFile(self, tmpMegaBytes, *refJobs):
        if len(refJobs):
            for job in refJobs:
                if not job.isSubmitted():
                    raise jobException, "reference job '" + job._jobDir + "' must be submitted first"
                if job.isRunning():
                    raise jobException, "reference job '" + job._jobDir + "' is running, unexpectedly"
                if self._verbose: print "WARNING - ignoring reference job: " + job._jobDir
        if tmpMegaBytes > 0:
            print "WARNING - ignoring requested tmpMegaBytes (%f)" % tmpMegaBytes
        print "starting job: " + self._jobDir
        try:
            fstdout = open(self._stdOutFile,'w')
            fstderr = open(self._stdErrFile,'w')
            p = subprocess.Popen(self._jobScriptFile, shell=True, stdout=fstdout, stderr=fstderr)
            if p.wait() == 0:
                self._status = self._DONE
                if self._verbose: print "job done"
            else:
                self._status = self._ERROR
                print ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ERROR WHILE EXECUTING JOB <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
            fstdout.close()
            fstderr.close()
        except:
            raise jobException, "error while executing job " + self._jobDir
        shutil.rmtree(self._localTmpDir)
        self._writeStatusFile()

    def _kill(self):
        pass

    def _updateStatus(self):
        pass
                       
    def _readStatusFile(self):
        try:
            f = open(self._statusFile,'r')
            self._status = int(f.readline())
            if self._status < 0 or self._status > len(self._statusNames)-1:
                raise jobStatusFileException, "illegal file format in status file " + self._statusFile
            self._localTmpDir = f.readline().strip()
            f.close()
        except IOError:
            raise jobStatusFileException, "error reading status file " + self._statusFile
    
    def _writeStatusFile(self):
        try:
            f = open(self._statusFile,'w')
            f.write(str(self._status) + '\n')
            f.write(self._localTmpDir + '\n')
            f.close()
        except IOError:
            raise jobStatusFileException, "error writing status file " + self._statusFile

    def getDestinationTmpDir(self):
        return self._localTmpDir

    
class lsfJob(job):
    _lsfState2status = {'PEND'  : job._SUBMITTED, \
                        'PSUSP' : job._SUBMITTED, \
                        'RUN'   : job._RUNNING, \
                        'USUSP' : job._RUNNING, \
                        'SSUSP' : job._RUNNING, \
                        'DONE'  : job._DONE, \
                        'EXIT'  : job._ERROR, \
                        'UNKWN' : job._UNKNOWN, \
                        'WAIT'  : job._SUBMITTED, \
                        'ZOMBI' : job._UNKNOWN}    
    _scriptPrefix  = "#!/bin/bash\necho date: `date`\necho system: `uname -a`\necho working directory: `pwd`\necho -----------------------------\n"
    _scriptPostfix = "STAT=$?\necho -----------------------------\ndate\nexit $STAT\n"
    _bsubCmd  = 'bsub' 
    _bjobsCmd = 'bjobs'
    _bkillCmd = 'bkill'
    _lsfPlatformMap = {'linux':'linux', 'intel':'linux', 'amd':'linux', 'solaris':'solaris'}
    _lsfPlatformDefault = 'linux'
    _lsfQueues = ('sim', 'sim_24h', 'sim_high', 'max15min', 'max30min')
    _lsfTmpDir = '/tmp/$LSB_JOBID.tmpdir'
    _statusFileName    = 'lsfJobStatus.txt'
    _jobScriptFileName = 'lsfJobScript.sh'
    _stdOutFileName    = 'lsfJobStdOut.txt'
    _stdErrFileName    = 'lsfJobStdErr.txt'

    def __init__(self, jobName=None, verbose=False, lsfPlatform=None, lsfQueue=None, lsfSendOutputAsMail=True):
        if not lsfPlatform:
            self._lsfPlatform = self._lsfPlatformDefault
        elif not (lsfPlatform in self._lsfPlatformMap.keys()):
            raise jobException, "specified platform is not in supported platforms list " + str(self._lsfPlatformMap.keys())
        else:
            self._lsfPlatform = lsfPlatform
        if not lsfQueue:
            self._lsfQueue = self._lsfQueues[0]
        elif not (lsfQueue in self._lsfQueues):
            raise jobException, "specified queue is not in supported queues list " + str(self._lsfQueues)
        else:
            self._lsfQueue = lsfQueue
        self._lsfSendOutputAsMail = lsfSendOutputAsMail
        self._lsfId = None
        job.__init__(self, jobName, verbose)

    def _command(self, cmdString):
#        print "sending command: " + cmdString
        try:
            proc = subprocess.Popen(cmdString, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
            status = proc.wait()
            out = proc.stdout.read()
            err = proc.stderr.read()
        except Exception:
            raise jobException, "error while executing command '" + cmdString + "'"
        return status, out, err

    def _getBsubCommand(self, extraResource, extraArgs):
        if not self._lsfSendOutputAsMail:
            extraArgs = extraArgs + " -o " + self._stdOutFile + " -e " + self._stdErrFile + " -N"
#        print "bsub command = %s" % self._bsubCmd + " -R 'select[" + self._lsfPlatform + "] " + extraResource + "' -q " + self._lsfQueue  + " " + extraArgs + " " + self._jobScriptFile
        return self._bsubCmd + " -R 'select[" + self._lsfPlatform + "] " + extraResource + "' -q " + self._lsfQueue  + " " + extraArgs + " " + self._jobScriptFile
                
    def _submitScriptFile(self, tmpMegaBytes, *refJobs):
        tmpMegaBytes = int(math.ceil(tmpMegaBytes))
        if tmpMegaBytes > 0:
            extraResource = "rusage[tmp=%d]" % tmpMegaBytes
        else:
            extraResource = ''
        extraArgs = ''
        if len(refJobs):
            for job in refJobs:
                if not job.isSubmitted():
                    raise jobException, "reference job '" + job._jobDir + "' must be submitted first"
                if not extraArgs:
                    extraArgs = "-w 'done(" + str(job._lsfId) + ")"
                else:
                    extraArgs = extraArgs + " && done(" + str(job._lsfId) + ")"
                if self._verbose: print "considering reference job (" + str(job._lsfId) + "): " + job._jobDir
            extraArgs = extraArgs + "'"
        status, out, err = self._command(self._getBsubCommand(extraResource, extraArgs))
        if status != 0:
            raise jobException, "bsub returned error code " + str(status)
        self._lsfId = int(re.search('\D(\d+)\D',out).group(1))
        self._status = self._SUBMITTED
        self._writeStatusFile()
        print "job submitted (%s,%s,%dMb,%d): %s" % (self._lsfPlatform, self._lsfQueue, tmpMegaBytes, self._lsfId, self._jobDir)

    def _kill(self):
        status, out, err = self._command(self._bkillCmd + " " + str(self._lsfId))
        if status != 0:
            raise jobException, "bkill returned error code " + str(status)

    def getStatus(self,update=True):
        return job.getStatus(self) + " (" + str(self._lsfId) + ")"

    def _updateStatus(self):
        if not self.isSubmitted(update=False) or self.isEnded(update=False): return # not submitted or already done
        status, out, err = self._command(self._bjobsCmd + " " + str(self._lsfId))
        if status == 0:
            outLines = out.splitlines()
            if (len(outLines) == 0) and ("not found" in err):
                print "%s: job id %d not found. Setting job status to 'unknown'." % (self._jobDir, self._lsfId)
                self._status = job._UNKNOWN
                self._writeStatusFile()
            elif len(outLines) != 2:
                raise jobException, "bjobs returned unexpected output\n" + out
            else:
                statusWords = outLines[1].split()
                if int(statusWords[0]) != self._lsfId:
                    raise jobException, "bjobs returned unexpected job ID " + statusWords[0] + ", expected " + str(self._lsfId)
                if len(statusWords) < 3:
                    raise jobException, "bjobs returned unexpected output\n" + out
                lsfState = statusWords[2]
                self._status = self._lsfState2status[lsfState]
                self._writeStatusFile()
        elif (status == 255) and ("not found" in err):
            print "%s: job id %d not found. Setting job status to 'unknown'." % (self._jobDir, self._lsfId)
            self._status = job._UNKNOWN
            self._writeStatusFile()
        else:
            raise jobException, "bjobs returned unexpected error code " + str(status)
                       
    def _readStatusFile(self):
        try:
            f = open(self._statusFile,'r')
            self._status = int(f.readline())
            if self._status < 0 or self._status > len(self._statusNames)-1:
                raise jobStatusFileException, "illegal file format in status file " + self._statusFile
            if self.isSubmitted(False):
                self._lsfId = int(f.readline())
            else:
                self._lsfId = None
            f.close()
        except IOError:
            raise jobStatusFileException, "error reading status file " + self._statusFile
    
    def _writeStatusFile(self):
        try:
            f = open(self._statusFile,'w')
            f.write(str(self._status) + '\n')
            if self._lsfId:
                f.write(str(self._lsfId) + '\n')
            f.close()
        except IOError:
            raise jobStatusFileException, "error writing status file " + self._statusFile

    def getDestinationPlatform(self):
        """ returns platform type ('linux' or 'solaris') """
        return self._lsfPlatformMap[self._lsfPlatform]

    def getDestinationTmpDir(self):
        return self._lsfTmpDir

'''
class geJob(job):
    _geState2status = {'WAITING' : job._SUBMITTED, \
                       'RUNNING' : job._RUNNING}
    _scriptPrefix  = "#!/bin/bash\necho date: `date`\necho system: `uname -a`\necho working directory: `pwd`\necho -----------------------------\n"
    _scriptPostfix = "STAT=$?\necho -----------------------------\ndate\nexit $STAT\n"
    _qsubCmd = 'qsub' 
    _qstatCmd = 'qstat'
    _qdelCmd = 'qdel'
    _gePlatformMap = {'linux':'linux'}
    _gePlatformDefault = 'linux'
    _geQueues = ('sim', 'sim_24h', 'sim_high', 'max15min', 'max30min')
    _geTmpDir = '/tmp/$LSB_JOBID.tmpdir'
    _statusFileName    = 'geJobStatus.txt'
    _jobScriptFileName = 'geJobScript.sh'
    _stdOutFileName    = 'geJobStdOut.txt'
    _stdErrFileName    = 'geJobStdErr.txt'

    def __init__(self, jobName=None, verbose=False, gePlatform=None, geQueue=None, geSendOutputAsMail=True):
        if not gePlatform:
            self._gePlatform = self._gePlatformDefault
        elif not (gePlatform in self._gePlatformMap.keys()):
            raise jobException, "specified platform is not in supported platforms list " + str(self._gePlatformMap.keys())
        else:
            self._gePlatform = gePlatform
        if not geQueue:
            self._geQueue = self._geQueues[0]
        elif not (geQueue in self._geQueues):
            raise jobException, "specified queue is not in supported queues list " + str(self._geQueues)
        else:
            self._geQueue = geQueue
        self._geSendOutputAsMail = geSendOutputAsMail
        if not self._geSendOutputAsMail:
            raise jobException, "grid engine does not support sending job output as mail"
        self._geId = None
        job.__init__(self, jobName, verbose)

    def _command(self, cmdString):
#        print "sending command: " + cmdString
        try:
            proc = subprocess.Popen(cmdString, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
            status = proc.wait()
            out = proc.stdout.read()
            err = proc.stderr.read()
        except Exception:
            raise jobException, "error while executing command '" + cmdString + "'"
        return status, out, err

    def _getQsubCommand(self, extraResource, extraArgs):
        extraArgs = extraArgs + " -o " + self._stdOutFile + " -e " + self._stdErrFile + " -me"
        return self._qsubCmd + " -q " + self._geQueue  + " " + extraArgs + " " + self._jobScriptFile
                
    def _submitScriptFile(self, tmpMegaBytes, *refJobs):
        if tmpMegaBytes > 0:
            print "WARNING - ignoring requested tmpMegaBytes (%f)" % tmpMegaBytes
        extraResource = ''
        extraArgs = ''
        if len(refJobs):
            for job in refJobs:
                if not job.isSubmitted():
                    raise jobException, "reference job '" + job._jobDir + "' must be submitted first"
                if job.isRunning():
                    raise jobException, "reference job '" + job._jobDir + "' is running, unexpectedly"
                if self._verbose: print "WARNING - ignoring reference job: " + job._jobDir
        status, out, err = self._command(self._getQsubCommand(extraResource, extraArgs))
        if status != 0:
            raise jobException, "qsub returned error code " + str(status)
        self._geId = int(re.search('\D(\d+)\D',out).group(1))
        self._status = self._SUBMITTED
        self._writeStatusFile()
        print "job submitted (%s,%s,%dMb,%d): %s" % (self._gePlatform, self._geQueue, tmpMegaBytes, self._geId, self._jobDir)

    def _kill(self):
        status, out, err = self._command(self._qdelCmd + " " + str(self._geId))
        if status != 0:
            raise jobException, "qdel returned error code " + str(status)

    def getStatus(self,update=True):
        return job.getStatus(self) + " (" + str(self._geId) + ")"

    def _updateStatus(self):
        if not self.isSubmitted(update=False) or self.isEnded(update=False): return # not submitted or already done
        status, out, err = self._command(self._qstatCmd)
        if status == 0:
            outLines = out.splitlines()
            foundWords = None
            for line in outLines:
                words = line.split()
                if (len(words) == 5) and (int(words[2]) == self._geId) and (words[4] in self._geState2status.keys()):
                    if foundWords is not None:
                        raise jobException, "ambiguous output of qstat experienced"
                    foundWords = words
            if foundWords is None:
                print "%s: job id %d not found. Setting job status to 'unknown'." % (self._jobDir, self._geId)
                self._status = job._UNKNOWN
            else:
                geState = foundWords[4]
                self._status = self._geState2status[geState]
            self._writeStatusFile()
        else:
            raise jobException, "qstat returned error code " + str(status)
                       
    def _readStatusFile(self):
        try:
            f = open(self._statusFile,'r')
            self._status = int(f.readline())
            if self._status < 0 or self._status > len(self._statusNames)-1:
                raise jobStatusFileException, "illegal file format in status file " + self._statusFile
            if self.isSubmitted(False):
                self._geId = int(f.readline())
            else:
                self._geId = None
            f.close()
        except IOError:
            raise jobStatusFileException, "error reading status file " + self._statusFile
    
    def _writeStatusFile(self):
        try:
            f = open(self._statusFile,'w')
            f.write(str(self._status) + '\n')
            if self._geId:
                f.write(str(self._geId) + '\n')
            f.close()
        except IOError:
            raise jobStatusFileException, "error writing status file " + self._statusFile

    def getDestinationPlatform(self):
        """ returns platform type ('linux' or 'solaris') """
        return self._gePlatformMap[self._gePlatform]

    def getDestinationTmpDir(self):
        return self._geTmpDir
'''

class remoteLsfJob(lsfJob):
    _winRoots = [r'\\rndsmb.rnd.ki.sw.ericsson.se\Projects',r'\\rndsmb101.rnd.ki.sw.ericsson.se\Projects']
    _linRoot  = r'/proj'
    _plinkCmd = "plink -load lsflingate"
    _netCmd   = "net use"
    
    def __init__(self, jobName=None, verbose=False, lsfPlatform=None, lsfQueue=None, lsfSendOutputAsMail=True):
        self._linDir = self.getDestinationCwd()
        lsfJob.__init__(self, jobName, verbose, lsfPlatform, lsfQueue, lsfSendOutputAsMail)
        self._linJobDir = posixpath.join(self._linDir, self._jobDirRelative)
        self._linJobScriptFile = posixpath.join(self._linJobDir, self._jobScriptFileName)
        self._linStdOutFile = posixpath.join(self._linJobDir, self._stdOutFileName)
        self._linStdErrFile = posixpath.join(self._linJobDir, self._stdErrFileName)

    def submit(self, command, tmpMegaBytes=0, *refJobs):
        self._linDir = self.getDestinationCwd()
        job.submit(self, command, tmpMegaBytes, *refJobs)
        
    def _command(self, cmdString):
        cmdString = self._plinkCmd + ' "cd ' + self._linDir + ' && ' + cmdString + '"'
        return lsfJob._command(self, cmdString)

    def _getBsubCommand(self, extraResource, extraArgs):
        if not self._lsfSendOutputAsMail:
            extraArgs = extraArgs + " -o " + self._linStdOutFile + " -e " + self._linStdErrFile + " -N"
        return self._bsubCmd + " -R 'select[" + self._lsfPlatform + "] " + extraResource + "' -q " + self._lsfQueue  + " " + extraArgs + " " + self._linJobScriptFile

    def getDestinationCwd(self):
        """ find equivalent linux path for current working path (works on project drive only) """
        if platform.system() != 'Windows':
            raise jobException, "class remoteLsfJob must be used on Windows"
        cwd = os.getcwd()
        unc, rest = os.path.splitunc(cwd)
        if not unc:
            """ get network location using net command """
            drive, rest = os.path.splitdrive(cwd)
            status, out, err = lsfJob._command(self, self._netCmd)
            if status:
                raise jobException, "net command returned error"                
            unc = None
            lines = out.splitlines()
            for line in lines:
                lineWords = line.split()
                if len(lineWords) >= 2:
                    if os.path.normpath(lineWords[0]) == os.path.normpath(drive):
                        unc = lineWords[1]
                        break
            if not unc:
                raise jobException, "remoteLsfJob cannot be operated from current working directory - change to " + self._winRoots[0]
        for i in range(len(self._winRoots)):
            self._winRoots[i] = os.path.normpath(self._winRoots[i])
        unc = os.path.normpath(unc)
        if not (unc in self._winRoots):
            raise jobException, "remoteLsfJob cannot be operated from current working directory - change to " + self._winRoots[0]
        if rest[0] == '\\':
            rest = rest[1:]
        rest = rest.replace('\\','/')
        self._linRoot = posixpath.normpath(self._linRoot)
        return posixpath.join(self._linRoot, rest)


class jobCtrl:
    _jobRootDefault='jobs'
    
    def __init__(self, jobType=lsfJob, jobRoot=None, verbose=False, *otherArgs):
        self._jobType = jobType
        self._verbose = verbose
        self._otherArgs = otherArgs
        if not jobRoot:
            jobRoot = self._jobRootDefault
        self._jobRootRelative = jobRoot
        curDir = os.getcwd()
        self._jobRoot = os.path.join(curDir, jobRoot)
        self._nonJobDirs = []
        if self._verbose: print "initializing jobCtrl with job root " + self._jobRoot
        self._jobs = []
        if os.path.isdir(self._jobRoot):
            if self._verbose: print "looking for existing jobs"
            self._initJobs()
        else:
            if self._verbose: print "creating job root"
            os.makedirs(self._jobRoot)

    def getNewJob(self, jobName=None):
        """get new job object"""
        curDir = os.getcwd()
        os.chdir(self._jobRoot)
        job = self._jobType(jobName,self._verbose,*self._otherArgs)
        os.chdir(curDir)
        self._jobs.append(job)
        return job

    def submitJob(self, command, jobName=None, tmpMegaBytes=0, *refJobs):
        """create new job object and submit command"""
        curDir = os.getcwd()
        os.chdir(self._jobRoot)
        job = self._jobType(jobName,self._verbose,*self._otherArgs)
        os.chdir(curDir)
        self._jobs.append(job)
        job.submit(command, tmpMegaBytes, *refJobs)
        return job
    
    def killAllJobs(self):
        """kill all managed jobs"""
        for job in self._jobs:
            job.kill()
    
    def printStatus(self):
        """print status of all managed jobs"""
        for job in self._jobs:
            print job.getStatus()
            
    def _initJobs(self):
        """look for job directories, create array of inspected directories that are no job directories"""
        for d in os.listdir(self._jobRoot):
            jobDir = os.path.join(self._jobRoot, d)
            if os.path.isdir(jobDir):
                try:
                    job = self._jobType(jobDir,self._verbose,*self._otherArgs)
                    self._jobs.append(job)
                except jobStatusFileException:
                    if self._verbose: print "WARNING - Found illegal dir in job root: " + jobDir
                    self._nonJobDirs.append(self._jobRoot + '/' + d)

    def getJobRoot(self):
        return self._jobRoot

    def getJobRootRelative(self):
        return self._jobRootRelative

    def getNonJobDirs(self):
        return self._nonJobDirs


def usage():
    print "jobCtrl.py [-r root] [-n name] [-x] [-v] (--submit script|--killAll|--status)"
    print "commands"
    print "  --submit script    create job dir in job root dir and submit job"
    print "  --killAll          kill all jobs under job root"
    print "  --status           print status for all jobs under job root"
    print "options"
    print "  -r root            specify root directory for jobs (under current working directory) [default: 'jobs/']"
    print "  -d                 do not check for job directories recursively"
    print "  -n name            specify job name (with --submit) [default: random name]"
    print "  -x                 submit job remotely on lsf cluster"
    print "  -v                 be verbose"
    sys.exit(2)


def killRecursive(jobType, jobRoot, verbose):
    jc = jobCtrl(jobType, jobRoot, verbose)
    jc.killAllJobs()
    for newRoot in jc.getNonJobDirs():
        print "---------------------------------------"
        print "==> %s <==" % newRoot
        killRecursive(jobType, newRoot, verbose)


def printStatusRecursive(jobType, jobRoot, verbose):
    jc = jobCtrl(jobType, jobRoot, verbose)
    jc.printStatus()
    for newRoot in jc.getNonJobDirs():
        print "---------------------------------------"
        print "==> %s <==" % newRoot
        printStatusRecursive(jobType, newRoot, verbose)


def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "r:dn:xgv", ["submit=", "killAll", "status"])
    except getopt.GetoptError, err:
        print str(err) # will print something like "option -a not recognized"
        usage()
    jobRoot   = None
    jobName   = None
    jobType   = lsfJob
    submit    = False
    killAll   = False
    recursive = True
    verbose   = False
    status    = None
    for o, a in opts:
        if o == "-r":
            jobRoot = a
        elif o == "-n":
            jobName = a
        elif o == "-x":
            jobType = remoteLsfJob
        elif o == "-g":
            jobType = geJob
        elif o == "-d":
            recursive = False
        elif o == "-v":
            verbose = True
        elif o == "--submit":
            submit = a
        elif o == "--killAll":
            killAll = True
        elif o == "--status":
            status = True
        else:
            assert False, "unhandled option"
    if (submit and killAll) or (submit and status) or (killAll and status):
        print "multiple commands specified"
        usage()
    if (not submit) and (not killAll) and (not status):
        usage()
    if not submit and jobName:
        print "redundant jobName specified"
        usage()

    if submit:
        jc = jobCtrl(jobType, jobRoot, verbose)
        jc.submitJob(command=submit, jobName=jobName)
    elif killAll:
        if recursive:
            killRecursive(jobType, jobRoot, verbose)
        else:
            jc = jobCtrl(jobType, jobRoot, verbose)
            jc.killAllJobs()
    elif status:
        if recursive:
            printStatusRecursive(jobType, jobRoot, verbose)
        else:
            jc = jobCtrl(jobType, jobRoot, verbose)
            jc.printStatus()
    else:
        assert False, "unknown error"
    

if __name__ == "__main__":
    main()
