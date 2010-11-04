#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "ConvexHull.h"
#include "ConvexHullCapi.h"

int IsectsSort(const void *v1, const void *v2)
{
  return ((Point_C *)v1)->x > ((Point_C *)v2)->x;
}

int PymConvexHull(Point_C *P, int n, Point_C *H) {
  qsort(P, n, sizeof(Point_C), IsectsSort); // Should be sorted before applying CH algo.
  std::vector<ConvexHull::Point> isectsCgal(n);
  for (int i = 0; i < n; ++i) {
    isectsCgal[i] = ConvexHull::Point(P[i].x, P[i].y);
    std::cout << "CH points " << P[i].x << " " << P[i].y << std::endl;
  }
  std::vector<ConvexHull::Point> out(isectsCgal.size() + 1);
  int outEnd = ConvexHull::chainHull_2D(&isectsCgal[0], isectsCgal.size(), &out[0]);
  for (int i = 0; i < outEnd; ++i) {
    H[i].x = out[i].x;
    H[i].y = out[i].y;
  }
  return outEnd;
}

