#ifndef __FIBEREFFECTIMPALL_H__


int ImpAll(
           const unsigned int nBody,
                       const unsigned int nMuscle,
                       TripletMatrix *dfdY_R[nBody],
                       TripletMatrix *dfdY_Q[nMuscle],
                       double f[nBody*14 + nMuscle],
                       double body[nBody][18],
                       double extForce[nBody][6],
                       double muscle[nMuscle][12],
                       unsigned int musclePair[nMuscle][2],
                       const double h);

#endif
