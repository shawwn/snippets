
inline float det(const float *p1,const float *p2,const float *p3)
{
  return  p1[0]*p2[1]*p3[2] + p2[0]*p3[1]*p1[2] + p3[0]*p1[1]*p2[2] -p1[0]*p3[1]*p2[2] - p2[0]*p1[1]*p3[2] - p3[0]*p2[1]*p1[2];
}

float computeMeshVolume(const float *vertices,unsigned int tcount,unsigned int *indices)
{
	float volume = 0;

	for (unsigned int i=0; i<tcount; i++,indices+=3)
	{

		const float *p1 = &vertices[ indices[0]*3 ];
		const float *p2 = &vertices[ indices[1]*3 ];
		const float *p3 = &vertices[ indices[2]*3 ];

		volume+=det(p1,p2,p3); // compute the volume of the tetrahedran relative to the origin.
	}

  volume*=(1.0f / 6.0f );
	if ( volume < 0 ) volume*=-1;
	return volume;
}

