
// CodeSnippet provided by John W. Ratcliff
// on March 23, 2006.
//
// mailto: jratcliff@infiniplex.net
//
// Personal website: http://jratcliffscarab.blogspot.com
// Coding Website:   http://codesuppository.blogspot.com
// FundRaising Blog: http://amillionpixels.blogspot.com
// Fundraising site: http://www.amillionpixels.us
// New Temple Site:  http://newtemple.blogspot.com
//
// This snippet provides some generally useful functions
// with plane equations.  Each function can be used
// just by itself and is not dependent on any other.
//
// All functions accept 'const float *' as input and
// a 'float *' for output.  You should be able to cast
// your vector or plane class in virtually any engine
// to a 'float *' and pass it to this routine.  In fact
// if you can't do that, you are a seriously screwed up
// looking math library.

#include <math.h> // need square root prototype.

// compute the distance between a 3d point and a plane
inline float distToPt(const float *p,const float *plane)
{
	return p[0]*plane[0] + p[1]*plane[1] + p[2]*plane[2] + plane[3];
}


// Returns true if the 3d point is in front of the Plane within a user specified 'epsilon' value.
inline bool getSidePlane(const float *p,const float *plane,float epsilon)
{
	bool ret = false;

	float d = p[0]*plane[0] + p[1]*plane[1] + p[2]*plane[2] + plane[3];

  if ( (d+epsilon) > 0 ) ret = true;

  return ret;
}



// inertesect a line semgent with a plane, return false if they don't intersect.
// otherwise computes and returns the intesection point 'split'
// p1 = 3d point of the start of the line semgent.
// p2 = 3d point of the end of the line segment.
// split = address to store the intersection location x,y,z.
// plane = the plane equation as four floats A,B,C,D.
bool intersectLinePlane(const float *p1,const float *p2,float *split,const float *plane)
{

  float dp1 = p1[0]*plane[0] + p1[1]*plane[1] + p1[2]*plane[2] + plane[3];
  float dp2 = p2[0]*plane[0] + p2[1]*plane[1] + p2[2]*plane[2] + plane[3];

	if ( dp1 > 0 && dp2 > 0 ) return false;
	if ( dp1 < 0 && dp2 < 0 ) return false;

  float dir[3];

  dir[0] = p2[0] - p1[0];
  dir[1] = p2[1] - p1[1];
  dir[2] = p2[2] - p1[2];

  float dot1 = dir[0]*plane[0] + dir[1]*plane[1] + dir[2]*plane[2];
  float dot2 = dp1 - plane[3];

  float    t = -(plane[3] + dot2 ) / dot1;

  split[0] = (dir[0]*t)+p1[0];
  split[1] = (dir[1]*t)+p1[1];
  split[2] = (dir[2]*t)+p1[2];

  return true;
}


// compute the plane equation from a set of 3 points.
// returns false if any of the points are co-incident and do not form a plane.
// A = point 1
// B = point 2
// C = point 3
// plane = destination for plane equation A,B,C,D
bool computePlane(const float *A,const float *B,const float *C,float *plane)
{
	bool ret = false;

	float vx = (B[0] - C[0]);
	float vy = (B[1] - C[1]);
	float vz = (B[2] - C[2]);

	float wx = (A[0] - B[0]);
	float wy = (A[1] - B[1]);
	float wz = (A[2] - B[2]);

	float vw_x = vy * wz - vz * wy;
	float vw_y = vz * wx - vx * wz;
	float vw_z = vx * wy - vy * wx;

	float mag = sqrtf((vw_x * vw_x) + (vw_y * vw_y) + (vw_z * vw_z));

	if ( mag > 0.000001f )
	{

		mag = 1.0f/mag; // compute the reciprocol distance

		ret = true;

    plane[0] = vw_x * mag;
    plane[1] = vw_y * mag;
    plane[2] = vw_z * mag;
    plane[3] = 0.0f - ((plane[0]*A[0])+(plane[1]*A[1])+(plane[2]*A[2]));

  }
  return ret;
}

