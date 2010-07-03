void SymbolicTensor(double I[4],
                    const double sx,
                    const double sy,
                    const double sz,
                    const double rho) {
    I[0] = rho*(sx*sx*sx)*sy*sz*(1.0/1.2E1);
	I[1] = rho*sx*(sy*sy*sy)*sz*(1.0/1.2E1);
	I[2] = rho*sx*sy*(sz*sz*sz)*(1.0/1.2E1);
	I[3] = rho*sx*sy*sz;
}

void BoxCorners(double corners[8][3], const double sx, const double sy, const double sz) {
    int i;
    for (i=0; i<8; ++i) {
        corners[i][0] = (1 - 2*( (i&4)>>2  )) * sx/2;
        corners[i][1] = (1 - 2*( (i&2)>>1  )) * sy/2;
        corners[i][2] = (1 - 2*(  i&1      )) * sz/2;
    }
}
