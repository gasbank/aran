#pragma once

// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

// Assume that a class is already given for the object:
//    Point with coordinates {float x, y;}
//===================================================================

namespace ConvexHull
{
	struct Point
	{
		Point() {}
		Point(double _x, double _y) : x(_x), y(_y) {}
		double x, y;
	};

	// isLeft(): tests if a point is Left|On|Right of an infinite line.
	//    Input:  three points P0, P1, and P2
	//    Return: >0 for P2 left of the line through P0 and P1
	//            =0 for P2 on the line
	//            <0 for P2 right of the line
	//    See: the January 2001 Algorithm on Area of Triangles
	inline double
		isLeft( Point P0, Point P1, Point P2 )
	{
		return (P1.x - P0.x)*(P2.y - P0.y) - (P2.x - P0.x)*(P1.y - P0.y);
	}
	//===================================================================


	// chainHull_2D(): Andrew's monotone chain 2D convex hull algorithm
	//     Input:  P[] = an array of 2D points
	//                   presorted by increasing x- and y-coordinates
	//             n = the number of points in P[]
	//     Output: H[] = an array of the convex hull vertices (max is n)
	//     Return: the number of points in H[]
	int
		chainHull_2D( Point* P, int n, Point* H );
}
