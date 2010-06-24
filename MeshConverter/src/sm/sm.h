#ifndef __SM_H_INCLUDED__
#define __SM_H_INCLUDED__

#include "../assets/material.h"
#include "../geometry/vector3.h"
#include "../geometry/vector2.h"
#include <string>


typedef struct
{
	unsigned int vertices[3];
	unsigned int normals[3];
	unsigned int texcoords[3];
	unsigned short colors[3];
	short material;
} SmPolygon;

typedef struct SmMaterial
{
	Material *material;
	unsigned int polyStart;
	unsigned int polyEnd;

	SmMaterial()
	{
		material = new Material();
		polyStart = 0;
		polyEnd = 0;
	};

	~SmMaterial()
	{
		delete material;
	}
} SmMaterial;

#define NO_MATERIAL -1

class StaticModel
{
public:
	StaticModel();
	virtual ~StaticModel()                                 { Release(); }

	void Release();
	bool Load(const std::string &file);
	bool ConvertToMesh(const std::string &file);

	SmMaterial* GetMaterial(unsigned short index)          { return &m_materials[index]; }
	SmPolygon* GetPolygon(unsigned int index)              { return &m_polygons[index]; }
	Vector3* GetVertex(unsigned int index)                 { return &m_vertices[index]; }
	unsigned int GetNumMaterials()                         { return (unsigned int)m_numMaterials; }
	unsigned int GetNumPolygons()                          { return m_numPolygons; }
	unsigned int GetNumVertices()                          { return m_numVertices; }

private:
	SmMaterial *m_materials;
	SmPolygon *m_polygons;
	Vector3 *m_vertices;
	Vector3 *m_normals;
	Vector2 *m_texCoords;
	int m_numMaterials;
	unsigned int m_numPolygons;
	unsigned int m_numVertices;
	unsigned int m_numNormals;
	unsigned int m_numTexCoords;
	bool m_hasNormals;
	bool m_hasTexCoords;
	bool m_hasColors;
};

#endif