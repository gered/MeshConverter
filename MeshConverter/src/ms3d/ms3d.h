#ifndef __MS3D_H_INCLUDED__
#define __MS3D_H_INCLUDED__

#include <string>
#include "../geometry/vector3.h"
#include "../geometry/vector2.h"

struct Ms3dHeader
{
	char id[10];
	long version;
};

struct Ms3dVertex
{
	unsigned char editorFlags;
	Vector3 vertex;
	char jointIndex;
	unsigned char unused;
};

struct Ms3dTriangle
{
	unsigned short editorFlags;
	unsigned short vertices[3];
	Vector3 normals[3];
	Vector2 texCoords[3];
	unsigned char smoothingGroup;
	unsigned char meshIndex;
};

struct Ms3dMesh
{
	unsigned char editorFlags;
	char name[32];
	unsigned short numTriangles;
	unsigned short *triangles;
	char materialIndex;

	Ms3dMesh()
	{
		triangles = NULL;
	}

	~Ms3dMesh()
	{
		delete[] triangles;
	}
};

struct Ms3dMaterial
{
	char name[32];
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float emissive[4];
	float shininess;
	float transparency;
	char mode;
	char texture[128];
	char alpha[128];
};

struct Ms3dKeyFrame
{
	float time;
	Vector3 param;
};

struct Ms3dJoint
{
	unsigned char editorFlags;
	char name[32];
	char parentName[32];
	Vector3 rotation;
	Vector3 position;
	unsigned short numRotationFrames;
	unsigned short numTranslationFrames;
	Ms3dKeyFrame *rotationFrames;
	Ms3dKeyFrame *translationFrames;

	Ms3dJoint()
	{
		rotationFrames = NULL;
		translationFrames = NULL;
	}

	~Ms3dJoint()
	{
		delete[] rotationFrames;
		delete[] translationFrames;
	}
};

class Ms3d
{
public:
	Ms3d();
	virtual ~Ms3d()                                        { Release(); }

	void Release();
	bool Load(const std::string &file);
	bool ConvertToMesh(const std::string &file);

	unsigned short GetNumVertices()                        { return m_numVertices; }
	unsigned short GetNumTriangles()                       { return m_numTriangles; }
	unsigned short GetNumMeshes()                          { return m_numMeshes; }
	unsigned short GetNumMaterials()                       { return m_numMaterials; }
	unsigned short GetNumJoints()                          { return m_numJoints; }
	Ms3dVertex* GetVertices()                              { return m_vertices; }
	Ms3dTriangle* GetTriangles()                           { return m_triangles; }
	Ms3dMesh* GetMeshes()                                  { return m_meshes; }
	Ms3dMaterial* GetMaterials()                           { return m_materials; }
	Ms3dJoint* GetJoints()                                 { return m_joints; }

private:
	unsigned short m_numVertices;
	unsigned short m_numTriangles;
	unsigned short m_numMeshes;
	unsigned short m_numMaterials;
	unsigned short m_numJoints;
	float m_animationFps;
	float m_editorAnimationTime;
	int m_numFrames;
	Ms3dVertex *m_vertices;
	Ms3dTriangle *m_triangles;
	Ms3dMesh *m_meshes;
	Ms3dMaterial *m_materials;
	Ms3dJoint *m_joints;
};

#endif
