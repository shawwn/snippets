#ifndef PHYSX2OBJ_H

#define PHYSX2OBJ_H

// This code snippet will take the contents of a PhysX SDK scene and save them either
// as a single large Wavefront OBJ file in world space or as a series of wavefront OBJ
// files in Object space with a response file containing the instantiation data for
// individual data assets.
//
// Works with the PhysX SDK 2.8.1 and PhysX 2.8.3
//
// Written by John W. Ratcliff on January 25, 2010
//
// This is useful for getting a 'game level' exported for testing in other applications.
//
// Note, that this does not handle primitive data types like boxes, spheres, and capsules.
// It only exports triangle meshes and convex hulls.

class NxScene;

// scene : The PhysX SDK scene to export to the Wavefront OBj file(s)
// fname : The base file name to save the scene as (does not include the extension of .obj) that will be appended.
// worldSpace : Whether to save the as a single large wavefront file in worldspace or a single large file in object space, with instancing tags
int exportPhysX2Obj(NxScene *scene,const char *fname,bool worldSpace);

#endif
