import MathUtil
from numpy import array

RB_FORMAT = \
'''\
name     = "%s";
p        = [%f, %f, %f];
q        = [%f, %f, %f, %f];
pd       = [%f, %f, %f];
qd       = [%f, %f, %f, %f];
rotParam = "QUAT_WFIRST";
mass     = %f;
size     = [%f, %f, %f];
grav     = true;
'''

MF_FORMAT = \
'''\
name         = "%s";
origin       = "%s";
insertion    = "%s";
KSE          = %f;
KPE          = %f;
b            = %f;
xrest        = %f;
T            = %f;
A            = %f;
originPos    = [%f, %f, %f];
insertionPos = [%f, %f, %f];
mType        = "%s";
'''

def WriteSimcoreConfFile(fileName, body, fibers, h, mu=1.0):
    f = open(fileName, 'w')
    f.write('# Pymuscle: rigid body and muscle fiber simulation\n')
    f.write('# Application-wide configuration file\n')
    f.write('version          = \"0.1\";\n')
    f.write('h                = %f;     # Simulation timestep\n' % h)    
    f.write('mu               = %f;     # friction coefficient\n' % mu)    
    f.write('simFrame         = 50000;  # Simulation length\n')
    f.write('plotSamplingRate = 1;\n')
    f.write('output           = \"simresult.txt\";\n')
    
    f.write('body = (\n')
    for b in body:
        assert b.rotParam in ['QUAT_WFIRST', 'EXP']
        #
        # Simcore configuration file only supports QUAT_WFIRST as a rotation parameterization.
        #
        if b.rotParam == 'EXP':
            chi_0  = list(b.chi_0[0:3]) + list(MathUtil.VtoQuat(b.chi_0[3:6]))
            chi_1  = list(b.q[0:3]) + list(MathUtil.VtoQuat(b.q[3:6]))
            chid_1 = (array(chi_1) - array(chi_0)) / h
        elif b.rotParam == 'QUAT_WFIRST':
            chi_1  = list(b.q)
            chid_1 = list(b.qd)
        else:
            assert False
        f.write('{\n')
        f.write(RB_FORMAT % tuple([b.name] + list(chi_1) + list(chid_1) + [b.mass] + list(b.boxsize)) )
        f.write('}%s\n' % (',' if b != body[-1] else ''))
    f.write(');\n')

    f.write('muscle = (\n')
    for fib in fibers:
        f.write('{\n')
        f.write(MF_FORMAT % tuple([fib.name, fib.orgBody, fib.insBody, fib.KSE, fib.KPE, fib.b, fib.xrest, fib.T, fib.A] + list(fib.orgPos) + list(fib.insPos) + [fib.mType]))
        f.write('}%s\n' % (',' if fib != fibers[-1] else ''))
    f.write(');\n')

    f.close()
