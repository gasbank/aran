#ifndef SIMCORE_H_INCLUDED
#define SIMCORE_H_INCLUDED


void SimCore(const double h, const int nBody, const int nMuscle,
             double body[nBody][18], double extForce[nBody][6],
             double muscle[nMuscle][12], unsigned int musclePair[nMuscle][2]);

#endif // SIMCORE_H_INCLUDED
