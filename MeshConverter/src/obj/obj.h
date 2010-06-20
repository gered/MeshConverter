#ifndef __OBJ_H_INCLUDED__
#define __OBJ_H_INCLUDED__

#include "../geometry/vector3.h"
#include "../geometry/vector2.h"
#include "../assets/material.h"

#include <string>

typedef enum OBJ_FACE_VERTEX_TYPE
{
	OBJ_VERTEX_TEXCOORD,
	OBJ_VERTEX_NORMAL,
	OBJ_VERTEX_FULL
};

typedef struct
{
	unsigned int vertices[3];
	unsigned int texcoords[3];
	unsigned int normals[3];
} ObjFace;

typedef struct ObjMaterial
{
	std::string name;
	Material *material;
	ObjFace *faces;
	unsigned int numFaces;
	unsigned int lastFaceIndex;

	ObjMaterial()
	{
		material = new Material();
		faces = NULL;
		numFaces = 0;
		lastFaceIndex = 0;
	}

	~ObjMaterial()
	{
		delete material;
		delete[] faces;
	}
} ObjMaterial;

class Obj
{
public:
	Obj();
	virtual ~Obj()                                  { Release(); }

	void Release();
	bool Load(const std::string &file, const std::string &texturePath);
	bool ConvertToMesh(const std::string &file);

	int GetNumVertices()                            { return m_numVertices; }
	int GetNumNormals()                             { return m_numNormals; }
	int GetNumTexCoords()                           { return m_numTexCoords; }
	int GetNumMaterials()                           { return m_numMaterials; }
	Vector3* GetVertices()                          { return m_vertices; }
	Vector3* GetNormals()                           { return m_normals; }
	Vector2* GetTexCoords()                         { return m_texCoords; }
	ObjMaterial* GetMaterials()                     { return m_materials; }

private:
	bool GetDataSizes(const std::string &file);
	bool LoadMaterialLibrary(const std::string &file, const std::string &texturePath);
	bool CountDefinedMaterials(const std::string &file);
	bool FindAndLoadMaterials(const std::string &materialPath, const std::string &texturePath, const std::string &file);
	void ParseFaceDefinition(const std::string &faceDefinition, ObjMaterial *currentMaterial);

	Vector3 *m_vertices;
	Vector3 *m_normals;
	Vector2 *m_texCoords;
	ObjMaterial *m_materials;
	unsigned int m_numVertices;
	unsigned int m_numNormals;
	unsigned int m_numTexCoords;
	unsigned int m_numMaterials;

};

#endif