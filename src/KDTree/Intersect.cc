
inline void cross(float u[], const float v[], const float w[])
{
	u[0] = v[1]*w[2] - v[2]*w[1];
	u[1] = v[2]*w[0] - v[0]*w[2];
	u[2] = v[0]*w[1] - v[1]*w[0];
}

inline void sub(float u[], const float v[], const float w[])
{
	u[0] = v[0] - w[0];
	u[1] = v[1] - w[1];
	u[2] = v[2] - w[2];
}

inline float dot(const float v[], const float w[])
{
	return v[0]*w[0] + v[1]*w[1] + v[2]*w[2];
}

bool IntersectTriangle( const float orig[], const float dir[], //ray
					   float v0[], float v1[], float v2[], //triangle
					   float* t, //Ray-intersection parameter distance.
					   float* u, //Barycentric hit coordinates, U.
					   float* v )//Barycentric hit coordinates, V.
					   //--------------------------------------------------------------------------------------
					   // Given a ray origin (orig) and direction (dir), and three vertices of a triangle, this
					   // function returns true and the interpolated texture coordinates if the ray intersects 
					   // the triangle
					   //--------------------------------------------------------------------------------------
{
	// Find vectors for two edges sharing vert0
	float edge1[3];
	edge1[0] = v1[0] - v0[0];
	edge1[1] = v1[1] - v0[1];
	edge1[2] = v1[2] - v0[2];

	float edge2[3];
	edge2[0] = v2[0] - v0[0];
	edge2[1] = v2[1] - v0[1];
	edge2[2] = v2[2] - v0[2];

	// Begin calculating determinant - also used to calculate U parameter
	float pvec[3];
	cross(pvec, dir, edge2);

	// If determinant is near zero, ray lies in plane of triangle
	float det = dot(edge1, pvec);

	float tvec[3];
	if( det > 0 )
	{
		sub(tvec, orig, v0);
	}
	else
	{
		sub(tvec, v0, orig);
		det = -det;
	}

	if( det < 0.0001f )
		return false;

	// Calculate U parameter and test bounds
	*u = dot( tvec, pvec );
	if( *u < 0.0f || *u > det )
		return false;

	// Prepare to test V parameter
	float qvec[3];
	cross(qvec, tvec , edge1);

	// Calculate V parameter and test bounds
	*v = dot( dir, qvec );
	if( *v < 0.0f || *u + *v > det )
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	*t = dot( edge2, qvec );
	float fInvDet = 1.0f / det;
	*t *= fInvDet;
	*u *= fInvDet;
	*v *= fInvDet;

	if ( *t < 0 )
		return false;

	return true;
}