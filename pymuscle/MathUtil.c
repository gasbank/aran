#include <math.h>

double NormalizeVector(int dim, double v[dim])
{
    double len = 0;
    int i;
    for (i = 0; i < dim; ++i)
        len += v[i]*v[i];
    len = sqrt(len);
    for (i = 0; i < dim; ++i)
        v[i] /= len;
    return len;
}
