import os, platform, time

class StopWatch:

    _startTime = None

    def __init__(self):
        if platform.system() == 'Windows':
            self._getTime = self._getTimeWindows
        else:
            self._getTime = self._getTimeLinux

    def start(self):
        """ starts the stop watch """
        self._startTime = self._getTime()

    def stop(self):
        """ stops the stop watch and returns elapsed user time, system time, and sum;
            all times are accumulated process and child process times """
        if self._startTime is None:
            result (0, 0, 0)
        else:
            stopTime = self._getTime()
            result = (stopTime[0]-self._startTime[0], stopTime[1]-self._startTime[1], stopTime[2]-self._startTime[2])
            self._startTime = None
        return result

    def _getTimeWindows(self):
        """ returns tuple of user time, system time, and their sum;
            process and child process times are accumulated """
        t = time.clock()
        return (0, 0, t)

    def _getTimeLinux(self):
        """ returns tuple of user time, system time, and their sum;
            process and child process times are accumulated """
        t = os.times()
        return (t[0]+t[2], t[1]+t[3], t[0]+t[1]+t[2]+t[3])
