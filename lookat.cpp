#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "lookat.h"

// CodeSnippet provided by John W. Ratcliff
// on April 3, 2006.
//
// mailto: jratcliff@infiniplex.net
//
// Personal website: http://jratcliffscarab.blogspot.com
// Coding Website:   http://codesuppository.blogspot.com
// FundRaising Blog: http://amillionpixels.blogspot.com
// Fundraising site: http://www.amillionpixels.us
// New Temple Site:  http://newtemple.blogspot.com
//
// This snippet shows how to build a projection matrix
// a view matrix and a matrix that orientats an object
// relative to an origin and lookat location.
//


static inline float Dot(const float *A,const float *B)
{
  return A[0]*B[0] + A[1]*B[1] + A[2]*B[2];
}

static inline void Cross(float *result,const float *A,const float *B)
{
	result[0] = A[1]*B[2] - A[2]*B[1];
	result[1] = A[2]*B[0] - A[0]*B[2];
	result[2] = A[0]*B[1] - A[1]*B[0];
}

static inline float Normalize(float *r)
{
	float d = sqrtf(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
	float recip = 1.0f / d;
	r[0]*=recip;
	r[1]*=recip;
	r[2]*=recip;
	return d;
}

static inline float Magnitude(const float *v)
{
  return sqrtf( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
}

void computeLookAt(const float *eye,const float *look,const float *upVector,float *matrix)
{

  float row2[3];

  row2[0] = look[0] - eye[0];
  row2[1] = look[1] - eye[1];
  row2[2] = look[2] - eye[2];

	Normalize(row2);

	float row0[3];
	float	row1[3];

  Cross(row0, upVector, row2 );
  Cross(row1, row2, row0 );

  Normalize(row0);
  Normalize(row1);

	matrix[0] = row0[0];
	matrix[1] = row0[1];
	matrix[2] = row0[2];
	matrix[3] = 0;

	matrix[4+0] = row1[0];
	matrix[4+1] = row1[1];
	matrix[4+2] = row1[2];
	matrix[4+3] = 0;

	matrix[8+0] = row2[0];
	matrix[8+1] = row2[1];
	matrix[8+2] = row2[2];
	matrix[8+3] = 0;

	matrix[12+0] = eye[0];
	matrix[12+1] = eye[1];
	matrix[12+2] = eye[2];
	matrix[12+3] = 1;

}

void computeProjection(float fovY,float aspect,float near_plane,float far_plane,float *matrix)
{

  memset(matrix,0,sizeof(float)*16);

  float  yScale = 1.0f/tanf(fovY*0.5f);  // 1/tan(x) == cot(x)
  float  xScale = yScale / aspect;

  float  Q = far_plane/(far_plane - near_plane);

  matrix[0*4+0] = xScale;
  matrix[1*4+1] = yScale;
  matrix[2*4+2] = Q;
  matrix[3*4+2] = -Q*near_plane;
  matrix[2*4+3] = 1;

}


void computeView(const float *vFrom,const float *vAt,const float *vWorldUp,float *matrix)
{
	// Get the z basis vector, which points straight ahead. This is the
	// difference from the eyepoint to the lookat point.
	float vView[3];

	vView[0] = vAt[0] - vFrom[0];
	vView[1] = vAt[1] - vFrom[1];
	vView[2] = vAt[2] - vFrom[2];

	float fLength = Magnitude(vView);

	if ( fLength < 1e-6f ) return; // don't set it, it's bogus.

	// Normalize the z basis vector
	float recip = 1.0f /fLength;
	vView[0]*=recip;
	vView[1]*=recip;
	vView[2]*=recip;

	// Get the dot product, and calculate the projection of the z basis
	// vector onto the up vector. The projection is the y basis vector.
	float fDotProduct = Dot(vWorldUp,vView);

	float vUp[3];

	vUp[0] = vWorldUp[0] - fDotProduct*vView[0];
	vUp[1] = vWorldUp[1] - fDotProduct*vView[1];
	vUp[2] = vWorldUp[2] - fDotProduct*vView[2];

	// If this vector has near-zero length because the input specified a
	// bogus up vector, let's try a default up vector
	if( 1e-6f > ( fLength = Magnitude(vUp) ) )
	{
		vUp[0] = 0.0f - vView[1]*vView[0];
		vUp[1] = 1.0f - vView[1]*vView[1];
		vUp[2] = 0.0f - vView[1]*vView[2];

		// If we still have near-zero length, resort to a different axis.
		if( 1e-6f > ( fLength = Magnitude(vUp) ) )
		{
			vUp[0] = 0.0f - vView[2]*vView[0];
			vUp[1] = 0.0f - vView[2]*vView[1];
			vUp[2] = 1.0f - vView[2]*vView[2];

			if( 1e-6f > ( fLength = Magnitude(vUp) ) )  return;
		}
	}

	// Normalize the y basis vector
	recip = 1.0f / fLength;

	vUp[0]*=recip;
	vUp[1]*=recip;
	vUp[2]*=recip;

	// The x basis vector is found simply with the cross product of the y
	// and z basis vectors

	float vRight[3];

	vRight[0] = vUp[1]*vView[2] - vUp[2]*vView[1];
	vRight[1] = vUp[2]*vView[0] - vUp[0]*vView[2];
	vRight[2] = vUp[0]*vView[1] - vUp[1]*vView[0];

	// Start building the matrix. The first three rows contains the basis
	// vectors used to rotate the view to point at the lookat point

	matrix[0*4+0] = vRight[0];
	matrix[0*4+1] = vUp[0];
	matrix[0*4+2] = vView[0];
  matrix[0*4+3] = 0;

	matrix[1*4+0] = vRight[1];
	matrix[1*4+1] = vUp[1];
	matrix[1*4+2] = vView[1];
  matrix[1*4+3] = 0;

	matrix[2*4+0] = vRight[2];
	matrix[2*4+1] = vUp[2];
	matrix[2*4+2] = vView[2];
  matrix[2*4+3] = 0;

	// Do the translation values (rotations are still about the eyepoint)

	matrix[3*4+0] = -Dot(vFrom,vRight);
	matrix[3*4+1] = -Dot(vFrom,vUp);
	matrix[3*4+2] = -Dot(vFrom,vView);
  matrix[3*4+3] = 1;
}
