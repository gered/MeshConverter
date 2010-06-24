#include "sm.h"

#include <stdio.h>

StaticModel::StaticModel()
{
	m_numMaterials = 0;
	m_numPolygons = 0;
	m_numVertices = 0;
	m_numNormals = 0;
	m_numTexCoords = 0;
	m_hasNormals = false;
	m_hasTexCoords = false;
	m_hasColors = false;
	m_materials = NULL;
	m_polygons = NULL;
	m_texCoords = NULL;
	m_vertices = NULL;
	m_normals = NULL;
}

void StaticModel::Release()
{
	delete[] m_materials;
	delete[] m_polygons;
	delete[] m_texCoords;
	delete[] m_vertices;
	delete[] m_normals;
}

bool StaticModel::Load(const std::string &file)
{
	FILE *fp;
	unsigned short numMaterials;
	unsigned int numPolys, numVertices, numNormals, numTexCoords;
	unsigned int ambient, diffuse, specular, emission;
	unsigned int n;
	int currentMaterial;
	int count;
	float x, y, z;
	unsigned char header[2];
	unsigned char c;
	std::string texture;

	fp = fopen(file.c_str(), "rb");
	if (!fp)
		return false;

	// Simple file type validation
	fread(&header[0], 2, 1, fp);
	if (header[0] != 'S' || header[1] != 'M')
		return false;

	fread(&numMaterials, 2, 1, fp);
	fread(&numPolys, 4, 1, fp);
	fread(&numVertices, 4, 1, fp);
	fread(&numNormals, 4, 1, fp);
	fread(&numTexCoords, 4, 1, fp);

	m_materials = new SmMaterial[numMaterials];
	m_polygons = new SmPolygon[numPolys];
	m_texCoords = new Vector2[numTexCoords];
	m_vertices = new Vector3[numVertices];
	m_normals = new Vector3[numNormals];

	//ASSERT(m_materials != NULL);
	//ASSERT(m_polygons != NULL);
	//ASSERT(m_texCoords != NULL);
	//ASSERT(m_vertices != NULL);
	//ASSERT(m_normals != NULL);

	m_numMaterials = numMaterials;
	m_numPolygons = numPolys;
	m_numVertices = numVertices;
	m_numNormals = numNormals;
	m_numTexCoords = numTexCoords;

	// Read in material definitions
	for (int i = 0; i < m_numMaterials; ++i)
	{
		fread(&ambient, 4, 1, fp);
		fread(&diffuse, 4, 1, fp);
		fread(&specular, 4, 1, fp);
		fread(&emission, 4, 1, fp);

		m_materials[i].material->SetAmbient(ambient);
		m_materials[i].material->SetDiffuse(diffuse);
		m_materials[i].material->SetSpecular(specular);
		m_materials[i].material->SetEmission(emission);

		// Read up to the null terminator on the texture filename (could be any length)
		count = 0;
		texture = "";
		do
		{
			fread(&c, 1, 1, fp);
			if (c)
				texture += c;
		} while (c != '\0');
		m_materials[i].material->SetTexture(texture);
	}

	// Read in triangle definitions (all are indexes into raw data following)
	currentMaterial = NO_MATERIAL;
	for (unsigned int i = 0; i < m_numPolygons; ++i)
	{
		// Vertices
		for (int j = 0; j < 3; ++j)
		{
			fread(&n, 4, 1, fp);
			m_polygons[i].vertices[j] = n;
		}

		// Normals
		for (int j = 0; j < 3; ++j)
		{
			fread(&n, 4, 1, fp);
			m_polygons[i].normals[j] = n;
		}

		// TexCoords
		for (int j = 0; j < 3; ++j)
		{
			fread(&n, 4, 1, fp);
			m_polygons[i].texcoords[j] = n;
		}
		
		// Vertex colors
		for (int j = 0; j < 3; ++j)
		{
			fread(&n, 2, 1, fp);
			m_polygons[i].colors[j] = n;
		}

		// Material index
		fread(&m_polygons[i].material, 2, 1, fp);

		// Record start/end indices for the different materials
		// This way rendering can be done per material while still only looping
		// over all of this model's polygons once per frame
		if (m_polygons[i].material != currentMaterial)
		{
			if (currentMaterial > NO_MATERIAL)
				m_materials[currentMaterial].polyEnd = i;

			currentMaterial = m_polygons[i].material;
			m_materials[currentMaterial].polyStart = i;
		}

		// If any of the normals, texcoords, or colors are non-zero, we need to flag that such
		// information exists in this model
		if (m_polygons[i].colors[0] != 0 || m_polygons[i].colors[1] != 0 || m_polygons[i].colors[2] != 0)
			m_hasColors = true;
	}

	// Will always include the last polygon due to the way the .SM exporter sorts
	m_materials[currentMaterial].polyEnd = numPolys;

	// Vertices
	for (unsigned int i = 0; i < m_numVertices; ++i)
	{
		fread(&x, 4, 1, fp);
		fread(&y, 4, 1, fp);
		fread(&z, 4, 1, fp);

		m_vertices[i].x = x / 2;
		m_vertices[i].y = y / 2;
		m_vertices[i].z = z / 2;
	}

	// Normals
	for (unsigned int i = 0; i < m_numNormals; ++i)
	{
		fread(&x, 4, 1, fp);
		fread(&y, 4, 1, fp);
		fread(&z, 4, 1, fp);
		//ASSERT(!((x >= 1.0f || x <= -1.0f) ||
		//	(y >= 1.0f || y <= -1.0f) ||
		//	(z >= 1.0f || z <= -1.0f)));
		m_normals[i].x = x;
		m_normals[i].y = y;
		m_normals[i].z = z;

		if (m_normals[i].x != 0 || m_normals[i].y != 0 || m_normals[i].z != 0)
			m_hasNormals = true;
	}

	// Texture coordinates
	for (unsigned int i = 0; i < m_numTexCoords; ++i)
	{
		fread(&x, 4, 1, fp);
		fread(&y, 4, 1, fp);
		//ASSERT(!((x >= 2048.0f || x <= -2048.0f) ||
		//	(y >= 2048.0f || y <= -2048.0f)));

		// TODO: revisit this, seems a bit hack-ish and probably only correct 
		// for the shitty models I made
		m_texCoords[i].x = x;
		m_texCoords[i].y = -y;

		if (m_texCoords[i].x != 0 || m_texCoords[i].y != 0)
			m_hasTexCoords = true;
	}

	fclose(fp);

	return true;
}

bool StaticModel::ConvertToMesh(const std::string &file)
{
	FILE *fp = fopen(file.c_str(), "wb");
	if (fp == NULL)
		return false;

	fputs("MESH", fp);
	unsigned char version = 1;
	fwrite(&version, 1, 1, fp);

	// vertices chunk
	fputs("VTX", fp);
	long numVertices = m_numVertices;
	long sizeofVertices = (sizeof(float) * 3) * numVertices + sizeof(long);
	fwrite(&sizeofVertices, sizeof(long), 1, fp);
	fwrite(&numVertices, sizeof(long), 1, fp);
	for (long i = 0; i < numVertices; ++i)
	{
		const Vector3 *vector = &m_vertices[i];
		fwrite(&vector->x, sizeof(float), 1, fp);
		fwrite(&vector->y, sizeof(float), 1, fp);
		fwrite(&vector->z, sizeof(float), 1, fp);
	}

	// normals chunk
	fputs("NRL", fp);
	long numNormals = m_numNormals;
	long sizeofNormals = (sizeof(float) * 3) * numNormals + sizeof(long);
	fwrite(&sizeofNormals, sizeof(long), 1, fp);
	fwrite(&numNormals, sizeof(long), 1, fp);
	for (long i = 0; i < numNormals; ++i)
	{
		const Vector3 *normal = &m_normals[i];
		fwrite(&normal->x, sizeof(float), 1, fp);
		fwrite(&normal->y, sizeof(float), 1, fp);
		fwrite(&normal->z, sizeof(float), 1, fp);
	}

	// texture coordinates chunk
	fputs("TXT", fp);
	long numTexCoords = m_numTexCoords;
	long sizeofTexCoords = (sizeof(float) * 2) * numTexCoords + sizeof(long);
	fwrite(&sizeofTexCoords, sizeof(long), 1, fp);
	fwrite(&numTexCoords, sizeof(long), 1, fp);
	for (long i = 0; i < numTexCoords; ++i)
	{
		const Vector2 *texCoord = &m_texCoords[i];
		fwrite(&texCoord->x, sizeof(float), 1, fp);
		fwrite(&texCoord->y, sizeof(float), 1, fp);
	}

	// materials chunk
	fputs("MTL", fp);

	long numMaterials = m_numMaterials;

	// figure out the size of all the material texture filename strings
	long sizeofNames = 0;
	for (int i = 0; i < numMaterials; ++i)
		sizeofNames += m_materials[i].material->GetTexture().length() + 1;

	long sizeofMaterials = numMaterials + sizeof(long);
	fwrite(&sizeofMaterials, sizeof(long), 1, fp);
	fwrite(&numMaterials, sizeof(long), 1, fp);
	for (long i = 0; i < numMaterials; ++i)
	{
		const SmMaterial *material = &m_materials[i];
		fputs(material->material->GetTexture().c_str(), fp);
		fwrite("\0", 1, 1, fp);
	}

	// triangles chunk
	fputs("TRI", fp);
	long numPolys = m_numPolygons;
	long sizeofPolys = ((sizeof(long) * 3) * 3 + sizeof(long)) * numPolys + sizeof(long);
	fwrite(&sizeofPolys, sizeof(long), 1, fp);
	fwrite(&numPolys, sizeof(long), 1, fp);
	for (long i = 0; i < numPolys; ++i)
	{
		const SmPolygon *triangle = &m_polygons[i];
		long data;

		data = triangle->vertices[0];
		fwrite(&data, sizeof(long), 1, fp);
		data = triangle->vertices[1];
		fwrite(&data, sizeof(long), 1, fp);
		data = triangle->vertices[2];
		fwrite(&data, sizeof(long), 1, fp);

		data = triangle->normals[0];
		fwrite(&data, sizeof(long), 1, fp);
		data = triangle->normals[1];
		fwrite(&data, sizeof(long), 1, fp);
		data = triangle->normals[2];
		fwrite(&data, sizeof(long), 1, fp);

		data = triangle->texcoords[0];
		fwrite(&data, sizeof(long), 1, fp);
		data = triangle->texcoords[1];
		fwrite(&data, sizeof(long), 1, fp);
		data = triangle->texcoords[2];
		fwrite(&data, sizeof(long), 1, fp);

		data = triangle->material;
		fwrite(&data, sizeof(long), 1, fp);
	}

	fclose(fp);
	return true;
}
