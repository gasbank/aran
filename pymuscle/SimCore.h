#ifndef SIMCORE_H_INCLUDED
#define SIMCORE_H_INCLUDED

int SimCore(const double h, const unsigned int nBody, const unsigned int nMuscle,
            const unsigned int nd, const unsigned int nY,
            double body[nBody][2*nd + 4], double extForce[nBody][6],
            double muscle[nMuscle][1 + 11], unsigned int musclePair[nMuscle][2],
            double *cost, double *cost2, double ustar[nMuscle],
            double Ydesired[nY], double w_y[nY], double w_u[nMuscle],
            cholmod_sparse *W_Ysp, cholmod_sparse *W_usp,
            cholmod_common *c);

#endif // SIMCORE_H_INCLUDED
