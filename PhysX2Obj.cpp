#include "PhysX2Obj.h"

#define USE_PHYSX 1

#if USE_PHYSX

#include <NxScene.h>
#include <stdio.h>
#include <vector>
#include <assert.h>

#include "NxScene.h"
#include "NxSphere.h"
#include "NxShape.h"
#include "NxBoxShape.h"
#include "NxPlaneShape.h"
#include "NxBoxShapeDesc.h"
#include "NxSphereShapeDesc.h"
#include "NxCapsuleShapeDesc.h"
#include "NxTriangleMeshShapeDesc.h"
#include "NxHeightField.h"
#include "NxHeightFieldDesc.h"
#include "NxHeightFieldShape.h"
#include "NxHeightFieldShapeDesc.h"
#include "NxConvexShapeDesc.h"
#include "NxTriangleMeshDesc.h"
#include "NxConvexMeshDesc.h"
#include "NxActor.h"
#include "NxSphereShape.h"
#include "NxCapsuleShape.h"
#include "NxCapsule.h"

#pragma warning(disable:4996)


namespace PHYSX2OBJ
{

//*******************************************
//*** Convert PhysX heightfields into triangle meshes.
//***********************************************

class PhysXHeightFieldInterface
{
public:
  virtual void receiveHeightFieldMesh(NxU32 vcount,const NxF32 *vertices,NxU32 tcount,const NxU32 *indices) = 0;
};

#define HF_STEP_SIZE 32    // the maximum row/column size for a sub-mesh.  Must be a #define because memory is allocated on the stack for this operation.  This routine is entirely thread safe!

struct Hsample
{
  short  height         : 16;
  NxU8   materialIndex0 :  7;
  NxU8   tessFlag       :  1;
  NxU8   materialIndex1 :  7;
  NxU8   unused         :  1;
};

static  inline NxU32 getIndex(NxI32 dx,NxI32 dy,NxI32 x,NxI32 y,NxI32 wid,NxI32 hit)
{
  x+=dx;
  y+=dy;
  if ( x < 0 ) x = 0;
  if ( x >= wid ) x = wid-1;
  if ( y < 0 ) y = 0;
  if ( y >= hit ) y =hit-1;
  return (NxU32)((y*wid)+x);
}


static inline void getV(NxI32 x,
						NxI32 z,
						NxI32 columns,
						NxI32 rows,
						NxF32 columnScale,
						NxF32 rowScale,
						NxF32 heightScale,
						Hsample *scan,
						NxF32 *vertices,
						NxU32 &vcount,
						const NxMat34 &pose)
{

	NxVec3 pos;

#if 1
	pos.x = (NxF32) z * rowScale;
	pos.z = (NxF32) x * columnScale;
#else
  	pos.x = (NxF32) x * columnScale;
  	pos.z = (NxF32) z * rowScale;
#endif
  	NxU32 index = getIndex(0,0,x,z,columns,rows);

  	pos.y = (NxF32) scan[index].height * heightScale;

	NxVec3 world;

	pose.multiply(pos,world);

	NxF32 *v = &vertices[vcount*3];

	v[0] = world.x;
	v[1] = world.y;
	v[2] = world.z;

  	vcount++;
}

static inline void pushTri(NxU32 i1,NxU32 i2,NxU32 i3,NxU32 *indices,NxU32 &tcount)
{
  NxU32 *dest = &indices[tcount*3];

  dest[0] = i1;
  dest[1] = i2;
  dest[2] = i3;

  tcount++;
}

void physXHeightFieldToMesh(NxI32 wid,                           // the width of the source heightfield
                            NxI32 depth,                           // the height of the source heightfield
                            const NxU32 *heightField,            // the raw 32 bit heightfield data.
                            NxF32 verticalExtent,                // the vertical extent of the heightfield
                            NxF32 columnScale,                   // the column scale
                            NxF32 rowScale,                      // the row scale
                            NxF32 heightScale,                   // the heightscale
                            const NxMat34 &pose,                 // object to world transform
							NxU32 holeMaterial,
                            PhysXHeightFieldInterface *callback)// your interface to receive the mesh data an an indexed triangle mesh.
{
  NxI32 stepx = (wid+(HF_STEP_SIZE-1))/HF_STEP_SIZE;
  NxI32 stepz = (depth+(HF_STEP_SIZE-1))/HF_STEP_SIZE;

  Hsample *scan = (Hsample *)heightField;

  bool reverse = false;

  if ( verticalExtent > 0 )
    reverse = true;

  #define MAX_TRIANGLES ((HF_STEP_SIZE+1)*(HF_STEP_SIZE+1)*2)
  #define MAX_VERTICES ((HF_STEP_SIZE+1)*(HF_STEP_SIZE+1))

  for (NxI32 iz=0; iz<stepz; iz++)
  {
    for (NxI32 ix=0; ix<stepx; ix++)
    {
      // ok..first build the points....
      NxI32 swid = wid-(ix*HF_STEP_SIZE);
      NxI32 sdepth = depth-(iz*HF_STEP_SIZE);

      if ( swid > HF_STEP_SIZE ) swid = HF_STEP_SIZE;
      if ( sdepth > HF_STEP_SIZE ) sdepth = HF_STEP_SIZE;

      NxU32 vcount = 0;
      NxU32 tcount = 0;

      NxF32 vertices[MAX_VERTICES*3];
      NxU32 indices[MAX_TRIANGLES*3];

      for (NxI32 rz=0; rz<=sdepth; rz++)
      {
        for (NxI32 rx=0; rx<=swid; rx++)
        {
          NxI32 x = rx+(ix*HF_STEP_SIZE);
          NxI32 z = rz+(iz*HF_STEP_SIZE);
          getV(x,z,wid,depth,columnScale,rowScale,heightScale,scan,vertices,vcount,pose);
        }
      }

      for (NxI32 rz=0; rz<sdepth; rz++)
      {
        for (NxI32 rx=0; rx<swid; rx++)
        {
          NxI32 x = rx+(ix*HF_STEP_SIZE);
          NxI32 z = rz+(iz*HF_STEP_SIZE);

          if ( x < (wid-1) && z < (depth-1) )
          {
            NxU32 index = (z*wid)+x;
            const Hsample *sample = (const Hsample *) &heightField[index];
            bool ok1 = true;
            bool ok2 = true;

            if ( sample->materialIndex0 == holeMaterial || sample->materialIndex1 == holeMaterial )
            {
              if ( sample->materialIndex0 == holeMaterial && sample->materialIndex1 == holeMaterial )
                continue;
              if ( sample->materialIndex0 == holeMaterial )
                ok2 = false;
              else
                ok1 = false;
            }

            bool flip = reverse;

            NxU32 i1,i2,i3,i4;

            if ( sample->tessFlag )
            {
              i1 = getIndex(0,0,rx,rz,swid+1,sdepth+1);
              i2 = getIndex(1,0,rx,rz,swid+1,sdepth+1);
              i3 = getIndex(1,1,rx,rz,swid+1,sdepth+1);
              i4 = getIndex(0,1,rx,rz,swid+1,sdepth+1);
            }
            else
            {
              if ( flip )
                flip = false;
              else
                flip = true;
              i1 = getIndex(0,1,rx,rz,swid+1,sdepth+1);
              i2 = getIndex(1,1,rx,rz,swid+1,sdepth+1);
              i3 = getIndex(1,0,rx,rz,swid+1,sdepth+1);
              i4 = getIndex(0,0,rx,rz,swid+1,sdepth+1);
            }

            if ( flip )
            {
              if ( ok1 ) pushTri(i3,i2,i1,indices,tcount);
              if ( ok2 ) pushTri(i4,i3,i1,indices,tcount);
            }
            else
            {
              if ( ok1 ) pushTri(i1,i2,i3,indices,tcount);
              if ( ok2 ) pushTri(i1,i3,i4,indices,tcount);
            }
          }
        }
      }

      if ( tcount )
      {
        callback->receiveHeightFieldMesh(vcount,vertices,tcount,indices);
      }

    }
  }
}

struct Mesh
{
	Mesh(void *m,NxU32 sf,NxU32 ef)
	{
		mMesh = m;
		mStartFace = sf;
		mEndFace = ef;
	}

	bool match(void *m,NxU32 &sf,NxU32 &ef) const
	{
		bool ret = false;

		if ( m == mMesh )
		{
			sf = mStartFace;
			ef = mEndFace;
			ret = true;
		}
		return ret;
	}

	void	*mMesh;
  	NxU32  mStartFace;
  	NxU32	 mEndFace;
};

class ExportObj : public PhysXHeightFieldInterface
{
public:
	ExportObj(NxScene *scene,const char * fname,bool worldSpace)
	{
		mWorldSpace = worldSpace;
		mObjectCount = 0;
		mFph = fopen(fname,"wb");
		if ( mFph )
		{
			fprintf(mFph,"# PhysX2Obj exported file.\r\n" );
    		if ( scene )
    		{
    			NxU32 acount = scene->getNbActors();
    			if ( acount )
    			{
    				NxActor **alist = scene->getActors();
    				for (NxU32 i=0; i<acount; i++)
    				{
    					NxActor *a = alist[i];
    					exportActor(a);
    				}
    			}
    		}

    		if ( !mIndices.empty() )
    		{
          		const NxF32 *vertices = &mPoints[0].x;
          		NxU32 vcount = (NxU32)mPoints.size();
          		fprintf(mFph,"# %d vertices\r\n", vcount );
          		for (NxU32 i=0; i<vcount; i++)
          		{
          			const NxF32 *pos = &vertices[i*3];
          			fprintf(mFph,"v %0.9f %0.9f %0.9f\r\n", pos[0], pos[1], pos[2] );
          		}
          		NxU32 tcount = (NxU32)mIndices.size()/3;
          		fprintf(mFph,"# %d triangles.\r\n", tcount );
          		for (NxU32 i=0; i<tcount; i++)
          		{
          			NxU32 i1 = mIndices[i*3+0];
          			NxU32 i2 = mIndices[i*3+1];
          			NxU32 i3 = mIndices[i*3+2];
          			fprintf(mFph,"f %d %d %d\r\n", i1+1, i2+1, i3+1 );
          		}
    		}
    		fclose(mFph);
    		mFph = 0;
    	}
	}

	~ExportObj(void)
	{
	}

	void exportActor(NxActor *a)
	{
		NxU32 nbShapes = a->getNbShapes();
		NxShape *const*shapes = a->getShapes();
		for (NxU32 i=0; i<nbShapes; i++)
		{
			NxShape *s = shapes[i];
			exportShape(s);
		}
	}

	void exportShape(NxShape *shape)
    {
    	mObjectCount++;
    	NxActor &actor = shape->getActor();
      	switch ( shape->getType() )
      	{
        	case NX_SHAPE_HEIGHTFIELD:
          	{
				NxHeightFieldShape *hmesh = shape->isHeightField();
              	NxHeightFieldShapeDesc desc;
              	hmesh->saveToDesc(desc);
              	NxHeightField *heightfield = desc.heightField;

  				NxHeightFieldDesc hdesc;
  				heightfield->saveToDesc(hdesc);

             	NxU32	size = heightfield->getNbRows()	*heightfield->getNbColumns() *heightfield->getSampleStride();

         		if ( size )
         		{
                	NxMat34 pose = shape->getGlobalPose();

                	NxU32 startFace=0;
					NxU32 endFace=0;
					bool found = false;

					if ( !mWorldSpace ) // if not saving in worldspace, set the pose to identity, and see if this mesh has already been exported
					{
						pose.id();
						found = findMesh(heightfield,startFace,endFace);
					}

					if ( !found ) // if the mesh has not already been exported, save it.
					{
						NxU8 *tempSamples	=	new NxU8[size];
						heightfield->saveCells(tempSamples,	size);

						NxI32 col    = (NxI32)heightfield->getNbColumns();
						NxI32 row    = (NxI32)heightfield->getNbRows();

						startFace = (NxU32)mIndices.size() / 3;

						physXHeightFieldToMesh(col,
						               			row,
						               			(const NxU32 *)tempSamples,
						               			hdesc.thickness,
						               			desc.columnScale,
						               			desc.rowScale,
						               			desc.heightScale,
						               			pose,
						               			desc.holeMaterial,
						               			this );

                		delete []tempSamples;
       					endFace = (NxU32) mIndices.size() / 3;
						if ( !mWorldSpace )
						{
       						saveMesh(heightfield,startFace,endFace);
       					}
       				}

       				if ( !mWorldSpace )
       				{
                    	NxMat34 pose = shape->getGlobalPose();
                    	NxF32 matrix[16];
                    	pose.getColumnMajor44(matrix);
             			fprintf(mFph,"#mesh startTri(%d) triCount(%d) transform(%0.9f,%0.9f,%0.9f,%0.9f,  %0.9f,%0.9f,%0.9f,%0.9f,  %0.9f,%0.9f,%0.9f,%0.9f, %0.9f,%0.9f,%0.9f,%0.9f)\r\n",
             			    startFace,endFace-startFace,
             			  matrix[0],matrix[1],matrix[2],matrix[3],
             			  matrix[4],matrix[5],matrix[6],matrix[7],
             			  matrix[8],matrix[9],matrix[10],matrix[11],
             			  matrix[12],matrix[13],matrix[14],matrix[15] );
       				}
              	}
          	}
			break;
   	  		case NX_SHAPE_MESH:
    		{
				NxTriangleMeshShape *tmesh = shape->isTriangleMesh();
      			NxTriangleMeshShapeDesc desc;
      			tmesh->saveToDesc(desc);
      			NxTriangleMesh *meshData = desc.meshData;
      			if ( meshData )
      			{
       				NxTriangleMeshDesc desc;
       				meshData->saveToDesc(desc);
                	NxMat34 pose = shape->getGlobalPose();
                	NxU32 startFace=0;
					NxU32 endFace=0;
					bool found = false;
					if ( !mWorldSpace ) // if not saving in worldspace, set the pose to identity, and see if this mesh has already been exported
					{
						pose.id();
						found = findMesh(meshData,startFace,endFace);
					}

					if ( !found ) // if the mesh has not already been exported, save it.
					{
						startFace = (NxU32)mIndices.size() / 3;

       					const char *pscan = (const char *) desc.points;

       					NxU32 baseIndex = (NxU32)mPoints.size();

                    	for (NxU32 i=0; i<desc.numVertices; i++)
                    	{
                      		const NxF32 *pos = (const NxF32 *) pscan;
                      		NxVec3 v(pos);
                      		NxVec3 t;
                      		pose.multiply(v,t);
                      		mPoints.push_back(t);
                      		pscan+=desc.pointStrideBytes;
                    	}

      			  		const char *scan = (const char *) desc.triangles;

           		  		for (NxU32 i=0; i<desc.numTriangles; i++)
           		  		{
           					if ( desc.flags & NX_CF_16_BIT_INDICES )
           					{
           						const NxU16 *source = (const NxU16 *) scan;
           						addTriangle(source[0],source[1],source[2],baseIndex);
       						}
       						else
       						{
       							const NxU32 *source = (const NxU32 *) scan;
       							addTriangle(source[0],source[1],source[2],baseIndex);
       						}
       						scan+=desc.triangleStrideBytes;
       					}
       					endFace = (NxU32) mIndices.size() / 3;
						if ( !mWorldSpace )
       						saveMesh(meshData,startFace,endFace);
       				}
       				if ( !mWorldSpace )
       				{
                    	NxMat34 pose = shape->getGlobalPose();
                    	NxF32 matrix[16];
                    	pose.getColumnMajor44(matrix);
             			fprintf(mFph,"#mesh startTri(%d) triCount(%d) transform(%0.9f,%0.9f,%0.9f,%0.9f,  %0.9f,%0.9f,%0.9f,%0.9f,  %0.9f,%0.9f,%0.9f,%0.9f, %0.9f,%0.9f,%0.9f,%0.9f)\r\n",
             			    startFace,endFace-startFace,
             			  matrix[0],matrix[1],matrix[2],matrix[3],
             			  matrix[4],matrix[5],matrix[6],matrix[7],
             			  matrix[8],matrix[9],matrix[10],matrix[11],
             			  matrix[12],matrix[13],matrix[14],matrix[15] );
       				}
              	}
            }
   			break;
			case NX_SHAPE_CONVEX:
    		{
				NxConvexShape *convex = shape->isConvexMesh();
      			NxConvexShapeDesc desc;
      			convex->saveToDesc(desc);
      			NxConvexMesh *meshData = desc.meshData;
   				if ( meshData )
   				{
					NxConvexMeshDesc desc;
       				meshData->saveToDesc(desc);

                	NxMat34 pose = shape->getGlobalPose();
                	NxU32 startFace=0;
					NxU32 endFace=0;

					bool found = false;

					if ( !mWorldSpace || actor.isDynamic() )
					{
						pose.id();
						found = findMesh(meshData,startFace,endFace);
					}

					if ( !found )
					{
						NxU32 baseIndex = (NxU32)mPoints.size();
           				const char *pscan = (const char *) desc.points;
                    	for (NxU32 i=0; i<desc.numVertices; i++)
                    	{
                      		const NxF32 *pos = (const NxF32 *) pscan;
                      		NxVec3 v(pos);
                      		NxVec3 t;
                      		pose.multiply(v,t);
                      		mPoints.push_back(t);
                      		pscan+=desc.pointStrideBytes;
                    	}


       					const char *scan = (const char *) desc.triangles;

    					startFace = (NxU32)mIndices.size() / 3;

       					for (NxU32 i=0; i<desc.numTriangles; i++)
       					{
    						if ( desc.flags & NX_CF_16_BIT_INDICES )
           					{
           						const NxU16 *source = (const NxU16 *) scan;
           						addTriangle(source[0],source[1],source[2],baseIndex);
       						}
       						else
       						{
       							const NxU32 *source = (const NxU32 *) scan;
       							addTriangle(source[0],source[1],source[2],baseIndex);
       						}
       						scan+=desc.triangleStrideBytes;
       					}
       					endFace = (NxU32)mIndices.size() / 3;
						if ( !mWorldSpace || actor.isDynamic() )
       						saveMesh(meshData,startFace,endFace);
       				}
   					if ( !mWorldSpace || actor.isDynamic() )
   					{
                    	NxMat34 pose = shape->getGlobalPose();
                    	NxF32 matrix[16];
                    	pose.getColumnMajor44(matrix);
             			fprintf(mFph,"#convex dynamic(%s) startTri(%d) triCount(%d) transform(%0.9f,%0.9f,%0.9f,%0.9f,  %0.9f,%0.9f,%0.9f,%0.9f,  %0.9f,%0.9f,%0.9f,%0.9f, %0.9f,%0.9f,%0.9f,%0.9f)\r\n",
      				  		actor.isDynamic() ? "true" : "false",
             			    startFace,endFace-startFace,
             			  matrix[0],matrix[1],matrix[2],matrix[3],
             			  matrix[4],matrix[5],matrix[6],matrix[7],
             			  matrix[8],matrix[9],matrix[10],matrix[11],
             			  matrix[12],matrix[13],matrix[14],matrix[15] );
   					}
   				}
            }
   			break;
   			case NX_SHAPE_SPHERE:
   			{
  		  		NxSphereShape *bs = shape->isSphere();
            	NxF32 radius = bs->getRadius();
            	NxMat34 pose = shape->getGlobalPose();
            	NxF32 matrix[16];
            	pose.getColumnMajor44(matrix);
       			fprintf(mFph,"#sphere dynamic(%s) radius(%0.9f) transform(%0.9f,%0.9f,%0.9f,%0.9f,  %0.9f,%0.9f,%0.9f,%0.9f,  %0.9f,%0.9f,%0.9f,%0.9f, %0.9f,%0.9f,%0.9f,%0.9f)\r\n",
				  actor.isDynamic() ? "true" : "false",
       			  radius,
       			  matrix[0],matrix[1],matrix[2],matrix[3],
       			  matrix[4],matrix[5],matrix[6],matrix[7],
       			  matrix[8],matrix[9],matrix[10],matrix[11],
       			  matrix[12],matrix[13],matrix[14],matrix[15] );
   			}
   			break;
   			case NX_SHAPE_CAPSULE:
   			{
   				NxCapsuleShape *bs = shape->isCapsule();
            	NxF32 height = bs->getHeight();
            	NxF32 radius = bs->getRadius();
            	NxMat34 pose = shape->getGlobalPose();
            	NxF32 matrix[16];
            	pose.getColumnMajor44(matrix);
       			fprintf(mFph,"#capsule dynamic(%s) height(%0.9f) radius(%0.9f) transform(%0.9f,%0.9f,%0.9f,%0.9f,  %0.9f,%0.9f,%0.9f,%0.9f,  %0.9f,%0.9f,%0.9f,%0.9f, %0.9f,%0.9f,%0.9f,%0.9f)\r\n",
				  actor.isDynamic() ? "true" : "false",
       			  height,radius,
       			  matrix[0],matrix[1],matrix[2],matrix[3],
       			  matrix[4],matrix[5],matrix[6],matrix[7],
       			  matrix[8],matrix[9],matrix[10],matrix[11],
       			  matrix[12],matrix[13],matrix[14],matrix[15] );
   			}
   			break;
   			case NX_SHAPE_BOX:
   			{
   				NxBoxShape *bs = shape->isBox();
				NxVec3 sides =  bs->getDimensions();
       			NxMat34 pose = shape->getGlobalPose();
       			NxF32 matrix[16];
       			pose.getColumnMajor44(matrix);
       			fprintf(mFph,"#box dynamic(%s) dimensions(%0.9f,%0.9f,%0.9f) transform(%0.9f,%0.9f,%0.9f,%0.9f,  %0.9f,%0.9f,%0.9f,%0.9f,  %0.9f,%0.9f,%0.9f,%0.9f, %0.9f,%0.9f,%0.9f,%0.9f)\r\n",
				  actor.isDynamic() ? "true" : "false",
       			  sides.x,sides.y,sides.z,
       			  matrix[0],matrix[1],matrix[2],matrix[3],
       			  matrix[4],matrix[5],matrix[6],matrix[7],
       			  matrix[8],matrix[9],matrix[10],matrix[11],
       			  matrix[12],matrix[13],matrix[14],matrix[15] );
   			}
   			break;
   			case NX_SHAPE_PLANE:
   			{
   				NxPlaneShape *ps = shape->isPlane();
       			NxMat34 pose = shape->getGlobalPose();
       			NxF32 matrix[16];
       			pose.getColumnMajor44(matrix);
       			NxPlane p = ps->getPlane();
       			fprintf(mFph,"#plane equation(%0.9f,%0.9f,%0.9f,%0.9f) transform(%0.9f,%0.9f,%0.9f,%0.9f,  %0.9f,%0.9f,%0.9f,%0.9f,  %0.9f,%0.9f,%0.9f,%0.9f, %0.9f,%0.9f,%0.9f,%0.9f)\r\n",
				  p.normal.x,p.normal.y,p.normal.z,p.d,
       			  matrix[0],matrix[1],matrix[2],matrix[3],
       			  matrix[4],matrix[5],matrix[6],matrix[7],
       			  matrix[8],matrix[9],matrix[10],matrix[11],
       			  matrix[12],matrix[13],matrix[14],matrix[15] );
   			}
   			break;
   			case NX_SHAPE_WHEEL:
   				fprintf(mFph,"#wheel\r\n");
   			break;
   			default:
          	assert(0);
    		break;
    	}
	}

	void addTriangle(NxU32 i1,NxU32 i2,NxU32 i3,NxU32 baseIndex)
	{
		mIndices.push_back(i1+baseIndex);
		mIndices.push_back(i2+baseIndex);
		mIndices.push_back(i3+baseIndex);
	}

	int getObjectCount(void) const
	{
		return mObjectCount;
	}


  	virtual void receiveHeightFieldMesh(NxU32 vcount,const NxF32 *vertices,NxU32 tcount,const NxU32 *indices)
  	{
  		NxU32 baseIndex = (NxU32)mPoints.size();

  		for (NxU32 i=0; i<vcount; i++)
  		{
  			NxVec3 v( &vertices[i*3] );
  			mPoints.push_back(v);
  		}

  		for (NxU32 i=0; i<tcount; i++)
  		{
  			NxU32 i1 = indices[i*3+0];
  			NxU32 i2 = indices[i*3+1];
  			NxU32 i3 = indices[i*3+2];
  			addTriangle(i3,i2,i1,baseIndex); // flip the winding order for heightfield triangles
  		}
  	}

	bool findMesh(void *mesh,NxU32 &startFace,NxU32 &endFace)
	{
	    bool ret = false;
		for (std::vector< Mesh >::iterator i=mMeshes.begin(); i!=mMeshes.end(); ++i)
		{
			if ( (*i).match(mesh,startFace,endFace) )
			{
				ret = true;
				break;
			}
		}

		return ret;
	}

    void saveMesh(void *mesh,NxU32 startFace,NxU32 endFace)
    {
    	Mesh m(mesh,startFace,endFace);
    	mMeshes.push_back(m);
    }

	int				mObjectCount;
	std::vector< NxU32 > mIndices;
	std::vector< Mesh >  mMeshes;
	bool			 mWorldSpace;
	std::vector< NxVec3 > mPoints;

	FILE			*mFph;
};

}; // end of namespace

#endif

using namespace PHYSX2OBJ;

int exportPhysX2Obj(NxScene *scene,const char *fname,bool worldSpace)
{
#if USE_PHYSX
	ExportObj o(scene,fname,worldSpace);
	return o.getObjectCount();
#else
	return 0;
#endif
}
