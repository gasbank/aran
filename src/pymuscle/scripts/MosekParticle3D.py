#
#  Copyright: Copyright (c) 1998-2009 MOSEK ApS, Denmark. All rights reserved.
#
#  File:    lo2.py
#
#  Purpose: Demonstrates how to solve small linear
#           optimization problem using the MOSEK Python API.
##

import sys
from numpy import array,zeros,ones
import mosek

# Since the actual value of Infinity is ignores, we define it solely
# for symbolic purposes:
inf = 0.0

# Define a stream printer to grab output from MOSEK
def streamprinter(text):
    sys.stdout.write(text)
    sys.stdout.flush()

# We might write everything directly as a script, but it looks nicer
# to create a function.
def main ():
    global xx
    global chi_0, chi_1
    chi_2 = 2*chi_1 - chi_0 + h**2/mass*f_g
    if chi_2[2] > 0 or chi_2[2]-chi_1[2] > 0:
        chi_0 = chi_1
        chi_1 = chi_2
        return False


    # Attach a printer to the environment
    #env.set_Stream (mosek.streamtype.log, streamprinter)

    # Create a task
    task = env.Task(0,0)


    # Attach a printer to the task
    #task.set_Stream (mosek.streamtype.log, streamprinter)

    # x_opt = [ eps, px, py, pz, fcx, fcy, fcz ]
    #
    #                                                [ eps ]
    #                                                [ px  ]
    #                                                [ py  ]
    #   [ 0   m/h            -1                    ] [ pz  ]
    #   [ 0       m/h           -1                 ] [ fcx ] =  [ m/h(2*chi_1 - chi_0) + fg ]
    #   [ 0            m/h        -1               ] [ fcy ]    [           chi_1           ]
    #   [ 0    1                       1           ] [ fcz ]
    #   [ 0        1                        1      ] [ dx  ]
    #   [ 0             1                       1  ] [ dy  ]
    #                                                [ dz  ]




    bc = mass/h**2*(2*chi_1 - chi_0) + f_g
    bkc = [ mosek.boundkey.fx, mosek.boundkey.fx, mosek.boundkey.fx, mosek.boundkey.fx, mosek.boundkey.fx, mosek.boundkey.fx ]
    blc = list(bc) + list(chi_1)
    buc = list(bc) + list(chi_1)

    c   = [1.0] + [0., 0., 1.,
                   0., 0., 0,
                   0, 0, 0]
    bkx = [ mosek.boundkey.fr,
            mosek.boundkey.fr,mosek.boundkey.fr, mosek.boundkey.lo,
            mosek.boundkey.fr,mosek.boundkey.fr,mosek.boundkey.lo,
            mosek.boundkey.fr,mosek.boundkey.fr,mosek.boundkey.fr ]
    blx = [             -inf ,
                        -inf,          -inf,             0,
                        -inf,            -inf,                 0,
                        -inf, -inf, -inf          ]
    bux = [              inf,
                         inf,           inf,           inf,
                         inf,             inf,                inf,
                         inf, inf, inf         ]

    asub = [ array([]), array([0,3]),         array([1,4]),         array([2,5]),         array([0]),   array([1]),   array([2]),    array([3]), array([4]),   array([5])  ]
    aval = [ array([]), array([mass/h**2,1]), array([mass/h**2,1]), array([mass/h**2,1]), array([-1.0]),array([-1.0]),array([-1.0]), array([-1.]), array([-1.]),   array([-1.])  ]


    NUMVAR = len(bkx)
    NUMCON = len(bkc)
    NUMANZ = 4
    # Give MOSEK an estimate of the size of the input data. 
    #  This is done to increase the speed of inputting data. 
    #  However, it is optional. 
    task.putmaxnumvar(NUMVAR)
    task.putmaxnumcon(NUMCON)
    task.putmaxnumanz(NUMANZ)
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

    # Input the cones
    task.appendcone(mosek.conetype.quad,
                    0.0,
                    [ 0, 7, 8, 9 ])
    task.appendcone(mosek.conetype.quad,
                    0.0,
                    [ 6, 4, 5 ])                  


    # Input the objective sense (minimize/maximize)
    task.putobjsense(mosek.objsense.minimize)

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

    if solsta == mosek.solsta.optimal or solsta == mosek.solsta.near_optimal:
        #print("Optimal solution: %s" % xx)
        pass
    elif solsta == mosek.solsta.dual_infeas_cer: 
        print("Primal or dual infeasibility.\n")
    elif solsta == mosek.solsta.prim_infeas_cer:
        print("Primal or dual infeasibility.\n")
    elif solsta == mosek.solsta.near_dual_infeas_cer:
        print("Primal or dual infeasibility.\n")
    elif  solsta == mosek.solsta.near_prim_infeas_cer:
        print("Primal or dual infeasibility.\n")
    elif mosek.solsta.unknown:
        print("Unknown solution status")
    else:
        print("Other solution status")

    chi_0 = chi_1
    chi_1 = array(xx[1:4])
    return True



# Make a MOSEK environment
xx = zeros(100)
env = mosek.Env ()
mosek.iparam.intpnt_num_threads = 4

mass = 1.
grav = 9.81
f_g = array([0, 0, -mass*grav])
h = 0.01
chi_0 = array([-0.1,0.2,5])
chi_1 = array([0.,0,5])

# call the main function
try:
    for i in range(1200):
        print 'Frame', i, ':', chi_1, xx[4:7]
        contacted = main ()
        #if contacted: break
except mosek.Exception, msg:
    if msg is not None:
        print "\t%s" % msg
        sys.exit(1)
except:
    import traceback
    traceback.print_exc()
    sys.exit(1)

print 'Frame', i, ':', chi_1
