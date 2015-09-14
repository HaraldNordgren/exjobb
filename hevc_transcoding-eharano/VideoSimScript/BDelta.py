import math


class polynomial2:
    def __init__(self, x, y):
        self.x1 = x[0]
        self.x2 = x[1]
        self.x3 = x[2]
        self.y1 = y[0]
        self.y2 = y[1]
        self.y3 = y[2]

    def det3(self,a,b,c,d,e,f,g,h,i):
        return (a*e*i + b*f*g + c*d*h - c*e*g - b*d*i - a*f*h) 

    def calcPoly(self):
        x1 = self.x1
        x2 = self.x2
        x3 = self.x3
        y1 = self.y1
        y2 = self.y2
        y3 = self.y3
        
        Det = self.det3(x1*x1, x1, 1, x2*x2, x2, 1, x3*x3, x3, 1)
        #print Det
        DetA = self.det3(y1, x1, 1, y2, x2, 1, y3, x3, 1)
        #print DetA
        DetB = self.det3(x1*x1, y1, 1, x2*x2, y2, 1, x3*x3, y3, 1)
        #print DetB
        DetC = self.det3(x1*x1, x1, y1, x2*x2, x2, y2, x3*x3, x3, y3)
        #print DetC

        A = DetA / Det
        B = DetB / Det
        C = DetC / Det

        self.A = A
        self.B = B
        self.C = C

        #print "A=%f, B=%f, C=%f" % (A,B,C)


    def integrate(self, a, b):
        area = (self.A*b*b*b/3.0 + self.B*b*b/2.0 +self.C*b)  - (self.A*a*a*a/3.0 + self.B*a*a/2.0 +self.C*a)
        #print "integral = %f\n\n" % (area/(b-a))
        return (area/(b-a))

class polynomial3:
    def __init__(self, x, y):
        self.x1 = x[0]
        self.x2 = x[1]
        self.x3 = x[2]
        self.x4 = x[3]
        self.y1 = y[0]
        self.y2 = y[1]
        self.y3 = y[2]
        self.y4 = y[3]

    def det3(self,a,b,c,d,e,f,g,h,i):
        return (a*e*i + b*f*g + c*d*h - c*e*g - b*d*i - a*f*h)

    def det4(self, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p):
        return (a*self.det3(f,g,h,j,k,l,n,o,p) - b*self.det3(e,g,h,i,k,l,m,o,p) + c*self.det3(e,f,h,i,j,l,m,n,p) - d*self.det3(e,f,g,i,j,k,m,n,o) )

    def calcPoly(self):
        x1 = self.x1
        x2 = self.x2
        x3 = self.x3
        x4 = self.x4
        y1 = self.y1
        y2 = self.y2
        y3 = self.y3
        y4 = self.y4
#        print "entering calcPoly"
#        print x1
#        print x2
#        print x3
#        print x4
#        print y1
#        print y2
#        print y3
#        print y4
        
        Det = self.det4(x1*x1*x1, x1*x1, x1, 1,  x2*x2*x2, x2*x2, x2, 1,  x3*x3*x3, x3*x3, x3, 1,  x4*x4*x4, x4*x4, x4, 1)
        #print Det
        DetA = self.det4(y1, x1*x1, x1, 1,  y2, x2*x2, x2, 1,  y3, x3*x3, x3, 1,  y4, x4*x4, x4, 1)
        #print DetA
        DetB = self.det4(x1*x1*x1, y1, x1, 1,  x2*x2*x2, y2, x2, 1,  x3*x3*x3, y3, x3, 1,  x4*x4*x4, y4, x4, 1)
        #print DetB
        DetC = self.det4(x1*x1*x1, x1*x1, y1, 1,  x2*x2*x2, x2*x2, y2, 1,  x3*x3*x3, x3*x3, y3, 1,  x4*x4*x4, x4*x4, y4, 1)
        #print DetC
        DetD = self.det4(x1*x1*x1, x1*x1, x1, y1,  x2*x2*x2, x2*x2, x2, y2,  x3*x3*x3, x3*x3, x3, y3,  x4*x4*x4, x4*x4, x4, y4)
        #print DetD

        
        A = DetA / Det
        B = DetB / Det
        C = DetC / Det
        D = DetD / Det

        self.A = A
        self.B = B
        self.C = C
        self.D = D

#        print "A=%f, B=%f, C=%f, D=%f" % (A,B,C,D)


    def integrate(self, a, b):
        area = (self.A*b*b*b*b/4.0 + self.B*b*b*b/3.0 + self.C*b*b/2.0 +self.D*b)  - (self.A*a*a*a*a/4.0 + self.B*a*a*a/3.0 + self.C*a*a/2.0 +self.D*a)
#        print "integrating [%f,%f] -> integral = %f, normalized = %f" % (a,b,area, area/(b-a))
        return (area/(b-a))


class Diff:
    def __init__(self, x, y, x2, y2):
        self.y = y
        self.y2 = y2
        self.x = x
        self.x2 = x2
#        print x
#        print y
#        print x2
#        print y2
        # assumes that the bitrates are log10
        #for i in range(len(x)):
        #    self.x[i] = math.log10(self.x[i])
        #    self.x2[i] = math.log10(self.x2[i])

    def bitrateDiffHigh(self):
        if(self.y[0]<self.y[3]):
          low = max(self.y[2], self.y2[2])
          high = min(self.y[3], self.y2[3])
        else:
          low = max(self.y[1], self.y2[1])
          high = min(self.y[0], self.y2[0])

        #print "low/high %f %f" % (low,high)
        if not(high>low):
            print ("the high point is not greater than the low point, returning")
            return (0.0,0)

	if(self.y[0]<self.y[3]):
          range = max(abs(self.y[3]-self.y[2]), abs(self.y2[3]-self.y2[2]))
	else:
          range = max(abs(self.y[1]-self.y[0]), abs(self.y2[1]-self.y2[0]))

        overlap = abs(high - low)/range
        #print "****** high overlap=%f" % overlap
        if(self.y[0]<self.y[3]):
          p = polynomial2(self.y[len(self.y)-3:len(self.y)],self.x[len(self.x)-3:len(self.x)]) # [len(a)-3:len(a)] just takes the last 3
        else:
          p = polynomial2(self.y[0:3],self.x[0:3]) 

        p.calcPoly()
        int1 = p.integrate(low, high)

        if(self.y[0]<self.y[3]):
          p2 = polynomial2(self.y2[len(self.y2)-3:len(self.y2)],self.x2[len(self.x2)-3:len(self.x2)])
        else:
          p2 = polynomial2(self.y2[0:3],self.x2[0:3])

        p2.calcPoly()
        int2 = p2.integrate(low, high)

        drate = ((math.pow(10,int2) / math.pow(10,int1)) - 1) * 100
        #print "drate high = %f" % drate 
        return (drate,overlap)

    def bitrateDiffLow(self):
        if(self.y[0]<self.y[3]):
          low = max(self.y[0], self.y2[0])
          high = min(self.y[1], self.y2[1])
        else:
          low = max(self.y[3], self.y2[3])
          high = min(self.y[2], self.y2[2])

        #print "low/high %f %f" % (low,high)
        if not(high>low):
            print ("the high point is not greater than the low point, returning")
            return (0.0,0)
        if(self.y[0]<self.y[3]):
          range = max(abs(self.y[1]-self.y[0]), abs(self.y2[1]-self.y2[0]))
        else:
          range = max(abs(self.y[3]-self.y[2]), abs(self.y2[3]-self.y2[2]))

        overlap = abs(high - low)/range
        #print "****** low overlap=%f" % overlap
        if(self.y[0]<self.y[3]):
          p = polynomial2(self.y[0:3],self.x[0:3]) #  just takes the first 3
        else:
          p = polynomial2(self.y[1:4],self.x[1:4]) 
        p.calcPoly()
        int1 = p.integrate(low, high)

        if(self.y[0]<self.y[3]):
          p2 = polynomial2(self.y2[0:3],self.x2[0:3])
        else:
          p2 = polynomial2(self.y2[1:4],self.x2[1:4])

        p2.calcPoly()
        int2 = p2.integrate(low, high)

        drate = ((math.pow(10,int2) / math.pow(10,int1)) - 1) * 100
        #print "drate low = %f" % drate 
        return (drate,overlap)

    def bitrateDiff(self):
        if(self.y[0]<self.y[3]):
          low = max(self.y[0], self.y2[0])
          high = min(self.y[3], self.y2[3])
        else:
          high = min(self.y[0], self.y2[0])
          low = max(self.y[3], self.y2[3])

        #print "low/high %f %f" % (low,high)
        if not(high>low):
            print ("the high point is not greater than the low point, returning")
            return (0.0,0)
        range = max(abs(self.y[3]-self.y[0]), abs(self.y2[3]-self.y2[0]))

#        print "high = %f, low = %f, range = %f" % (high,low,range)
        overlap = abs(high - low)/range
        #print "****** avg overlap=%f" % overlap
        p = polynomial3(self.y[0:4],self.x[0:4])

        p.calcPoly()
        int1 = p.integrate(low, high)

        p2 = polynomial3(self.y2[0:4],self.x2[0:4])
        p2.calcPoly()
        int2 = p2.integrate(low, high)

        drate = ((math.pow(10,int2) / math.pow(10,int1)) - 1) * 100
        #print "drate = %f perc." % drate 
        return (drate,overlap)

    def psnrDiffHigh(self):
        if(self.x[0]<self.x[3]):
          #low = max(self.x[-2], self.x2[-2])
          #high = min(self.x[-1], self.x2[-1])
          low = max(self.x[2], self.x2[2])
          high = min(self.x[3], self.x2[3])
        else:
          low = max(self.x[1], self.x2[1])
          high = min(self.x[0], self.x2[0])

        #print "low/high %f %f" % (low,high)
        if not(high>low):
            print ("the high point is not greater than the low point, returning")
            return (0.0,0)
        if(self.x[0]<self.x[3]):
          range = max(abs(self.x[3]-self.x[2]), abs(self.x2[3]-self.x2[2]))
        else:
          range = max(abs(self.x[1]-self.x[0]), abs(self.x2[1]-self.x2[0]))

        overlap = abs(high - low)/range
        #print "****** high overlap=%f" % overlap
        if(self.x[0]<self.x[3]):
          p = polynomial2(self.x[1:4],self.y[1:4]) # [len(a)-3:len(a)] just takes the last 3
        else:
          p = polynomial2(self.x[0:3],self.y[0:3]) 

        p.calcPoly()
        int1 = p.integrate(low, high)

        if(self.y[0]<self.y[3]):
          p2 = polynomial2(self.x2[1:4],self.y2[1:4])
        else:
          p2 = polynomial2(self.x2[0:3],self.y2[0:3])

        p2.calcPoly()
        int2 = p2.integrate(low, high)

        dpsnr = int2-int1
        #print "dpsnr high = %f" % dpsnr 
        return (dpsnr,overlap)

    def psnrDiffLow(self):
        if(self.x[0]<self.x[3]):
          low = max(self.x[0], self.x2[0])
          high = min(self.x[1], self.x2[1])
        else:
          low = max(self.x[3], self.x2[3])
          high = min(self.x[2], self.x2[2])

        #print "low/high %f %f" % (low,high)
        if not(high>low):
            print ("the high point is not greater than the low point, returning")
            return (0.0,0)
        if(self.x[0]<self.x[3]):
          range = max(abs(self.x[1]-self.x[0]),abs(self.x2[1]-self.x2[0]))
        else:
          range = max(abs(self.x[3]-self.x[2]), abs(self.x2[3]-self.x2[2]))

        overlap = abs(high - low)/range
        #print "****** low overlap=%f" % overlap
        if(self.x[0]<self.x[3]):
          p = polynomial2(self.x[0:4],self.y[0:4]) #  just takes the first 3
        else:
          p = polynomial2(self.x[1:4],self.y[1:4]) 
        p.calcPoly()
        int1 = p.integrate(low, high)

        if(self.x[0]<self.x[3]):
          p2 = polynomial2(self.x2[0:4],self.y2[0:4])
        else:
          p2 = polynomial2(self.x2[1:4],self.y2[1:4])

        p2.calcPoly()
        int2 = p2.integrate(low, high)

        dpsnr = int2-int1
        #print "dpsnr low = %f" % dpsnr 
        return (dpsnr,overlap)

    def psnrDiff(self):
        if(self.x[0]<self.x[3]):
          low = max(self.x[0], self.x2[0])
          high = min(self.x[3], self.x2[3])
        else:
          high = min(self.x[0], self.x2[0])
          low = max(self.x[3], self.x2[3])

        if not(high>low):
            print ("the high point is not greater than the low point, returning")
            return (0.0,0)
        range = max(abs(self.x[3]-self.x[0]), abs(self.x2[3]-self.x2[0]))
        overlap = abs(high - low)/range
        #print "****** avg overlap=%f" % overlap
        p = polynomial3(self.x[0:4],self.y[0:4])
        p.calcPoly()
        int1 = p.integrate(low, high)

        p2 = polynomial3(self.x2[0:4],self.y2[0:4])
        p2.calcPoly()
        int2 = p2.integrate(low, high)

        dpsnr = int2-int1
        #print "dpsnr = %f perc." % dpsnr 
        return (dpsnr,overlap)

    def check(self):
     #"Just at test to verify with VCEG results in VCEG-AI11"
     y = [30.860,33.583, 36.659, 40.125]
     x = [593.942, 1445.958, 4299.331, 14376.960]
     y2 = [30.988, 33.526, 36.624, 39.731]
     x2 = [380.470, 1178.000, 4479.860, 14677.280]

     for i in range(4):
      x[i] = math.log10(x[i])
      x2[i] = math.log10(x2[i])

     d =  Diff(x, y, x2, y2)
     (rate,overlap)=d.bitrateDiff()
     print "drate/overlap = %f %f" % (rate,overlap)
     (rate,overlap)=d.bitrateDiffLow()
     print "low drate/overlap = %f %f" % (rate,overlap)
     (rate,overlap)=d.bitrateDiffHigh()
     print "high drate/overlap = %f %f" % (rate,overlap)

     xtmp=[]
     ytmp=[]
     x2tmp=[]
     y2tmp=[]
     
     for i in range(4):
      print i
      xtmp.append(x[3-i])
      ytmp.append(y[3-i])
      x2tmp.append(x2[3-i])
      y2tmp.append(y2[3-i])

     x=xtmp;
     y=ytmp;
     x2=x2tmp;
     y2=y2tmp;
     print "psnr=[%f %f %f %f]\n" % (y[0],y[1],y[2],y[3])

     d =  Diff(x, y, x2, y2)
     (rate,overlap)=d.bitrateDiff()
     print "drate/overlap = %f %f" % (rate,overlap)
     (rate,overlap)=d.bitrateDiffLow()
     print "low drate/overlap = %f %f" % (rate,overlap)
     (rate,overlap)=d.bitrateDiffHigh()
     print "high drate/overlap = %f %f" % (rate,overlap)


####################################################################################################

class ObsData:
    def __init__(self, dat=None):
        if dat is not None:
            self.getData(dat)
    
    def getData(self, dat):
        # assumes that rates are log10
        self.n = len(dat)-1
        self.x = []
        self.y = []
        for i in range(self.n+1):
            self.x.append(dat[i][0])
            self.y.append(dat[i][1])
        self.xrange = [min(self.x), max(self.x)]
        self.yrange = [min(self.y), max(self.y)]
#        print self.x
#        print self.y
    
    def buildRegularlySpacedData(self, xmin, xmax, n):
        self.n = n
        self.x = []
        self.y = []
        for i in range(self.n+1):
            self.x.append(xmin + i * float(xmax - xmin) / float(n))
            self.y.append(0)

    def swap(self):
        t = self.x
        self.x = self.y
        self.y = t
        t = self.xrange
        self.xrange = self.yrange
        self.yrange = t

        
class Polynom:
    def __init__(self, deg):
        self.reset(deg)

    def reset(self, deg):        
        self.deg = deg
        self.offset = 0
        self.c = []
        for i in range(self.deg+1):
            self.c.append(0)
        self.v = []
        self.nbObs = None

    def setXn(self, n):
        self.reset(self.deg)
        self.c[n] = 1

    def eval(self, x):
        t = 1
        y = 0
        for i in range(self.deg+1):
            y = y + t * self.c[i]
            t = t * (x - self.offset)
        return y

    def evalOnObs(self, obsData):
        self.nbObs = obsData.n
        self.v = []
        for i in range(self.nbObs+1):
            self.v.append(self.eval(obsData.x[i]))

    def estimateVarianceError(self, obsData):
        assert self.nbObs == obsData.n
        s = 0
        for i in range(self.nbObs+1):
            s = s + (self.v[i] - obsData.y[i]) * (self.v[i] - obsData.y[i])
        if self.nbObs > self.deg:
            s = s / (self.nbObs - self.deg)
        else:
            s = 0
        return s

    def integrate(self, a, b):
        ta = a - self.offset
        tb = b - self.offset
        s = 0
        for i in range(self.deg+1):
            s = s + self.c[i] * float(tb - ta) / float(i + 1)
            ta = ta * (a - self.offset)
            tb = tb * (b - self.offset)
#        print self.c
#        print "integrating [%f,%f] -> %s" % (a,b,s)
        return s

    def crossProduct(self, p2):
        assert self.nbObs == p2.nbObs
        y = 0
        for i in range(self.nbObs+1):
            y = y + self.v[i] * p2.v[i]
        return y

    def mAdd(self, a, p2):
        assert self.deg == p2.deg
        assert self.nbObs == p2.nbObs
        for i in range(self.deg+1):
            self.c[i] = self.c[i] + a * p2.c[i]
        for i in range(self.nbObs+1):
            self.v[i] = self.v[i] + a * p2.v[i]

    def div(self, a):
        if float(a) == 0: raise BDError("polynom approximation unstable")
        for i in range(self.deg+1):
            self.c[i] = float(self.c[i]) / float(a)
        for i in range(self.nbObs+1):
            self.v[i] = float(self.v[i]) / float(a)

    def coeffString(self):
        s = ""
        for i in range(self.deg+1):
            s = "%s %f" % (s, self.c[i])
        return s

    def valString(self):
        s = ""
        for i in range(self.nbObs+1):
            s = "%s %f" % (s, self.v[i])
        return s


class Interpolation:
    def __init__(self, data, deg):
        self.linearRegression(data, deg)

    def linearRegression(self, data, deg):
#        print "entering regression, deg = %d" % deg
#        print data.x
#        print data.y
#        print data.n
        self._computePolynomBase(data, deg)
        self._computePolynomInterp(data, deg)
        self.estimVar = self.p.estimateVarianceError(data)
  
    def computeAreaBelow(self, xmin, xmax):
        self.area = self.p.integrate(xmin, xmax)
        self.confArea = self._rawConfInterval(xmin, xmax)

    def _computePolynomBase(self, data, deg):
        #===== compute offset for polynomial
        offset = float(data.xrange[0] + data.xrange[1]) / 2.0
        #===== use basic polynomials
        p = []
        for i in range(deg+1):
            p.append(Polynom(deg))
            p[i].setXn(i)
            p[i].offset = offset
        #===== compute value of polynoms
        for i in range(deg+1):
            p[i].evalOnObs(data)
        #==== compute scalar products
        crossProduct = {}
        for i in range(deg+1):
            for j in range(deg+1):
                crossProduct[i,j] = float(p[i].crossProduct(p[j]))
#                print "CP(%d,%d) = %f" % (i,j,crossProduct[i,j])
        #==== Graham Schmidt orthogonalisation
        for i in range(1,deg+1):
            for j in range(i-1+1):
                a = crossProduct[i,j] / crossProduct[j,j]
                #-- update basis polynomial
                p[i].mAdd(-a, p[j])
                #-- update cross products
                for k in range(deg+1):
                    if i!=k:
                        crossProduct[i,k] = crossProduct[i,k] - a * crossProduct[j, k]
                    else:
                        crossProduct[i,i] = crossProduct[i, i] - a * a * crossProduct[j, j]
#                    print "-- CP(%d,%d) = %f" % (i,k,crossProduct[i, k])
                for k in range(deg+1):
                    crossProduct[k,i] = crossProduct[i,k]
        #============ Polygon normalisation
        for i in range(deg+1):
            if crossProduct[i,i] < 0: raise BDError("polynom approximation unstable")
            a = math.sqrt(crossProduct[i,i])
            p[i].div(a)
        self.pBase = p
        
    def _computePolynomInterp(self, data, deg):
        p = Polynom(deg)
        p.evalOnObs(data)
        p.offset = self.pBase[0].offset
        for i in range(deg+1):
            # compute scalar produt of y data with pBase(i)
            sp = 0
            for j in range(data.n+1):
                sp = sp + data.y[j] * self.pBase[i].v[j]
            # update interpolation polynom
            p.mAdd(sp, self.pBase[i])
        self.p = p

    def _rawConfInterval(self, xmin, xmax):
        p = self.pBase
        np = self.p.deg
        data = ObsData()
        data.buildRegularlySpacedData(xmin, xmax, 32)
        for i in range(np+1):
            p[i].evalOnObs(data)
        ss = 0
        for i in range(data.n+1):
            s = 0
            for j in range(np+1):
                s = s + p[j].v[i] * p[j].v[i]
            s = math.sqrt(s)
            if i == 0 or i == data.n:
                ss = ss + s / 2.0
            else:
                ss = ss + s
        return float(ss) / float(data.n)

class BDError(Exception):
    def __init__(self, value):
        print "############# BDError: %s" % value
        
class NDiff:
    def __init__(self, x1, y1, x2, y2, maxDeg=3):
        # assumes that the bitrates are log10
        self.n1 = len(x1)
        self.n2 = len(x2)
        assert len(y1) == self.n1
        assert len(y2) == self.n2
        self.dat1 = []
        for i in range(self.n1):
            self.dat1.append([x1[i],y1[i]])
        self.dat2 = []
        for i in range(self.n2):
            self.dat2.append([x2[i],y2[i]])
        self.dat1.sort()
        self.dat2.sort()
#        print self.dat1
#        print self.dat2
        self.maxDeg = maxDeg

    def _NBJM_calc(self, refData, propData, xmin, xmax, deg, swap, t):
        #-- Ref: get polynomial regression and compute area below curve
        irRef = Interpolation(refData, deg)
        irRef.computeAreaBelow(xmin, xmax)
        #-- Prop: get polynomial regression and compute area below curve
        irProp = Interpolation(propData, deg)
        irProp.computeAreaBelow(xmin, xmax)
        #-- compute difference of areas
        res = irProp.area - irRef.area
        res = res - t * (math.sqrt(irProp.estimVar) * irProp.confArea + math.sqrt(irRef.estimVar) * irRef.confArea)
        
        res = res / float(xmax - xmin)
#        print "xmin = %f, xmax = %f => res = %f" % (xmin, xmax, res)
        #-- for BD rate, change to %
        if swap:
            res = ((10**res) - 1) * 100.0
        return res

    def _NBJM(self, rangeRef, rangeProp, boundsRangeRef, boundsRangeProp, deg, swap, t):
        # data
        dat1 = []
        for i in rangeRef:
            dat1.append(self.dat1[i])
        dat2 = []
        for i in rangeProp:
            dat2.append(self.dat2[i])
        refData = ObsData(dat1)
        propData = ObsData(dat2)
        # boundaries
        bdat1 = []
        for i in boundsRangeRef:
            bdat1.append(self.dat1[i])
        bdat2 = []
        for i in boundsRangeProp:
            bdat2.append(self.dat2[i])
        brefData = ObsData(bdat1)
        bpropData = ObsData(bdat2)
        #-- swap x/y if computing BD rate
        if swap:
            refData.swap()
            propData.swap()
            brefData.swap()
            bpropData.swap()
        #-- estimate integration interval and integrate
        xmin = max([brefData.xrange[0], bpropData.xrange[0]])
        xmax = min([brefData.xrange[1], bpropData.xrange[1]])
        if xmax > xmin:
            delta = self._NBJM_calc(refData, propData, xmin, xmax, deg, swap, t)
            # determine overlap
            range = max(brefData.xrange[1]-brefData.xrange[0], bpropData.xrange[1]-bpropData.xrange[0])
            overlap = float(xmax - xmin) / float(range)
        else:
            delta = 0.0
            overlap = 0.0
        return (delta, overlap)

    def _RBJM(self, range1, range2, deg=3, t=0):
        if self.n1 < deg+1 or self.n2 < deg+1: return (-1, -1)
        return self._NBJM(range1, range2, range1, range2, deg, True, -t)

    def _PBJM(self, range1, range2, deg=3, t=0):
        if self.n1 < deg+1 or self.n2 < deg+1: return (-1, -1)
        return self._NBJM(range1, range2, range1, range2, deg, False, t)

    def _BRBJM(self, range1, range2, boundsRange1, boundsRange2, deg=3, t=0):
        if self.n1 < deg+1 or self.n2 < deg+1: return (-1, -1)
        return self._NBJM(range1, range2, boundsRange1, boundsRange2, deg, True, -t)

    def _BPBJM(self, range1, range2, boundsRange1, boundsRange2, deg=3, t=0):
        if self.n1 < deg+1 or self.n2 < deg+1: return (-1, -1)
        return self._NBJM(range1, range2, boundsRange1, boundsRange2, deg, False, t)
        
    def bitrateDiffHigh(self):
        # 3 highest RD points for fitting, 2 highest RD points for integration, degree 2
        try:
            result = self._BRBJM(range(self.n1-3, self.n1-1+1), range(self.n2-3, self.n2-1+1), range(self.n1-2, self.n1-1+1), range(self.n2-2, self.n2-1+1), 2)
        except BDError:
            result = (-9999, -9999)
        return result

    def bitrateDiffLow(self):
        # 3 lowest RD points for fitting, 2 lowest RD points for integration, degree 2
        try:
            result = self._BRBJM(range(0, 2+1), range(0, 2+1), range(0, 1+1), range(0, 1+1), 2)
        except BDError:
            result = (-9999, -9999)
        return result

    def bitrateDiff(self):
        # all RD points for fitting and integration, degree depending on number of RD points
        assert self.n1 == self.n2
        deg = min(self.n1 - 1, self.maxDeg)
        try:
            result = self._RBJM(range(0, self.n1-1+1), range(0, self.n2-1+1), deg)
        except BDError:
            result = (-9999, -9999)
        return result

    def psnrDiffHigh(self):
        # 3 highest RD points for fitting, 2 highest RD points for integration, degree 2
        return self._BPBJM(range(self.n1-3, self.n1-1+1), range(self.n2-3, self.n2-1+1), range(self.n1-2, self.n1-1+1), range(self.n2-2, self.n2-1+1), 2)

    def psnrDiffLow(self):
        # 3 lowest RD points for fitting, 2 lowest RD points for integration, degree 2
        return self._BPBJM(range(0, 2+1), range(0, 2+1), range(0, 1+1), range(0, 1+1), 2)

    def psnrDiff(self):
        # all RD points for fitting and integration, degree depending on number of RD points
        assert self.n1 == self.n2
        deg = min(self.n1 - 1, self.maxDeg)
        return self._PBJM(range(0, self.n1-1+1), range(0, self.n2-1+1), deg)


def check(fct):
    #"Just at test to verify with VCEG results in VCEG-AI11"
    y = [30.860,33.583, 36.659, 40.125]
    x = [593.942, 1445.958, 4299.331, 14376.960]
    y2 = [30.988, 33.526, 36.624, 39.731]
    x2 = [380.470, 1178.000, 4479.860, 14677.280]

    for i in range(4):
        x[i] = math.log10(x[i])
        x2[i] = math.log10(x2[i])

    d = fct(x, y, x2, y2)
    (rate,overlap)=d.bitrateDiff()
    print "drate/overlap = %f %f" % (rate,overlap)
    (rate,overlap)=d.bitrateDiffLow()
    print "low drate/overlap = %f %f" % (rate,overlap)
    (rate,overlap)=d.bitrateDiffHigh()
    print "high drate/overlap = %f %f" % (rate,overlap)
    (rate,overlap)=d.psnrDiff()
    print "dpsnr/overlap = %f %f" % (rate,overlap)
    (rate,overlap)=d.psnrDiffLow()
    print "low dpsnr/overlap = %f %f" % (rate,overlap)
    (rate,overlap)=d.psnrDiffHigh()
    print "high dpsnr/overlap = %f %f" % (rate,overlap)

    xtmp=[]
    ytmp=[]
    x2tmp=[]
    y2tmp=[]

    for i in range(4):
        print i
        xtmp.append(x[3-i])
        ytmp.append(y[3-i])
        x2tmp.append(x2[3-i])
        y2tmp.append(y2[3-i])

    x=xtmp;
    y=ytmp;
    x2=x2tmp;
    y2=y2tmp;
    print "psnr=[%f %f %f %f]\n" % (y[0],y[1],y[2],y[3])

    d = fct(x, y, x2, y2)
    (rate,overlap)=d.bitrateDiff()
    print "drate/overlap = %f %f" % (rate,overlap)
    (rate,overlap)=d.bitrateDiffLow()
    print "low drate/overlap = %f %f" % (rate,overlap)
    (rate,overlap)=d.bitrateDiffHigh()
    print "high drate/overlap = %f %f" % (rate,overlap)
    (rate,overlap)=d.psnrDiff()
    print "dpsnr/overlap = %f %f" % (rate,overlap)
    (rate,overlap)=d.psnrDiffLow()
    print "low dpsnr/overlap = %f %f" % (rate,overlap)
    (rate,overlap)=d.psnrDiffHigh()
    print "high dpsnr/overlap = %f %f" % (rate,overlap)

def check2(): # M16894
    x1 = [639.68,546.44,469.52,403.81,351.4,302.85,263.2,233.9,204.11,180.66,158.25,140.55,123.21,110.02,98.02,86.58]
    y1 = [40.35,39.73,39.16,38.53,37.96,37.37,36.77,36.21,35.58,35.04,34.45,33.9,33.25,32.78,32.16,31.6]
    x2 = [771.51,653.67,559.92,475.28,408.9,360.13,313.48,271.1,237,209.96,187.85,163.7,144.8,128.17,113.15,101.49]
    y2 = [40.37,39.75,39.14,38.48,37.89,37.35,36.77,36.13,35.54,35,34.45,33.86,33.26,32.69,32.1,31.49]
    conf = [2.13,2.14,2.16,2.18,2.20,2.23,2.26,2.31,2.36,2.45,2.57,2.78,3.18,4.30,12.71]

    for i in range(len(x1)):
        x1[i] = math.log10(float(x1[i]))
    for i in range(len(x2)):
        x2[i] = math.log10(float(x2[i]))

    d = NDiff(x2,y2,x1,y1)
    for i in range(15):
        r1 = d._RBJM(range(len(x1)),range(len(x2)),i,conf[i])[0]
        r2 = d._RBJM(range(len(x1)),range(len(x2)),i,0)[0]
        r3 = d._RBJM(range(len(x1)),range(len(x2)),i,-conf[i])[0]
        print "%.2f %.2f %.2f %.2f" % (r1,r2,r3,r1-r3)


#check(Diff)
#print "=================="
#check(NDiff)
#print "=================="
#check2()

