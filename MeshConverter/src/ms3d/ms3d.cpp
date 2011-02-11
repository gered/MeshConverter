#include "ms3d.h"

#include <stdio.h>

Ms3d::Ms3d()
{
	m_numVertices = 0;
	m_numTriangles = 0;
	m_numMeshes = 0;
	m_numMaterials = 0;
	m_vertices = NULL;
	m_triangles = NULL;
	m_meshes = NULL;
	m_materials = NULL;
}

void Ms3d::Release()
{
	delete[] m_vertices;
	delete[] m_triangles;
	delete[] m_meshes;
	delete[] m_materials;
	m_numVertices = 0;
	m_numTriangles = 0;
	m_numMeshes = 0;
	m_numMaterials = 0;
}

bool Ms3d::Load(const std::string &file)
{
	FILE *fp;
	Ms3dHeader header;

	fp = fopen(file.c_str(), "rb");
	if (!fp)
		return false;

	// filetype verification
	fread(&header.id, 10, 1, fp);
	if (strncmp(header.id, "MS3D000000", 10) != 0)
	{
		fclose(fp);
		return false;
	}
	fread(&header.version, 4, 1, fp);
	if (header.version != 4)
	{
		fclose(fp);
		return false;
	}

	// read vertices
	fread(&m_numVertices, 2, 1, fp);
	m_vertices = new Ms3dVertex[m_numVertices];

	for (int i = 0; i < m_numVertices; ++i)
	{
		Ms3dVertex *vertex = &m_vertices[i];

		fread(&vertex->editorFlags, 1, 1, fp);
		fread(&vertex->vertex.x, 4, 1, fp);
		fread(&vertex->vertex.y, 4, 1, fp);
		fread(&vertex->vertex.z, 4, 1, fp);
		fread(&vertex->jointIndex, 1, 1, fp);
		fread(&vertex->unused, 1, 1, fp);
	}

	// read triangle definitions
	fread(&m_numTriangles, 2, 1, fp);
	m_triangles = new Ms3dTriangle[m_numTriangles];

	for (int i = 0; i < m_numTriangles; ++i)
	{
		Ms3dTriangle *triangle = &m_triangles[i];

		fread(&triangle->editorFlags, 2, 1, fp);
		for (int j = 0; j < 3; ++j)
			fread(&triangle->vertices[j], 2, 1, fp);
		for (int j = 0; j < 3; ++j)
		{
			fread(&triangle->normals[j].x, 4, 1, fp);
			fread(&triangle->normals[j].y, 4, 1, fp);
			fread(&triangle->normals[j].z, 4, 1, fp);
		}
		for (int j = 0; j < 3; ++j)
		{
			fread(&triangle->texCoords[j].x, 4, 1, fp);
			fread(&triangle->texCoords[j].y, 4, 1, fp);
		}
		fread(&triangle->smoothingGroup, 1, 1, fp);
		fread(&triangle->meshIndex, 1, 1, fp);
	}

	// read mesh information
	fread(&m_numMeshes, 2, 1, fp);
	m_meshes = new Ms3dMesh[m_numMeshes];

	for (int i = 0; i < m_numMeshes; ++i)
	{
		Ms3dMesh *mesh = &m_meshes[i];

		fread(&mesh->editorFlags, 1, 1, fp);
		fread(&mesh->name, 32, 1, fp);
		fread(&mesh->numTriangles, 2, 1, fp);
		mesh->triangles = new unsigned short[mesh->numTriangles];
		for (int j = 0; j < mesh->numTriangles; ++j)
			fread(&mesh->triangles[j], 2, 1, fp);
		fread(&mesh->materialIndex, 1, 1, fp);
	}

	// read material information
	fread(&m_numMaterials, 2, 1, fp);
	m_materials = new Ms3dMaterial[m_numMaterials];

	for (int i = 0; i < m_numMaterials; ++i)
	{
		Ms3dMaterial *material = &m_materials[i];

		fread(&material->name, 32, 1, fp);
		for (int j = 0; j < 4; ++j)
			fread(&material->ambient[j], 4, 1, fp);
		for (int j = 0; j < 4; ++j)
			fread(&material->diffuse[j], 4, 1, fp);
		for (int j = 0; j < 4; ++j)
			fread(&material->specular[j], 4, 1, fp);
		for (int j = 0; j < 4; ++j)
			fread(&material->emissive[j], 4, 1, fp);
		fread(&material->shininess, 4, 1, fp);
		fread(&material->transparency, 4, 1, fp);
		fread(&material->mode, 1, 1, fp);
		fread(&material->texture, 128, 1, fp);
		fread(&material->alpha, 128, 1, fp);
	}

	fclose(fp);

	return true;
}

bool Ms3d::ConvertToMesh(const std::string &file)
{
	return false;
}
