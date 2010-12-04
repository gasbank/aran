# MosekRbLcp3D.py
# 2010 Geoyeob Kim
# As a part of the thesis implementation
#
# A rigid body with flat ground
#
# Rigid body state is parameterized by ExpMap (6-DOF)
#
import sys
from numpy import array, zeros, ones, linalg, hstack, dot,set_printoptions,nan
import mosek
from math import sqrt, pi, ceil, sin
from scipy import sparse
from ExpBodyMoEq_real import MassMatrixAndCqdVector
from dRdv_real import GeneralizedForce
import ExpBody
import ZV
import copy
import RigidBody
import MuscleFiber
import GlobalContext
inf   = 0.0 # just for using the symbol 'inf' on MOSEK
MU    = GlobalContext.GetFrictionCoefficient()
GRAV  = GlobalContext.GetGravitationalAcceleration()
h     = GlobalContext.GetSimTimeStep()


# Define a stream printer to grab output from MOSEK
def streamprinter(text):
    sys.stdout.write(text)
    sys.stdout.flush()

def PrintOptimizedResult(p1, x_opt):
    ci = p1.Aci # Sub-column indices
    chi_1 = x_opt[ ci[0]:ci[1] ]
    f_c   = x_opt[ ci[1]:ci[2] ]
    c_c   = x_opt[ ci[2]:ci[3] ]
    print 'Optimal solution'
    print '-'*80
    print '       BODY 1'
    print '     chi^(i+1) =', chi_1[0:3], chi_1[3:6]
    print '           f_c ='
    for j in xrange(len(p1.p)):
        print ' '*10, f_c[6*j : 6*j+3]

def Main ():
    # Configure Numpy printing options
    set_printoptions(threshold=nan, linewidth=378, precision=8)
    
    print 'Initialize a rigid body with initial conditions...'
    #
    #                  ________
    #                 |       |
    #                 |  p2   |    ----- 4.5
    #                 |_______|    _____ 4.0
    #                     |
    #                     | f1
    #                  ___|____    _____ 1.0
    #                 |       |
    #                 |  p1   |    ----- 0.5
    #                 |_______|
    #  --------------------------------------------------
    #  //////////////////////////////////////////////////
    #
    p1BoxSize = array([2.,2.,1.])
    p2BoxSize = array([1.,1.,1.])
    
    p1 = RigidBody.RigidBody( 'BODY 0', None, 10.0, p1BoxSize, array([0.,0,0.5,0,0,0]), array([0.,0,0.0,0,0,0]), (0.1,0.2,0.3), 'EXP' )   # Bottom box
    p2 = RigidBody.RigidBody( 'BODY 1', None,  2.0, p2BoxSize, array([0.,0,4.5,0,0,0]), array([0.,0,0.0,0,0,0]), (0.1,0.2,0.3), 'EXP' )   # Top box
    plist = [p1, p2]
    
    p1AttPos = (p1BoxSize[0]/2.0, p1BoxSize[1]/2.0, +p1BoxSize[2]/2.0)
    p2AttPos = (p2BoxSize[0]/2.0, p2BoxSize[1]/2.0, -p2BoxSize[2]/2.0)
    AttPosMultiplier = [ (1, 1, 1), (1, -1, 1), (-1, 1, 1), (-1, -1, 1) ]
    flist = []
    for i in xrange(len(AttPosMultiplier)):
        for mp in AttPosMultiplier:
            p1ap = array([a*b for a, b in zip(mp, p1AttPos)])
            p2ap = array([a*b for a, b in zip(AttPosMultiplier[i], p2AttPos)])
            print p1ap, p2ap
            
            #name, mType, bAttachedPosNormalized, T_0, A, kse, kpe, b, x_r0, x_rl, x_ru, p1Name, p2Name, p1, p2, p1AttPos, p2AttPos
            f = MuscleFiber.MuscleFiber('Fiber 0',
                                        'MUSCLE',
                                        False,
                                        0.,    # T_0
                                        None,  # A
                                        10.,    # kse
                                        10.,    # kpe
                                        10.,    # b
                                        3.0,   # x_r0
                                        1.,   # x_rl
                                        10.,   # x_ru
                                        None,
                                        None,
                                        p1, p2, p1ap, p2ap)
            flist.append(f)

    
    # Make a MOSEK environment
    env = mosek.Env ()
    #mosek.iparam.intpnt_num_threads = 4

    nFrame = int(sys.argv[1])
    printMsg = '-d' in sys.argv[1:]
    # Attach a printer to the environment
    if printMsg:
        env.set_Stream (mosek.streamtype.log, streamprinter)
    nd = 6
    
    nb = len(plist)
    nm = len(flist)
    for ff in xrange(nFrame):
        if ff < 20:
            plist[1].chi_ref = array([0.05,-0.05,4.5+0.05*ff,0.5,0.1,0.3])
            #plist[1].chi_ref = array([0.,0,4.5,0,0,0])
        if ff==76:
            pass
        
        frameReport = ''
        frameReport += '='*40 + ' FRAME ' + str(ff) + ' START ' + '='*40 + '\n'
        for i in xrange(nb):
            frameReport += '  BODY %d:\n' % i
            frameReport += '    chi_0 = ' + str(plist[i].chi_0) + '\n'
            frameReport += '    chi_1 = ' + str(plist[i].q) + '\n'
        for j in xrange(nm):
            frameReport += '  MUSCLE %d:  T_0 = %f   x_r = %f\n' % (j, flist[j].T_0, flist[j].x_r0)
        frameReport += '  Updating current state dependent values...' + '\n'
        for i in xrange(nb):
            frameReport += plist[i].updateCurrentStateDependentValues()
        frameReport += '  Done.' + '\n'
        frameReport += '  Updating fiber related values...' + '\n'
        for i in xrange(nb):
            plist[i].updateFiberRelatedValues() # Should be called after all updateCurrentStateDependentValues() calls.
        frameReport += '  Done.' + '\n'
        frameReport += '  Optimizing to compute next state...' + '\n'
        subFrameReport, x_opt, cost, Ari, Aci, solsta = Optimize(env, printMsg, plist, flist)
        if solsta == mosek.solsta.optimal:
            frameReport += 'RESULT : Optimal\n'
        elif solsta == mosek.solsta.near_optimal:
            frameReport += 'RESULT : Near optimal\n'
        elif solsta == mosek.solsta.unknown:
            frameReport += 'RESULT : Unknown\n'
        frameReport += subFrameReport
        if x_opt is None:
            frameReport += 'Mosek optimizer failed?!\n'
            '''
            print '='*30, 'MATRIX A^T', '='*30
            print p1_A.T
            '''
            print frameReport
            sys.exit(-123)
        for i in xrange(nb):
            if len(plist[i].p) > 0:
                frameReport += ' '*4 + ' - BODY %d calculated contact forces\n' % i
                for pfi, j in zip(plist[i].p, xrange(len(plist[i].p))):
                    frameReport += ' '*10 + str(pfi) + ':' + str( x_opt[ Aci[i] + plist[i].Aci[1] + nd*j : Aci[i] + plist[i].Aci[1] + nd*j + 3] ) + '\n'
        for j in xrange(nm):
            frameReport += ' '*4 + ' - MUSCLE %d T = %f    A = %f   R = %f\n' % tuple( [j] + [ x_opt[Aci[nb+k]+j] for k in xrange(3) ] )
        frameReport += '  Done. Cost = ' + str(cost) + '\n'
        for i in xrange(nb):
            frameReport += plist[i].setChi_1( x_opt[ Aci[i] + plist[i].Aci[0] : Aci[i] + plist[i].Aci[1] ] )
            frameReport += plist[i].reparameterize()
            frameReport += '  BODY %d Current state updated to:\n' % i
            frameReport += '  chi_0 = ' + str(plist[i].chi_0) + '\n'
            frameReport += '  chi_1 = ' + str(plist[i].q) + '\n'
        for j in xrange(nm):
            flist[j].setT_0( x_opt[ Aci[nb+0] + j ] )
            flist[j].x_r0 =  x_opt[ Aci[nb+2] + j ]
        
        # Print buffered frame report entirely
        print frameReport
        #print linalg.norm(plist[1].chi_ref - plist[1].q)
        #print linalg.norm(plist[0].chi_ref - plist[0].q)

def Optimize(env, printMsg, plist, flist):
    # Create a task
    task = env.Task(0,0)

    # Attach a printer to the task
    if printMsg:
        task.set_Stream (mosek.streamtype.log, streamprinter)
    Alist = [pi.getAMatrix() for pi in plist]
    nplist = [len(pi.p) for pi in plist]
    nmlist = [len(pi.fibers) for pi in plist]
    nb = len(plist)     # Number of RBs
    nd = 6              # 6-DOF
    nm = len(flist)     # Numer of muscle fibers
    # Calculate subblock matrices' row/col indices
    Asubrowsizes = [Ai.shape[0] for Ai in Alist] + [nd*nmj for nmj in nmlist] + [ nm ]
    Asubcolsizes = [Ai.shape[1] for Ai in Alist] + [ nm, nm, nm, 1, 1 ]
    Ari = [0]
    Aci = [0]
    for a in Asubrowsizes:
        Ari.append( Ari[-1] + a )
    for a in Asubcolsizes:
        Aci.append( Aci[-1] + a )
    
    # Objective function coefficients
    c   = zeros(Aci[-1])
    for i in xrange(nb):
        for j in xrange(nplist[i]):
            c[ Aci[i] + plist[i].Aci[3] + 4*j + 2 ] = 0                 # Estimated position of z-coordinate of contact point
        c[ Aci[i] + plist[i].Aci[5] : Aci[i] + plist[i].Aci[6] ] = 0    # minimize the movement of candidate contact points
        c[ Aci[i] + plist[i].Aci[8] ] = 20 # minimize the movement of COMs
    c[ Aci[nb+3]:Aci[nb+4] ] = 1e-6 # minimize aggregate tension
    c[ Aci[nb+4]:Aci[nb+5] ] = 1e-6 # minimize aggregate actuation force
    
    # Matrix and vector for equality constraints
    A = sparse.lil_matrix((Ari[-1],Aci[-1]))
    for i in xrange(nb):
        A[ Ari[i]:Ari[i+1]       , Aci[i]:Aci[i+1]                ] = Alist[i]
        if nmlist[i] > 0:
            A[ Ari[nb+i]:Ari[nb+i+1] , Aci[i+1]-nd*nmlist[i]:Aci[i+1] ] = -sparse.identity(nd*nmlist[i])
        for j in xrange(nmlist[i]):
            # Transform index: Calculate global fiber index of
            #                  j-th attached fiber on Body i
            gbl_j = flist.index( plist[i].fibers[j] )
            assert gbl_j >= 0
            A[ Ari[nb+i]+nd*j:Ari[nb+i]+nd*(j+1), Aci[nb] + gbl_j] = plist[i].R[j].reshape(6,1)
    for j in xrange(nm):
        Kj = flist[j].getK()
        A[ Ari[2*nb] + j, Aci[nb + 0] + j ] = Kj[0]
        A[ Ari[2*nb] + j, Aci[nb + 1] + j ] = Kj[1]
        A[ Ari[2*nb] + j, Aci[nb + 2] + j ] = Kj[2]
        
    Eta = sparse.lil_matrix((Ari[-1], 1))
    for i in xrange(nb):
        Eta[ Ari[i]:Ari[i+1], 0 ] = plist[i].getEta()
    for j in xrange(nm):
        Eta[ Ari[2*nb] + j, 0 ] = flist[j].gets()
    
    NUMVAR = A.shape[1]
    NUMCON = A.shape[0]
    NUMANZ = A.nnz
    # Give MOSEK an estimate of the size of the input data. 
    #  This is done to increase the speed of inputting data. 
    #  However, it is optional. 
    task.putmaxnumvar(NUMVAR)
    task.putmaxnumcon(NUMCON)
    task.putmaxnumanz(NUMANZ)

    # Configure all constraints defined by the above matrix and vector are equality constraints
    bkc = [ mosek.boundkey.fx, ] * A.shape[0]
    Etaden = array(Eta.todense())
    blc = [ Etaden[i,0] for i in xrange(Etaden.shape[0]) ]
    buc = copy.deepcopy(blc)



    # Optimization variable boundary configuration
    # ( Default: unbounded, i.e., (-inf,inf) )
    bkx = [ mosek.boundkey.fr, ] * A.shape[1]
    blx = [ -inf, ] * A.shape[1]
    bux = [  inf, ] * A.shape[1]
    
    nonnegatives = []
    fixed_ones   = []
    fixed_zeros  = []
    
    for i in xrange(nb):
        nonnegatives += [ Aci[i] + plist[i].Aci[0] +  2                                  ] # chi_2_z
        nonnegatives += [ Aci[i] + plist[i].Aci[1] + nd*j + 2 for j in xrange(nplist[i]) ] # f_c_z
        fixed_zeros  += [ Aci[i] + plist[i].Aci[2] +  5*j + 2 for j in xrange(nplist[i]) ] # c_c_z (TODO: Assumes flat ground)
        fixed_zeros  += [ Aci[i] + plist[i].Aci[2] +  5*j + 3 for j in xrange(nplist[i]) ] # c_c_w
        nonnegatives += [ Aci[i] + plist[i].Aci[2] +  5*j + 4 for j in xrange(nplist[i]) ] # c_c_n
        nonnegatives += [ Aci[i] + plist[i].Aci[3] +  4*j + 2 for j in xrange(nplist[i]) ] # p_c_2_z
        fixed_zeros  += [ Aci[i] + plist[i].Aci[3] +  4*j + 2 for j in xrange(nplist[i]) ] # p_c_2_z (TODO: How to allow contact break?)
        fixed_ones   += [ Aci[i] + plist[i].Aci[3] +  4*j + 3 for j in xrange(nplist[i]) ] # p_c_2_w
        nonnegatives += range(Aci[i] + plist[i].Aci[5], Aci[i] + plist[i].Aci[6]) # eps_fric
        nonnegatives += range(Aci[i] + plist[i].Aci[6], Aci[i] + plist[i].Aci[7]) # muf_cz
        nonnegatives += range(Aci[i] + plist[i].Aci[8], Aci[i] + plist[i].Aci[9]) # eps_delta
    
    for j in xrange(nm):
        i = Aci[nb+2]+j
        bkx[i], blx[i], bux[i] = mosek.boundkey.ra, flist[j].x_rl, flist[j].x_ru     # Muscle fiber rest length range
        i = Aci[nb+1]+j
        #bkx[i], blx[i], bux[i] = mosek.boundkey.lo, 0, inf                           # Actuation force
    for i in nonnegatives:
        bkx[i], blx[i], bux[i] = mosek.boundkey.lo, 0, inf
    for i in fixed_ones:
        bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 1, 1
    for i in fixed_zeros:
        bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 0, 0
        
    ### DEBUG PURPOSE (REMOVE IT!) ###
    i = Aci[1]+0 # linear x-axis position of body 1 (top)
    #bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 0,0
    i = Aci[1]+1 # linear y-axis position of body 1 (top)
    #bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 0,0
    i = Aci[1]+2 # linear z-axis position of body 1 (top)
    #bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 4.5, 4.5
    i = Aci[1]+3 # linear rx-axis position of body 1 (top)
    #bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 0.1, 0.1
    i = Aci[1]+4 # linear ry-axis position of body 1 (top)
    #bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 0, 0
    i = Aci[1]+5 # linear rz-axis position of body 1 (top)
    #bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 0, 0
    
    i = Aci[2] # Muscle fiber tension
    #bkx[i], blx[i], bux[i] = mosek.boundkey.ra, -plist[0].mass*GRAV, plist[0].mass*GRAV
    
    i = Aci[0]+0 # linear x-axis position of body 1 (top)
    bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 0,0
    i = Aci[0]+1 # linear y-axis position of body 1 (top)
    bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 0,0
    i = Aci[0]+2 # linear z-axis position of body 1 (top)
    bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 0.5, 0.5
    i = Aci[0]+3 # linear rx-axis position of body 0 (bottom)
    bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 0, 0
    i = Aci[0]+4 # linear ry-axis position of body 0 (bottom)
    bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 0, 0
    i = Aci[0]+5 # linear rz-axis position of body 0 (bottom)
    bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 0, 0
    
    #set_printoptions(threshold=nan, linewidth=378, precision=4)
    #print A.todense()
    #print Eta
    
    Acsc = A.tocsc()
    asub = []
    aval = []
    for i in xrange(Acsc.shape[1]):
        asub.append( Acsc.indices[Acsc.indptr[i]:Acsc.indptr[i+1]] )
        aval.append( Acsc.data[Acsc.indptr[i]:Acsc.indptr[i+1]] )

    # Append 'NUMCON' empty constraints.
    # The constraints will initially have no bounds. 
    task.append(mosek.accmode.con,NUMCON)

    #Append 'NUMVAR' variables.
    # The variables will initially be fixed at zero (x=0). 
    task.append(mosek.accmode.var,NUMVAR)

    #Optionally add a constant term to the objective. 
    task.putcfix(0.0)

    for j in range(NUMVAR):
        # Set the linear term c_j in the objective.
        task.putcj(j,c[j])
        # Set the bounds on variable j
        # blx[j] <= x_j <= bux[j] 
        task.putbound(mosek.accmode.var,j,bkx[j],blx[j],bux[j])

    for j in range(len(aval)):
        # Input column j of A
        task.putavec(mosek.accmode.var,  # Input columns of A.
                     j,                  # Variable (column) index.
                     asub[j],            # Row index of non-zeros in column j.
                     aval[j])            # Non-zero Values of column j. 
    for i in range(NUMCON):
        task.putbound(mosek.accmode.con,i,bkc[i],blc[i],buc[i])
    #
    # Input the cones
    #
    for i in xrange(nb):
        # epsilon_Delta >= || Delta_chi_i || (6-DOF)
        task.appendcone(mosek.conetype.quad, 0.0, [ Aci[i] + plist[i].Aci[8] ] + range(Aci[i] + plist[i].Aci[7], Aci[i] + plist[i].Aci[8]))
        for j in xrange(nplist[i]):
            # CP movement minimization
            task.appendcone(mosek.conetype.quad, 0.0, [ Aci[i] + plist[i].Aci[5] + j,
                                                        Aci[i] + plist[i].Aci[4]+4*j+0,
                                                        Aci[i] + plist[i].Aci[4]+4*j+1,
                                                        Aci[i] + plist[i].Aci[4]+4*j+2 ])
            # Friction cone constraints
            task.appendcone(mosek.conetype.quad, 0.0, [ Aci[i] + plist[i].Aci[6]+j,
                                                        Aci[i] + plist[i].Aci[1]+6*j+0,
                                                        Aci[i] + plist[i].Aci[1]+6*j+1 ])
    # Minimal tension force constraints
    task.appendcone(mosek.conetype.quad, 0.0, [ Aci[nb+3] ] + range(Aci[nb+0],Aci[nb+1]))
    # Minimal actuation force constraints
    task.appendcone(mosek.conetype.quad, 0.0, [ Aci[nb+4] ] + range(Aci[nb+1],Aci[nb+2]))

    # Input the objective sense (minimize/maximize)
    task.putobjsense(mosek.objsense.minimize)

    task.writedata('mosek_bugreport.opf')
    # Optimize the task
    task.optimize()
    # Print a summary containing information
    # about the solution for debugging purposes
    task.solutionsummary(mosek.streamtype.msg)
    prosta = []
    solsta = []
    [prosta,solsta] = task.getsolutionstatus(mosek.soltype.itr)

    # Output a solution
    xx = zeros(NUMVAR, float)
    task.getsolutionslice(mosek.soltype.itr,
                          mosek.solitem.xx, 
                          0,NUMVAR,          
                          xx)
    frameReport = ''
    cost = dot(c, xx)
    if solsta == mosek.solsta.optimal or solsta == mosek.solsta.near_optimal:
        return frameReport, xx, cost, Ari, Aci, solsta
    elif solsta == mosek.solsta.dual_infeas_cer: 
        frameReport += "Dual infeasibility.\n"
    elif solsta == mosek.solsta.prim_infeas_cer:
        frameReport += "Primal infeasibility.\n"
    elif solsta == mosek.solsta.near_dual_infeas_cer:
        frameReport += "Near dual infeasibility.\n"
    elif  solsta == mosek.solsta.near_prim_infeas_cer:
        frameReport += "Near primal infeasibility.\n"
    elif solsta == mosek.solsta.unknown:
        frameReport += "?!?!?!?!?!? W A R N I N G : Unknown solution status ?!?!?!?!?!?\n"
        #print("See mosek_investigate.opf for details.")
        #task.writedata('mosek_investigate.opf')
        return frameReport, xx, cost, Ari, Aci, solsta
    else:
        frameReport += "Other solution status?!?!?!?!?"
    return frameReport, None, None, None, None, None

if __name__ == '__main__':
    Main()
    '''
    rb = RigidBody(array([1.,5,2.5,1,2,3]), array([-1.,-2,-3,3,1,2]), 10, array([1.,2.,3.]))
    rb.updateCurrentStateDependentValues()
    A = rb.getAMatrix()
    eta = rb.getEta()
    print type(A)
    print A
    print eta
    '''
