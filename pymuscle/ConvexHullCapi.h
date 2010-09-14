#ifndef CONVEXHULLCAPI_H
#define CONVEXHULLCAPI_H

#ifdef __cplusplus
#define PYMCONVEXHULL_API extern "C"
#else
#define PYMCONVEXHULL_API
#endif

typedef struct _Point_C {
    double x, y;
} Point_C;

PYMCONVEXHULL_API int PymConvexHull(Point_C *P, int n, Point_C *H);

#endif // #ifndef CONVEXHULLCAPI_H
