#ifndef EXPBODYMOEQ_REAL_H_INCLUDED
#define EXPBODYMOEQ_REAL_H_INCLUDED

void MassMatrixAndCqdVector(double M[6][6], double Cqd[6],
                            const double p[3], const double v[3],
                            const double pd[3], const double vd[3],
                            const double I[4]);

#endif // EXPBODYMOEQ_REAL_H_INCLUDED
