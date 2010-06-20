#include "obj.h"

#include <stdio.h>
#include <fstream>
#include <sstream>

Obj::Obj()
{
	m_vertices = NULL;
	m_normals = NULL;
	m_texCoords = NULL;
	m_materials = NULL;
	m_numVertices = 0;
	m_numNormals = 0;
	m_numTexCoords = 0;
	m_numMaterials = 0;
}

void Obj::Release()
{
	delete[] m_vertices;
	delete[] m_normals;
	delete[] m_texCoords;
	delete[] m_materials;
	m_vertices = NULL;
	m_normals = NULL;
	m_texCoords = NULL;
	m_materials = NULL;
	m_numVertices = 0;
	m_numNormals = 0;
	m_numTexCoords = 0;
	m_numMaterials = 0;
}

bool Obj::Load(const std::string &file, const std::string &texturePath)
{
	std::ifstream input;
	std::string line;
	std::string op;
	std::string path;
	std::string tempName;
	Vector3 vertex;
	Vector3 normal;
	ObjMaterial *currentMaterial = NULL;
	int currentVertex = 0;
	int currentNormal = 0;
	int currentTexCoord = 0;
	int numGroups = 0;

	// Get pathname from filename given (if present)
	// Need this as we assume any .mtl files specified are in the same path as this .obj file
	if (file.find_last_of('/') != std::string::npos)
		path = file.substr(0, file.find_last_of('/') + 1);

	if (!FindAndLoadMaterials(path, texturePath, file))
		return false;
	if (!GetDataSizes(file))
		return false;

	input.open(file.c_str());
	if (input.fail())
		return false;

	// Extract name of model from the filename given (basically, chop off the extension and path)
	//if (file.find_last_of('.') != std::string::npos)
	//	model->name = file.substr(path.length(), file.find_last_of('.') - path.length());

	// Begin reading in model data
	while (!input.eof())
	{
		if (numGroups > 1)
			break;

		std::getline(input, line, '\n');

		op = line.substr(0, line.find(' '));

		// Comments
		if (op == "#")
		{
		}

		// Vertex
		else if (op == "v")
		{
			sscanf(line.c_str(), "v %f %f %f", &m_vertices[currentVertex].x, &m_vertices[currentVertex].y, &m_vertices[currentVertex].z);
			++currentVertex;
		}

		// Texture coordinate
		else if (op ==  "vt")
		{
			sscanf(line.c_str(), "vt %f %f", &m_texCoords[currentTexCoord].x, &m_texCoords[currentTexCoord].y);
			m_texCoords[currentTexCoord].x = -m_texCoords[currentTexCoord].y;
			++currentTexCoord;
		}

		// Vertex normal
		else if (op == "vn")
		{
			sscanf(line.c_str(), "vn %f %f %f", &m_normals[currentNormal].x, &m_normals[currentNormal].y, &m_normals[currentNormal].z);
			++currentNormal;
		}

		// Face definition
		else if (op == "f")
		{
			ParseFaceDefinition(line, currentMaterial);
		}

		// Group name
		else if (op == "g")
		{
			++numGroups;
		}

		// Object name
		else if (op == "o")
		{
		}

		// Material
		else if (op == "usemtl")
		{
			tempName = line.substr(line.find(' ') + 1);

			currentMaterial = NULL;

			// Find the named material and set it as current
			for (unsigned int i = 0; i < m_numMaterials; ++i)
			{
				if (m_materials[i].name == tempName)
				{
					currentMaterial = &m_materials[i];
					break;
				}
			}

			//ASSERT(currentMaterial != NULL);
		}

		// Material file
		else if (op == "mtllib")
		{
			// Already would have been loaded
		}
	}

	input.close();

	return true;
}

void Obj::ParseFaceDefinition(const std::string &faceDefinition, ObjMaterial *currentMaterial)
{
	static int numFaceVertices = 0;
	static OBJ_FACE_VERTEX_TYPE vertexType;
	std::string def;
	int pos;
	int n = 0;

	// Just get the vertex index part of the line (can be variable length, and we dont want the newline at the end)
	pos = faceDefinition.find(' ') + 1;
	def = faceDefinition.substr(pos);

	// Few different face formats, and variable amount of vertices per face possible

	// How many vertices are there in this face definition? (only calc this once)
	// Also calc the vertex format
	if (!numFaceVertices)
	{
		pos = 0;
		while (def.length() > 0 && def[def.length() - 1] == ' ')
			def = def.substr(0, def.length() - 1);
		//ASSERT(def.length() > 0);

		while (pos != std::string::npos)
		{
			++pos;
			++numFaceVertices;
			pos = def.find(' ', pos);
		}

		std::string tempVertex = def.substr(0, def.find(' '));
		if (tempVertex.find("//") != std::string::npos)
			vertexType = OBJ_VERTEX_NORMAL;
		else
		{
			pos = 0;
			while (pos != std::string::npos)
			{
				++pos;
				++n;
				pos = tempVertex.find('/', pos);
			}

			if (n == 1)
				vertexType = OBJ_VERTEX_TEXCOORD;
			else
				vertexType = OBJ_VERTEX_FULL;
		}
	}

	// Parse out vertices in this face
	// We also store only triangles. Since OBJ file face definitions can have any number
	// of vertices per face, we need to split it up into triangles here for easy rendering
	// This is done as follows:
	// - first 3 vertices = first triangle for face
	// - for each additional 1 vertex, take the first vertex read for this face + the previously
	//   read vertex for this face and combine to make a new triangle
	std::istringstream parser;
	std::string currentVertex;
	int thisVertex[3];
	int firstVertex[3];
	int lastReadVertex[3];
	int thisTriangle[3][3];
	ObjFace face;

	memset(&firstVertex, 0, sizeof(int) * 3);
	memset(&lastReadVertex, 0, sizeof(int) * 3);
	memset(&thisTriangle, 0, sizeof(int) * (3 * 3));
	parser.clear();
	parser.str(def);

	for (int i = 0; i < numFaceVertices; ++i)
	{
		// Get current vertex for this face
		parser >> currentVertex;

		// Add vertex/texcoord/normal indexes to the data arrays 
		// (OBJ file indexes are NOT zero based. We fix that here)
		memset(&thisVertex, 0, sizeof(int) * 3);
		switch (vertexType)
		{
		case OBJ_VERTEX_FULL:			// v/vt/vn
			sscanf(currentVertex.c_str(), "%d/%d/%d", &thisVertex[0], &thisVertex[1], &thisVertex[2]);
			break;
		case OBJ_VERTEX_NORMAL:			// v//vn
			sscanf(currentVertex.c_str(), "%d//%d", &thisVertex[0], &thisVertex[2]);
			break;
		case OBJ_VERTEX_TEXCOORD:		// v/vt
			sscanf(currentVertex.c_str(), "%d/%d", &thisVertex[0]);
			break;
		}

		// Save the first vertex read for a face
		if (i == 0)
			memcpy(&firstVertex, &thisVertex, sizeof(int) * 3);

		// First 3 vertices simply form a triangle
		if (i <= 2)
		{
			face.vertices[i] = thisVertex[0] - 1;
			face.texcoords[i] = thisVertex[1] - 1;
			face.normals[i] = thisVertex[2] - 1;
		}

		// Store the first triangle
		if (i == 2)
		{
			//ASSERT(currentMaterial != NULL);
			currentMaterial->faces[currentMaterial->lastFaceIndex] = face;
			++currentMaterial->lastFaceIndex;
		}

		// Combine vertices to form additional triangles
		if (i > 2)
		{
			//ASSERT(currentMaterial != NULL);
			face.vertices[0] = firstVertex[0] - 1;		face.texcoords[0] = firstVertex[1] - 1;		face.normals[0] = firstVertex[2] - 1;
			face.vertices[1] = lastReadVertex[0] - 1;	face.texcoords[1] = lastReadVertex[1] - 1;	face.normals[1] = lastReadVertex[2] - 1;
			face.vertices[2] = thisVertex[0] - 1;		face.texcoords[2] = thisVertex[1] - 1;		face.normals[2] = thisVertex[2] - 1;
			currentMaterial->faces[currentMaterial->lastFaceIndex] = face;
			++currentMaterial->lastFaceIndex;
		}

		// Save as "previously read vertex"
		memcpy(&lastReadVertex, &thisVertex, sizeof(int) * 3);
	}
}

bool Obj::GetDataSizes(const std::string &file)
{
	std::ifstream input;
	std::string line;
	std::string op;
	int countVertices = 0;
	int countNormals = 0;
	int countTexCoords = 0;
	int numGroups = 0;
	std::string useMtlName;
	ObjMaterial *currentMaterial;

	input.open(file.c_str());
	if (input.fail())
		return false;

	while (!input.eof())
	{
		if (numGroups > 1)
			break;

		std::getline(input, line, '\n');

		op = line.substr(0, line.find(' '));

		if (op == "g")
			++numGroups;
		else if (op == "v")
			++countVertices;
		else if (op == "vt")
			++countTexCoords;
		else if (op == "vn")
			++countNormals;
		else if (op == "f")
			// TODO: count number of vertices per face definition, and adjust the ++ operation accordingly (i.e. for 4 vertices per face, needs to be "+= 2")
			++currentMaterial->numFaces;

		else if (op == "usemtl")
		{
			std::string useMtlName = line.substr(line.find(' ') + 1);

			currentMaterial = NULL;

			// Find the named material and set it as current
			for (unsigned int i = 0; i < m_numMaterials; ++i)
			{
				if (m_materials[i].name == useMtlName)
				{
					currentMaterial = &m_materials[i];
					break;
				}
			}

			//ASSERT(currentMaterial != NULL);
		}
	}

	input.close();

	m_numVertices = countVertices;
	m_numTexCoords = countTexCoords;
	m_numNormals = countNormals;
	m_vertices = new Vector3[m_numVertices];
	m_texCoords = new Vector2[m_numTexCoords];
	m_normals = new Vector3[m_numNormals];
	for (unsigned int i = 0; i < m_numMaterials; ++i)
	{
		m_materials[i].faces = new ObjFace[m_materials[i].numFaces];
	}

	return true;
}

bool Obj::LoadMaterialLibrary(const std::string &file, const std::string &texturePath)
{
	std::ifstream input;
	std::string line;
	std::string op;
	int currentMaterial = -1;
	float r, g, b;

	if (!CountDefinedMaterials(file))
		return false;
	m_materials = new ObjMaterial[m_numMaterials];

	input.open(file.c_str());
	if (input.fail())
		return false;

	while (!input.eof())
	{
		std::getline(input, line, '\n');

		op = line.substr(0, line.find(' '));

		// New material definition (possibility of multiple per .mtl file)
		if (op == "newmtl")
		{
			++currentMaterial;
			m_materials[currentMaterial].name = line.substr(op.length() + 1);
		}

		// Ambient color
		else if (op == "Ka")
		{
			//ASSERT(currentMaterial >= 0);
			sscanf(line.c_str(), "Ka %f %f %f", &r, &g, &b);
			m_materials[currentMaterial].material->SetAmbient(RGB_24_f(r, g, b));
		}

		// Diffuse color
		else if (op == "Kd")
		{
			//ASSERT(currentMaterial >= 0);
			sscanf(line.c_str(), "Kd %f %f %f", &r, &g, &b);
			m_materials[currentMaterial].material->SetDiffuse(RGB_24_f(r, g, b));
		}

		// Specular color
		else if (op == "Ks")
		{
			//ASSERT(currentMaterial >= 0);
			sscanf(line.c_str(), "Ks %f %f %f", &r, &g, &b);
			m_materials[currentMaterial].material->SetSpecular(RGB_24_f(r, g, b));
		}

		// Alpha value
		else if (op == "d" || op == "Tr")
		{
			//ASSERT(currentMaterial >= 0);
		}

		// Shininess
		else if (op == "Ns")
		{
			//ASSERT(currentMaterial >= 0);
		}

		// Illumination model
		else if (op == "illum")
		{
			//ASSERT(currentMaterial >= 0);
		}

		// Texture
		else if (op == "map_Ka" || op == "map_Kd")
		{
			//ASSERT(currentMaterial >= 0);
			m_materials[currentMaterial].material->SetTexture(texturePath + line.substr(op.length() + 1));
		}

	}

	input.close();

	return true;
}

bool Obj::CountDefinedMaterials(const std::string &file)
{
	std::ifstream input;
	std::string line;
	std::string op;
	int count = 0;

	input.open(file.c_str());
	if (input.fail())
		return false;

	while (!input.eof())
	{
		std::getline(input, line, '\n');

		op = line.substr(0, line.find(' '));
		if (op == "newmtl")
			++m_numMaterials;
	}

	input.close();

	return true;
}

bool Obj::FindAndLoadMaterials(const std::string &materialPath, const std::string &texturePath, const std::string &file)
{
	std::ifstream input;
	std::string line;
	std::string op;

	input.open(file.c_str());
	if (input.fail())
		return false;

	while (!input.eof())
	{
		std::getline(input, line, '\n');

		op = line.substr(0, line.find(' '));

		if (op == "mtllib")
		{
			LoadMaterialLibrary(materialPath + line.substr(line.find(' ') + 1), texturePath);
			break;
		}
	}

	input.close();

	return true;
}

bool Obj::ConvertToMesh(const std::string &file)
{
	return false;
}
