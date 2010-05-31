#ifndef _SYMBOLICTENSOR_H_
#define _SYMBOLICTENSOR_H_

void SymbolicTensor(double I[4],
                    const double sx,
                    const double sy,
                    const double sz,
                    const double rho);
void BoxCorners(double corners[8][3], const double sx, const double sy, const double sz);
void BoxCornersOffset(double corners[8][3], const double sx, const double sy, const double sz, const double offx, const double offy, const double offz);
#endif
