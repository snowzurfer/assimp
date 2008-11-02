/*
Open Asset Import Library (ASSIMP)
----------------------------------------------------------------------

Copyright (c) 2006-2008, ASSIMP Development Team
All rights reserved.

Redistribution and use of this software in source and binary forms, 
with or without modification, are permitted provided that the 
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the ASSIMP team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the ASSIMP Development Team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/

/** @file Implementation of the StandardShapes class
 */

#include "AssimpPCH.h"
#include "StandardShapes.h"

namespace Assimp	{

	// note - flip the face order
#define ADD_TRIANGLE(n0,n1,n2) \
	positions.push_back(n2); \
	positions.push_back(n1); \
	positions.push_back(n0);

#	define ADD_PENTAGON(n0,n1,n2,n3,n4) \
	if (polygons) \
	{ \
		positions.push_back(n0); \
		positions.push_back(n1); \
		positions.push_back(n2); \
		positions.push_back(n3); \
		positions.push_back(n4); \
	} \
	else \
	{ \
		ADD_TRIANGLE(n0, n1, n2) \
		ADD_TRIANGLE(n0, n2, n3) \
		ADD_TRIANGLE(n0, n3, n4) \
	}

#	define ADD_QUAD(n0,n1,n2,n3) \
	if (polygons) \
	{ \
		positions.push_back(n0); \
		positions.push_back(n1); \
		positions.push_back(n2); \
		positions.push_back(n3); \
	} \
	else \
	{ \
		ADD_TRIANGLE(n0, n1, n2) \
		ADD_TRIANGLE(n0, n2, n3) \
	}


// ------------------------------------------------------------------------------------------------
void Subdivide(std::vector<aiVector3D>& positions)
{
	// assume this to be constant - input must be a Platonic primitive!
	const float fl1 = positions[0].Length();

	unsigned int origSize = (unsigned int)positions.size();
	for (unsigned int i = 0 ; i < origSize ; i+=3)
	{
		aiVector3D& tv0 = positions[i];
		aiVector3D& tv1 = positions[i+1];
		aiVector3D& tv2 = positions[i+2];

		aiVector3D a = tv0, b = tv1, c = tv2;
		aiVector3D v1 = aiVector3D(a.x+b.x, a.y+b.y, a.z+b.z).Normalize()*fl1;
		aiVector3D v2 = aiVector3D(a.x+c.x, a.y+c.y, a.z+c.z).Normalize()*fl1;
		aiVector3D v3 = aiVector3D(b.x+c.x, b.y+c.y, b.z+c.z).Normalize()*fl1;

		tv0 = v1; tv1 = v3; tv2 = v2; // overwrite the original
		ADD_TRIANGLE(v2, v1, a);
		ADD_TRIANGLE(v3, v2, c);
		ADD_TRIANGLE(v1, v3, b);
	}
}

// ------------------------------------------------------------------------------------------------
aiMesh* StandardShapes::MakeMesh(const std::vector<aiVector3D>& positions,
	unsigned int numIndices)
{
	if (positions.size() & numIndices || positions.empty() || !numIndices)return NULL;

	aiMesh* out = new aiMesh();
	switch (numIndices)
	{
	case 1:
		out->mPrimitiveTypes = aiPrimitiveType_POINT;
		break;
	case 2:
		out->mPrimitiveTypes = aiPrimitiveType_LINE;
		break;
	case 3:
		out->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
		break;
	default:
		out->mPrimitiveTypes = aiPrimitiveType_POLYGON;
		break;
	};

	out->mNumFaces = (unsigned int)positions.size() / numIndices;
	out->mFaces = new aiFace[out->mNumFaces];
	for (unsigned int i = 0, a = 0; i < out->mNumFaces;++i)
	{
		aiFace& f = out->mFaces[i];
		f.mNumIndices = numIndices;
		f.mIndices = new unsigned int[numIndices];
		for (unsigned int i = 0; i < numIndices;++i,++a)
			f.mIndices[i] = a;
	}
	out->mNumVertices = (unsigned int)positions.size();
	out->mVertices = new aiVector3D[out->mNumVertices];
	::memcpy(out->mVertices,&positions[0],out->mNumVertices*sizeof(aiVector3D));
	return out;
}

// ------------------------------------------------------------------------------------------------
aiMesh* StandardShapes::MakeMesh ( unsigned int (*GenerateFunc)(
	std::vector<aiVector3D>&))
{
	std::vector<aiVector3D> temp;
	unsigned num = (*GenerateFunc)(temp);
	return MakeMesh(temp,num);
}

// ------------------------------------------------------------------------------------------------
aiMesh* StandardShapes::MakeMesh ( unsigned int (*GenerateFunc)(
	std::vector<aiVector3D>&, bool))
{
	std::vector<aiVector3D> temp;
	unsigned num = (*GenerateFunc)(temp,true);
	return MakeMesh(temp,num);
}

// ------------------------------------------------------------------------------------------------
aiMesh* StandardShapes::MakeMesh (unsigned int num,  void (*GenerateFunc)(
	unsigned int,std::vector<aiVector3D>&))
{
	std::vector<aiVector3D> temp;
	(*GenerateFunc)(num,temp);
	return MakeMesh(temp,3);
}

// ------------------------------------------------------------------------------------------------
unsigned int StandardShapes::MakeIcosahedron(std::vector<aiVector3D>& positions)
{
	positions.reserve(positions.size()+60);

	const float t = (1.f + 2.236067977f)/2.f;
	const float s = sqrt(1.f + t*t);
	
	aiVector3D v0  = aiVector3D(t,1.f, 0.f)/s;
	aiVector3D v1  = aiVector3D(-t,1.f, 0.f)/s;
	aiVector3D v2  = aiVector3D(t,-1.f, 0.f)/s;
	aiVector3D v3  = aiVector3D(-t,-1.f, 0.f)/s;
	aiVector3D v4  = aiVector3D(1.f, 0.f, t)/s;
	aiVector3D v5  = aiVector3D(1.f, 0.f,-t)/s;
	aiVector3D v6  = aiVector3D(-1.f, 0.f,t)/s;
	aiVector3D v7  = aiVector3D(-1.f, 0.f,-t)/s;
	aiVector3D v8  = aiVector3D(0.f, t, 1.f)/s;
	aiVector3D v9  = aiVector3D(0.f,-t, 1.f)/s;
	aiVector3D v10 = aiVector3D(0.f, t,-1.f)/s;
	aiVector3D v11 = aiVector3D(0.f,-t,-1.f)/s;

	ADD_TRIANGLE(v0,v8,v4);
	ADD_TRIANGLE(v0,v5,v10);
	ADD_TRIANGLE(v2,v4,v9);
	ADD_TRIANGLE(v2,v11,v5);

	ADD_TRIANGLE(v1,v6,v8);
	ADD_TRIANGLE(v1,v10,v7);
	ADD_TRIANGLE(v3,v9,v6);
	ADD_TRIANGLE(v3,v7,v11);

	ADD_TRIANGLE(v0,v10,v8);
	ADD_TRIANGLE(v1,v8,v10);
	ADD_TRIANGLE(v2,v9,v11);
	ADD_TRIANGLE(v3,v11,v9);

	ADD_TRIANGLE(v4,v2,v0);
	ADD_TRIANGLE(v5,v0,v2);
	ADD_TRIANGLE(v6,v1,v3);
	ADD_TRIANGLE(v7,v3,v1);

	ADD_TRIANGLE(v8,v6,v4);
	ADD_TRIANGLE(v9,v4,v6);
	ADD_TRIANGLE(v10,v5,v7);
	ADD_TRIANGLE(v11,v7,v5);
	return 3;
}

// ------------------------------------------------------------------------------------------------
unsigned int StandardShapes::MakeDodecahedron(std::vector<aiVector3D>& positions,
	bool polygons /*= false*/)
{
	positions.reserve(positions.size()+108);

	const float a = 1.f / 1.7320508f;
	const float b = sqrt((3.f-2.23606797f)/6.f);
	const float c = sqrt((3.f+2.23606797f)/6.f);

	aiVector3D v0  = aiVector3D(a,a,a);
	aiVector3D v1  = aiVector3D(a,a,-a);
	aiVector3D v2  = aiVector3D(a,-a,a);
	aiVector3D v3  = aiVector3D(a,-a,-a);
	aiVector3D v4  = aiVector3D(-a,a,a);
	aiVector3D v5  = aiVector3D(-a,a,-a);
	aiVector3D v6  = aiVector3D(-a,-a,a);
	aiVector3D v7  = aiVector3D(-a,-a,-a);
	aiVector3D v8  = aiVector3D(b,c,0.f);
	aiVector3D v9  = aiVector3D(-b,c,0.f);
	aiVector3D v10 = aiVector3D(b,-c,0.f);
	aiVector3D v11 = aiVector3D(-b,-c,0.f);
	aiVector3D v12 = aiVector3D(c, 0.f, b);
	aiVector3D v13 = aiVector3D(c, 0.f, -b);
	aiVector3D v14 = aiVector3D(-c, 0.f, b);
	aiVector3D v15 = aiVector3D(-c, 0.f, -b);
	aiVector3D v16 = aiVector3D(0.f, b, c);
	aiVector3D v17 = aiVector3D(0.f, -b, c);
	aiVector3D v18 = aiVector3D(0.f, b, -c);
	aiVector3D v19 = aiVector3D(0.f, -b, -c);

	ADD_PENTAGON(v0, v8, v9, v4, v16);
	ADD_PENTAGON(v0, v12, v13, v1, v8);
	ADD_PENTAGON(v0, v16, v17, v2, v12);
	ADD_PENTAGON(v8, v1, v18, v5, v9);
	ADD_PENTAGON(v12, v2, v10, v3, v13);
	ADD_PENTAGON(v16, v4, v14, v6, v17);
	ADD_PENTAGON(v9, v5, v15, v14, v4);

	ADD_PENTAGON(v6, v11, v10, v2, v17);
	ADD_PENTAGON(v3, v19, v18, v1, v13);
	ADD_PENTAGON(v7, v15, v5, v18, v19);
	ADD_PENTAGON(v7, v11, v6, v14, v15);
	ADD_PENTAGON(v7, v19, v3, v10, v11);
	return (polygons ? 5 : 3);
}

// ------------------------------------------------------------------------------------------------
unsigned int StandardShapes::MakeOctahedron(std::vector<aiVector3D>& positions)
{
	positions.reserve(positions.size()+24);

	aiVector3D v0  = aiVector3D(1.0f, 0.f, 0.f) ;
	aiVector3D v1  = aiVector3D(-1.0f, 0.f, 0.f);
	aiVector3D v2  = aiVector3D(0.f, 1.0f, 0.f);
	aiVector3D v3  = aiVector3D(0.f, -1.0f, 0.f);
	aiVector3D v4  = aiVector3D(0.f, 0.f, 1.0f);
	aiVector3D v5  = aiVector3D(0.f, 0.f, -1.0f);

	ADD_TRIANGLE(v4,v0,v2);
	ADD_TRIANGLE(v4,v2,v1);
	ADD_TRIANGLE(v4,v1,v3);
	ADD_TRIANGLE(v4,v3,v0);

	ADD_TRIANGLE(v5,v2,v0);
	ADD_TRIANGLE(v5,v1,v2);
	ADD_TRIANGLE(v5,v3,v1);
	ADD_TRIANGLE(v5,v0,v3);
	return 3;
}

// ------------------------------------------------------------------------------------------------
unsigned int StandardShapes::MakeTetrahedron(std::vector<aiVector3D>& positions)
{
	positions.reserve(positions.size()+9);

	const float a = 1.41421f/3.f;
	const float b = 2.4494f/3.f;

	aiVector3D v0  = aiVector3D(0.f,0.f,1.f);
	aiVector3D v1  = aiVector3D(2*a,0,-1.f/3.f);
	aiVector3D v2  = aiVector3D(-a,b,-1.f/3.f);
	aiVector3D v3  = aiVector3D(-a,-b,-1.f/3.f);

	ADD_TRIANGLE(v0,v1,v2);
	ADD_TRIANGLE(v0,v2,v3);
	ADD_TRIANGLE(v0,v3,v1);
	ADD_TRIANGLE(v1,v3,v2);
	return 3;
}

// ------------------------------------------------------------------------------------------------
unsigned int StandardShapes::MakeHexahedron(std::vector<aiVector3D>& positions,
	bool polygons /*= false*/)
{
	positions.reserve(positions.size()+36);
	float length = 1.f/1.73205080f;

	aiVector3D v0  = aiVector3D(-1.f,-1.f,-1.f)*length;
	aiVector3D v1  = aiVector3D(1.f,-1.f,-1.f)*length;
	aiVector3D v2  = aiVector3D(1.f,1.f,-1.f)*length;
	aiVector3D v3  = aiVector3D(-1.f,1.f,-1.f)*length;
	aiVector3D v4  = aiVector3D(-1.f,-1.f,1.f)*length;
	aiVector3D v5  = aiVector3D(1.f,-1.f,1.f)*length;
	aiVector3D v6  = aiVector3D(1.f,1.f,1.f)*length;
	aiVector3D v7  = aiVector3D(-1.f,1.f,1.f)*length;

	ADD_QUAD(v0,v3,v2,v1);
	ADD_QUAD(v0,v1,v5,v4);
	ADD_QUAD(v0,v4,v7,v3);
	ADD_QUAD(v6,v5,v1,v2);
	ADD_QUAD(v6,v2,v3,v7);
	ADD_QUAD(v6,v7,v4,v5);
	return (polygons ? 4 : 3);
}

// Cleanup ...
#undef ADD_TRIANGLE
#undef ADD_QUAD
#undef ADD_PENTAGON

// ------------------------------------------------------------------------------------------------
void StandardShapes::MakeSphere(unsigned int	tess,
	std::vector<aiVector3D>& positions)
{
	MakeIcosahedron(positions);

	for (unsigned int i = 0; i<tess;++i)
		Subdivide(positions);
}

// ------------------------------------------------------------------------------------------------
void StandardShapes::MakeCone(
	aiVector3D&		center1,
	float			radius1,
	aiVector3D&		center2,
	float			radius2,
	unsigned int	tess, 
	std::vector<aiVector3D>& positions, 
	bool bOpened /*= false*/)
{
}

// ------------------------------------------------------------------------------------------------
void StandardShapes::MakeCircle(
	const aiVector3D&	center, 
	const aiVector3D&	normal, 
	float				radius,
	unsigned int		tess,
	std::vector<aiVector3D>& positions)
{
	//aiVector3D current = aiVector3D ( normal.x,
}

} // ! Assimp
