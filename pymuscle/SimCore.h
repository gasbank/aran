#ifndef SIMCORE_H_INCLUDED
#define SIMCORE_H_INCLUDED


int SimCore(const double h, const int nBody, const int nMuscle,
            const int nd, const int nY,
            double body[nBody][2*nd + 4], double extForce[nBody][6],
            double muscle[nMuscle][1 + 11], unsigned int musclePair[nMuscle][2],
            double *cost, double ustar[nMuscle],
            double Ydesired[nY], double w_y[nY], double w_u[nY],
            cholmod_sparse *W_Ysp, cholmod_sparse *W_usp,
            cholmod_sparse *Fsp,cholmod_common *c);

#endif // SIMCORE_H_INCLUDED
