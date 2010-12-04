#include "PymCorePch.h"
#include <iostream>
#include <vector>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <assert.h>
#include "ConvexHullCapi.h"

std::ostream &operator <<(std::ostream &s, const std::pair<double,double> &point )
{
  s << "("
    << point.first
    << ","
    << point.second
    << ")";
  return s;
}

class GrahamScan
{
public :
  GrahamScan( size_t n, double xmin, double xmax, double ymin, double ymax )
    : N( n )
    , x_range( xmin, xmax )
    , y_range( ymin, ymax )
  {
    //
    // In this constructor I generate the N random points asked for
    // by the caller. Their values are randomly assigned in the x/y
    // ranges specified as arguments
    //
    srand( static_cast<unsigned int>( time(NULL) ) );
    for ( size_t i = 0 ; i < N ; i++ ) {
      double x = (double)rand()/RAND_MAX * ( (int)x_range.second - (int)x_range.first + 1 ) + x_range.first;
      double y = (double)rand()/RAND_MAX * ( (int)y_range.second - (int)y_range.first + 1 ) + y_range.first;
      raw_points.push_back( std::make_pair( x, y ) );
    }
  }
  explicit GrahamScan( size_t n )
    : N( n )
    , x_range( -1, 1) /* not used */
    , y_range( -1, 1) /* not used */
    , raw_points(n)
  {
  }
  void set_raw_point(int i, double x, double y) {
    raw_points[i].first = x;
    raw_points[i].second = y;
  }
  //
  // The initial array of points is stored in vectgor raw_points. I first
  // sort it, which gives me the far left and far right points of the hull.
  // These are special values, and they are stored off separately in the left
  // and right members.
  //
  // I then go through the list of raw_points, and one by one determine whether
  // each point is above or below the line formed by the right and left points.
  // If it is above, the point is moved into the upper_partition_points sequence. If it
  // is below, the point is moved into the lower_partition_points sequence. So the output
  // of this routine is the left and right points, and the sorted points that are in the
  // upper and lower partitions.
  //
  void partition_points()
  {
    //
    // Step one in partitioning the points is to sort the raw data
    //
    std::sort( raw_points.begin(), raw_points.end() );
    //
    // The the far left and far right points, remove them from the
    // sorted sequence and store them in special members
    //
    left = raw_points.front();
    raw_points.erase( raw_points.begin() );
    right = raw_points.back();
    raw_points.pop_back();
    //
    // Now put the remaining points in one of the two output sequences
    //
    for ( size_t i = 0 ; i < raw_points.size() ; i++ )
    {
      double dir = direction( left, right, raw_points[ i ] );
      if ( dir < 0 )
        upper_partition_points.push_back( raw_points[ i ] );
      else
        lower_partition_points.push_back( raw_points[ i ] );
    }
  }
  //
  // Building the hull consists of two procedures: building the lower and
  // then the upper hull. The two procedures are nearly identical - the main
  // difference between the two is the test for convexity. When building the upper
  // hull, our rull is that the middle point must always be *above* the line formed
  // by its two closest neighbors. When building the lower hull, the rule is that point
  // must be *below* its two closest neighbors. We pass this information to the 
  // building routine as the last parameter, which is either -1 or 1.
  //
  void build_hull( std::ofstream &f )
  {
    build_half_hull( f, lower_partition_points, lower_hull, 1 );
    build_half_hull( f, upper_partition_points, upper_hull, -1 );
  }
  //
  // This is the method that builds either the upper or the lower half convex
  // hull. It takes as its input a copy of the input array, which will be the
  // sorted list of points in one of the two halfs. It produces as output a list
  // of the points in the corresponding convex hull.
  //
  // The factor should be 1 for the lower hull, and -1 for the upper hull.
  //
  void build_half_hull( std::ostream &f, 
    std::vector< std::pair<double,double> > input,
    std::vector< std::pair<double,double> > &output,
    int factor )
  {
    //
    // The hull will always start with the left point, and end with the right
    // point. According, we start by adding the left point as the first point
    // in the output sequence, and make sure the right point is the last point 
    // in the input sequence.
    //
    input.push_back( right );
    output.push_back( left );
    //
    // The construction loop runs until the input is exhausted
    //
    while ( input.size() != 0 ) {
      //
      // Repeatedly add the leftmost point to the null, then test to see 
      // if a convexity violation has occured. If it has, fix things up
      // by removing the next-to-last point in the output suqeence until 
      // convexity is restored.
      //
      output.push_back( input.front() );
      plot_hull( f, "adding a new point" );
      input.erase( input.begin() );
      while ( output.size() >= 3 ) {
        size_t end = output.size() - 1;
        if ( factor * direction( output[ end - 2 ], 
          output[ end ], 
          output[ end - 1 ] ) <= 0 ) {
            output.erase( output.begin() + end - 1 );
            plot_hull( f, "backtracking" );
        }
        else
          break;
      }
    }
  }
  //
  // In this program we frequently want to look at three consecutive
  // points, p0, p1, and p2, and determine whether p2 has taken a turn
  // to the left or a turn to the right.
  //
  // We can do this by by translating the points so that p1 is at the origin,
  // then taking the cross product of p0 and p2. The result will be positive,
  // negative, or 0, meaning respectively that p2 has turned right, left, or
  // is on a straight line.
  //
  static double direction( std::pair<double,double> p0,
    std::pair<double,double> p1,
    std::pair<double,double> p2 )
  {
    return ( (p0.first - p1.first ) * (p2.second - p1.second ) )
      - ( (p2.first - p1.first ) * (p0.second - p1.second ) );
  }
  void log_raw_points( std::ostream &f )
  {
    f << "Creating raw points:\n";
    for ( size_t i = 0 ; i < N ; i++ ) 
      f << raw_points[ i ] << " ";
    f << "\n";
  }
  void log_partitioned_points( std::ostream &f )
  {
    f << "Partitioned set:\n"
      << "Left : " << left << "\n"
      << "Right : " << right << "\n"
      << "Lower partition: ";
    for ( size_t i = 0 ; i < lower_partition_points.size() ; i++ )
      f << lower_partition_points[ i ];
    f << "\n";
    f << "Upper partition: ";
    for ( size_t i = 0 ; i < upper_partition_points.size() ; i++ )
      f << upper_partition_points[ i ];
    f << "\n";
  }
  void log_hull( std::ostream & f )
  {
    f << "Lower hull: ";
    for ( size_t i = 0 ; i < lower_hull.size() ; i++ )
      f << lower_hull[ i ];
    f << "\n";
    f << "Upper hull: ";
    for ( size_t i = 0 ; i < upper_hull.size() ; i++ )
      f << upper_hull[ i ];
    f << "\n";

    f << "Convex hull: ";
    for ( size_t i = 0 ; i < lower_hull.size() ; i++ )
      f << lower_hull[ i ] << " ";
    for ( std::vector< std::pair<double,double> >::reverse_iterator ii = upper_hull.rbegin() + 1 ;
      ii != upper_hull.rend();
      ii ++ )
      f << *ii << " ";
    f << "\n";

  }
  int get_num_hull_points() const
  {
    return lower_hull.size() + upper_hull.size() - 1;
  }
  void get_hull_points(std::vector<double> &x, std::vector<double> &y) const
  {
    int idx = 0;
    for ( size_t i = 0 ; i < lower_hull.size() ; i++ ) {
      x[idx] = lower_hull[i].first;
      y[idx] = lower_hull[i].second;
      ++idx;
    }
    for ( std::vector< std::pair<double,double> >::const_reverse_iterator ii = upper_hull.rbegin() + 1 ;
      ii != upper_hull.rend();
      ii ++ ) {
      x[idx] = ii->first;
      y[idx] = ii->second;
      ++idx;
    }
    assert(idx == get_num_hull_points());
  }
  void plot_raw_points( std::ostream &f )
  {
    f << "set xrange ["
      << x_range.first 
      << ":"
      << x_range.second
      << "]\n";
    f << "set yrange ["
      << y_range.first 
      << ":"
      << y_range.second
      << "]\n";
    f << "unset mouse\n";
    f << "set title 'The set of raw points in the set' font 'Arial,12'\n";
    f << "set style line 1 pointtype 7 linecolor rgb 'red'\n";
    f << "set style line 2 pointtype 7 linecolor rgb 'green'\n";
    f << "set style line 3 pointtype 7 linecolor rgb 'black'\n";
    f << "plot '-' ls 1 with points notitle\n";
    for ( size_t i = 0 ; i < N ; i++ )
      f << raw_points[ i ].first << " " << raw_points[ i ].second << "\n";
    f << "e\n";
    f << "pause -1 'Hit OK to move to the next state'\n";
  }
  void plot_partitioned_points( std::ostream &f )
  {
    f << "set title 'The points partitioned into an upper and lower hull' font 'Arial,12'\n";
    f << "plot '-' ls 1 with points notitle, "
      << "'-' ls 2 with points notitle, "
      << "'-' ls 3 with linespoints notitle\n";
    for ( size_t i = 0 ; i < lower_partition_points.size() ; i++ )
      f << lower_partition_points[ i ].first 
      << " " 
      << lower_partition_points[ i ].second 
      << "\n";
    f << "e\n";
    for ( size_t i = 0 ; i < upper_partition_points.size() ; i++ )
      f << upper_partition_points[ i ].first 
      << " " 
      << upper_partition_points[ i ].second 
      << "\n";
    f << "e\n";
    f << left.first << " " << left.second << "\n";
    f << right.first << " " << right.second << "\n";
    f << "e\n";
    f << "pause -1 'Hit OK to move to the next state'\n";
  }
  void plot_hull( std::ostream &f, std::string text )
  {
    return;
    f << "set title 'The hull in state: "
      << text
      << "' font 'Arial,12'\n";
    f << "plot '-' ls 1 with points notitle, ";
    if ( lower_hull.size() )
      f << "'-' ls 3 with linespoints notitle, ";
    if ( upper_hull.size() )
      f << "'-' ls 3 with linespoints notitle, ";
    f << "'-' ls 2 with points notitle\n";
    for ( size_t i = 0 ; i < lower_partition_points.size() ; i++ )
      f << lower_partition_points[ i ].first 
      << " " 
      << lower_partition_points[ i ].second 
      << "\n";
    f << right.first << " " << right.second << "\n";
    f << "e\n";
    if ( lower_hull.size() ) {
      for ( size_t i = 0 ; i < lower_hull.size() ; i++ )
        f << lower_hull[ i ].first 
        << " " 
        << lower_hull[ i ].second 
        << "\n";
      f << "e\n";
    }
    if ( upper_hull.size() ) {
      for ( std::vector< std::pair<double,double> >::reverse_iterator ii = upper_hull.rbegin();
        ii != upper_hull.rend();
        ii++ ) 
        f << ii->first 
        << " " 
        << ii->second 
        << "\n";
      f << "e\n";
    }
    for ( size_t i = 0 ; i < upper_partition_points.size() ; i++ )
      f << upper_partition_points[ i ].first 
      << " " 
      << upper_partition_points[ i ].second 
      << "\n";
    f << "e\n";
    f << "pause -1 'Hit OK to move to the next state'\n";
  }
private :
  //
  // These values determine the range of numbers generated to 
  // provide the input data. The values are all passed in as part
  // of the constructor
  //
  const size_t N;
  const std::pair<double,double> x_range;
  const std::pair<double,double> y_range;
  // The raw data points generated by the constructor
  std::vector< std::pair<double,double> > raw_points;
  //
  // These values are used to represent the partitioned set. A special
  // leftmost and rightmost value, and the sorted set of upper and lower
  // partitioned points that lie inside those two points.
  //
  std::pair<double,double> left;
  std::pair<double,double> right;
  std::vector< std::pair<double,double> > upper_partition_points;
  std::vector< std::pair<double,double> > lower_partition_points;
  //
  // After the convex hull is created, the lower hull and upper hull
  // are stored in these sorted sequences. There is a bit of duplication
  // between the two, because both sets include the leftmost and rightmost point.
  //
  std::vector< std::pair<double,double> > lower_hull;
  std::vector< std::pair<double,double> > upper_hull;
};

int PymConvexHull(Point_C *P, int n, Point_C *H) {
  assert(n < 1000);
  if (n < 3)
    return 0;

  std::ofstream null_file( "nul" );
  GrahamScan g(n);
  for (int i = 0; i < n; ++i) {
    g.set_raw_point(i, P[i].x, P[i].y);
  }
  g.partition_points();
  g.build_hull(null_file);
  g.log_hull(null_file);
  int nhull = g.get_num_hull_points();
  std::vector<double> out_x(nhull), out_y(nhull);  
  g.get_hull_points(out_x, out_y);
  for (int i = 0; i < nhull; ++i) {
    H[i].x = out_x[i];
    H[i].y = out_y[i];
  }
  return nhull;
}
