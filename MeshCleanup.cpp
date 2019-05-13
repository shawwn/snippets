#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <assert.h>

/*!
**
** Copyright (c) 2007 by John W. Ratcliff mailto:jratcliff@infiniplex.net
**
** Portions of this source has been released with the PhysXViewer application, as well as
** Rocket, CreateDynamics, ODF, and as a number of sample code snippets.
**
** If you find this code useful or you are feeling particularily generous I would
** ask that you please go to http://www.amillionpixels.us and make a donation
** to Troy DeMolay.
**
** DeMolay is a youth group for young men between the ages of 12 and 21.
** It teaches strong moral principles, as well as leadership skills and
** public speaking.  The donations page uses the 'pay for pixels' paradigm
** where, in this case, a pixel is only a single penny.  Donations can be
** made for as small as $4 or as high as a $100 block.  Each person who donates
** will get a link to their own site as well as acknowledgement on the
** donations blog located here http://www.amillionpixels.blogspot.com/
**
** If you wish to contact me you can use the following methods:
**
** Skype Phone: 636-486-4040 (let it ring a long time while it goes through switches)
** Skype ID: jratcliff63367
** Yahoo: jratcliff63367
** AOL: jratcliff1961
** email: jratcliff@infiniplex.net
**
**
** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/


#pragma warning(disable:4267)

#define SHOW_DEBUG 0 // only enabled for debugging purposes


#include "MeshCleanup.h"
#include "VertexLookup.h"
#include "FloatMath.h"
#include "IntPtr.h"

#if SHOW_DEBUG
#include "RenderDebug/RenderDebug.h"
#endif

#include <vector>

#pragma warning(disable:4244) // ignore warning message


namespace MESH_CLEANUP
{

#define POW21 (1<<21)

UINT64 computeHash(unsigned int i1,unsigned int i2,unsigned int i3)
{
  UINT64 ret = 0;

  assert( i1 < POW21 );
  assert( i2 < POW21 );
  assert( i3 < POW21 );

  UINT64 ui1 = (UINT64)i1;
  UINT64 ui2 = (UINT64)i2;
  UINT64 ui3 = (UINT64)i3;

  ret = (ui1<<42)|(ui2<<21)|ui3;

  return ret;
}

class QuickTri
{
public:
  QuickTri(void)
  {
    mI[0] = mI[1] = mI[2] = 0;
    mDoubleSided = false;
    mJoined = -1;
    mMatch[0] = mMatch[1] = mMatch[2] = 0;
  }

  UINT64 getHash(void) const
  {
    return computeHash(mI[0],mI[1],mI[2]);
  }

  bool identical(const IntInt<UINT64, unsigned int> &edgeList) const
  {
    bool ret = false;

    unsigned int v = edgeList.get( computeHash(mI[0],mI[1],mI[2]) );
    if ( v == 0 )
    {
      v = edgeList.get( computeHash(mI[1],mI[2],mI[0]) );
      if ( v == 0 )
      {
        v = edgeList.get( computeHash(mI[2],mI[0],mI[1]) );
      }
    }

    if ( v != 0 ) ret = true; // if we found this ordering of indices already in the hash table, then this triangle is a duplicate!

    return ret;
  }

  unsigned int isDoubleSided(const IntInt<UINT64, unsigned int> &edgeList)
  {
    unsigned int ret = 0;

    ret = edgeList.get( computeHash(mI[2],mI[1],mI[0]) );
    if ( ret == 0 )
    {
      ret = edgeList.get( computeHash(mI[0],mI[2],mI[1]) );
      if ( ret == 0 )
      {
        ret = edgeList.get( computeHash(mI[1],mI[0],mI[2]) );
        if ( ret != 0 )
        {
          mMatch[0] = 1;
          mMatch[1] = 0;
          mMatch[2] = 2;
        }
      }
      else
      {
        mMatch[0] = 0;
        mMatch[1] = 2;
        mMatch[2] = 1;
      }
    }
    else
    {
      mMatch[0] = 2;
      mMatch[1] = 1;
      mMatch[2] = 0;
    }

    return ret;

  }

  void computePlane(const float *vertices)
  {
    const float *p1 = &vertices[mI[0]*3];
    const float *p2 = &vertices[mI[1]*3];
    const float *p3 = &vertices[mI[2]*3];
    mPlane[3] = fm_computePlane(p3,p2,p1,mPlane);
  }

  void project(const float *p,float *t,const float *plane,float scale)
  {
    t[0] = p[0]+(plane[0]*scale);
    t[1] = p[1]+(plane[1]*scale);
    t[2] = p[2]+(plane[2]*scale);
  }

#if SHOW_DEBUG
  void debugMe(RENDER_DEBUG::RenderDebug *debug,const float *vertices,unsigned int i1,unsigned int i2,unsigned int i3)
  {
    if ( debug )
    {
      const float *p1 = &vertices[i1*3];
      const float *p2 = &vertices[i2*3];
      const float *p3 = &vertices[i3*3];
      debug->DebugTri(p1,p2,p3,0xFF4000,6000.0f);
    }
  }

  void debugSphere(RENDER_DEBUG::RenderDebug *debug,const float *vertices,unsigned int i,unsigned int color)
  {
    if ( debug )
    {
      const float *p = &vertices[i*3];
      debug->DebugSphere(p,0.1f,color,6000.0f);
    }
  }
#endif

  void performProject(VertexLookup vlook,const float *vertices,QuickTri *join,std::vector< unsigned int > &indices,float projectDistance,RENDER_DEBUG::RenderDebug *debug)
  {

    const float *p1 = &vertices[mI[0]*3];
    const float *p2 = &vertices[mI[1]*3];
    const float *p3 = &vertices[mI[2]*3];

    if ( mDoubleSided )
    {
      float tp1[3];
      float tp2[3];
      float tp3[3];

      project(p1,tp1,mPlane,projectDistance);
      project(p2,tp2,mPlane,projectDistance);
      project(p3,tp3,mPlane,projectDistance);

      mT[0] = Vl_getIndex(vlook,tp1);
      mT[1] = Vl_getIndex(vlook,tp2);
      mT[2] = Vl_getIndex(vlook,tp3);
#if SHOW_DEBUG
      if ( debug )
      {
        debug->DebugTri(tp1,tp2,tp3,0xFFFF00,6000.0f);
      }
#endif
    }
    else
    {
      mT[0] = Vl_getIndex(vlook,p1);
      mT[1] = Vl_getIndex(vlook,p2);
      mT[2] = Vl_getIndex(vlook,p3);
    }

    indices.push_back(mT[0]);
    indices.push_back(mT[1]);
    indices.push_back(mT[2]);

    if ( join )
    {

      unsigned int j1 = join->mT[mMatch[0]];
      unsigned int j2 = join->mT[mMatch[1]];
      unsigned int j3 = join->mT[mMatch[2]];

      unsigned int i1 = mT[0];
      unsigned int i2 = mT[1];
      unsigned int i3 = mT[2];

      const float *vertices = Vl_getVertices(vlook);

#if SHOW_DEBUG
      debugSphere(debug,vertices,j1,0xFF0000);
      debugSphere(debug,vertices,j2,0x00FF00);
      debugSphere(debug,vertices,j3,0x0000FF);

      debugSphere(debug,vertices,i1,0xFF0000);
      debugSphere(debug,vertices,i2,0x00FF00);
      debugSphere(debug,vertices,i3,0x0000FF);
      debugMe(debug,vertices,j2,j3,i3);
      debugMe(debug,vertices,j2,i3,i2);
      debugMe(debug,vertices,j3,j1,i1);
      debugMe(debug,vertices,j3,i1,i3);
      debugMe(debug,vertices,j1,j2,i2);
      debugMe(debug,vertices,j1,i2,i1);
#endif

      // j2,j3,i3
      // j2,i3,i2
      indices.push_back(j2);
      indices.push_back(j3);
      indices.push_back(i3);
      indices.push_back(j2);
      indices.push_back(i3);
      indices.push_back(i2);

      // j3,j1,i1
      // j3,i1,i3
      indices.push_back(j3);
      indices.push_back(j1);
      indices.push_back(i1);
      indices.push_back(j3);
      indices.push_back(i1);
      indices.push_back(i3);

      // j1,j2,i2
      // j1,i2,i1
      indices.push_back(j1);
      indices.push_back(j2);
      indices.push_back(i2);
      indices.push_back(j1);
      indices.push_back(i2);
      indices.push_back(i1);

    }
  }


  bool         mProjected;

  unsigned int mI[3];          // original indices
  unsigned int mT[3];         // projected final indices

  unsigned int mMatch[3];

  bool         mDoubleSided;
  float        mPlane[4];
  int          mJoined;        // the triangle we are 'joined' to.
};


#if 1

#define SMINX -1000
#define SMAXX  1000
#define SMINY -1000
#define SMAXY  1000
#define SMINZ -1000
#define SMAXZ  1000

#endif

static bool skipPoint(const float *p)
{
  bool skip = true;

  if ( p[0] >= SMINX && p[0] <= SMAXX &&
       p[1] >= SMINY && p[1] <= SMAXY &&
       p[2] >= SMINZ && p[2] <= SMAXZ )
  {
    skip = false;
  }
  return skip;
}

static bool skipTriangle(const float *p1,const float *p2,const float *p3)
{
  bool skip = false;

//  if ( skipPoint(p1) && skipPoint(p2) && skipPoint(p3) ) skip = true;      Used for debugging purposes only

  return skip;
}




bool meshCleanup(MeshCleanupDesc &desc,float weldDistance,float projectDistance,RENDER_DEBUG::RenderDebug *debug)
{
  bool ret = false;

  float *vertices = new float[desc.inputVcount*3];
  const char *scan = (const char *) desc.inputVertices;
  float *dest = vertices;


  for (unsigned int i=0; i<desc.inputVcount; i++)
  {
    const float *source = (const float *) scan;
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = source[2];
    dest+=3;
    scan+=desc.inputVstride;
  }

  const char *indices = (const char *)desc.inputIndices;

  std::vector< QuickTri > triangles;

  IntInt< UINT64, unsigned int > edgeList;

  for (unsigned int i=0; i<desc.inputTcount; i++)
  {
    const char *base = indices+i*desc.inputTstride;
    const unsigned int *idx = (const unsigned int *)base;

    unsigned int i1 = idx[0];
    unsigned int i2 = idx[1];
    unsigned int i3 = idx[2];

    const float *p1 = &vertices[i1*3];
    const float *p2 = &vertices[i2*3];
    const float *p3 = &vertices[i3*3];

    if ( skipTriangle(p1,p2,p3) || i1 == i2 || i1 == i3 || i2 == i3 ) // skip degenerate triangles
    {
      desc.degenerateCount++;
#if SHOW_DEBUG
      if ( debug )
      {
        debug->DebugTri(p1,p2,p3,0xFF0000,6000.0f);
        debug->DebugSphere(p1,0.02f,0xFFFF00,6000.0f);
        debug->DebugSphere(p2,0.03f,0xFFFF00,6000.0f);
        debug->DebugSphere(p3,0.04f,0xFFFF00,6000.0f);
      }
#endif
    }
    else
    {
      QuickTri t;

      t.mI[0] = i1;
      t.mI[1] = i2;
      t.mI[2] = i3;

      t.computePlane(vertices);

      if ( t.identical(edgeList) )
      {
        desc.duplicateCount++;
      }
      else
      {
        unsigned int dindex = t.isDoubleSided(edgeList);
        if ( dindex )
        {
          QuickTri &qt = triangles[dindex-1];
          t.mJoined = dindex-1;
          qt.mDoubleSided = true;
          t.mDoubleSided  = true;
        }
        UINT64 hash = t.getHash();
        unsigned int index = triangles.size()+1;
        edgeList.set(index,hash);
        triangles.push_back(t);
      }
    }
  }


  if ( !triangles.empty() )
  {
    ret = true;


    VertexLookup vlook = Vl_createVertexLookup(weldDistance);

    std::vector< unsigned int > indices;

    std::vector< QuickTri >::iterator i;
    for (i=triangles.begin(); i!=triangles.end(); ++i)
    {

      QuickTri &t = (*i);

      if ( t.mDoubleSided )
      {
        desc.doubleSidedCount++;
      }

      QuickTri *join=0;
      if ( t.mJoined >= 0 )
      {
        join = &triangles[t.mJoined];
      }

      t.performProject(vlook,vertices,join,indices,projectDistance,debug);
    }

    unsigned int icount = indices.size();
    desc.outputIndices = new unsigned int[icount];
    memcpy(desc.outputIndices,&indices[0],sizeof(unsigned int)*icount);
    desc.outputTcount = indices.size()/3;


    desc.outputVcount   = Vl_getVcount(vlook);
    desc.outputVertices = new float[desc.outputVcount*3];
    memcpy(desc.outputVertices,Vl_getVertices(vlook),sizeof(float)*desc.outputVcount*3);

    Vl_releaseVertexLookup(vlook);

  }

  delete vertices;

  return ret;
}


void releaseMeshCleanup(MeshCleanupDesc &desc)
{
  delete desc.outputVertices;
  delete desc.outputIndices;
  desc.outputVertices = 0;
  desc.outputIndices  = 0;
  desc.outputVcount   = 0;
  desc.outputTcount   = 0;
}

}; // end of namespace

