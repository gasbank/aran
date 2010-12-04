#ifndef EXPBODYMOEQ_REAL_H_INCLUDED
#define EXPBODYMOEQ_REAL_H_INCLUDED

// Calculates coefficients of equations of motion
//        ..           .    .
// M(chi)chi + C(chi, chi) chi = f
// M(chi) --> mass matrix M[6][6]
//         .    .
// C(chi, chi) chi --> Cqd[6]
void MassMatrixAndCqdVector(double M[3+3][3+3], double Cqd[3+3],
                            const double p[3], const double v[3],
                            const double pd[3], const double vd[3],
                            const double I[4]);

// Calculates coefficients of equations of motion
//        ..           .    .
// M(chi)chi + C(chi, chi) chi = f
// M(chi) --> mass matrix M[6][7]
//         .    .
// C(chi, chi) chi --> Cqd[6]
void MassMatrixAndCqdVectorQuat(double M[3+4][3+4], double Cqd[3+4],
  const double p[3], const double q[4],
  const double pd[3], const double qd[4],
  const double I[4]);

#endif // EXPBODYMOEQ_REAL_H_INCLUDED
