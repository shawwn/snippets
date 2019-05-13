#ifndef LOOK_AT_H

#define LOOK_AT_H

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
// This is a reference implementation that you should be able to use
// with virtually any vector or matrix class (so long as the matrices are (4x4)
// and your vector is 3 floats.  And who has a vector class that isn't three floats X,Y,Z ?
//
// The first one is a helpful routine to get an object to world transform if you know the origin
// of an object and some place it is 'looking'.  You must specify the 'up vector' for that object.
//
// I used this helpful routine when I wanted to place a bunch of dominos in a spiral.  Any time
// you have two points and you just want an object to 'face that way' you can use
// 'computeLookAt'
//
// computeView builds a view matrix from an eye position to a look at location with a
// reference 'up vector'.  It stores the result in a 4x4 matrix that use pass by address.
//
// The last routine computes a projection matrix.
//
// You can either use these three routines simply as a reference implementation or
// you can build your own camera classes out of them.  Maybe you don't even think they are
// useful, I don't know.  I, personally, like being able to use them independent of any
// vector or matrix classes.  I will be using them for the demo of the axis-aligned bounding
// volume tree code I will be releasing in a day or so.


void computeLookAt(const float *eye,const float *look,const float *upVector,float *matrix);
void computeView(const float *eye,const float *look,const float *upVector,float *matrix);
void computeProjection(float fov,float aspect,float nearPlane,float farPlane,float *matrix);

#endif
