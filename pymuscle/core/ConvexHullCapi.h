#ifndef CONVEXHULLCAPI_H
#define CONVEXHULLCAPI_H

typedef struct _Point_C {
    double x, y;
} Point_C;

PYMCORE_API int PymConvexHull(Point_C *P, int n, Point_C *H);

#endif // #ifndef CONVEXHULLCAPI_H
