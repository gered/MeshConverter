#ifndef __MD2_H_INCLUDED__
#define __MD2_H_INCLUDED__

#include "../geometry/vector3.h"
#include "../geometry/vector2.h"

#include <string>
#include <vector>

#define MD2_SKIN_NAME_LENGTH 64
#define MD2_FRAME_NAME_LENGTH 16

typedef struct 
{
	char ident[4];                               // Should be "IDP2"
	int version;                                 // Should be 8
	int skinWidth;                               // Texture dimensions
	int skinHeight;
	int frameSize;                               // Size of a single frame in bytes
	int numSkins;
	int numVertices;
	int numTexCoords;
	int numPolys;
	int numGlCmds;
	int numFrames;
	int offsetSkins;
	int offsetTexCoords;
	int offsetPolys;
	int offsetFrames;
	int offsetGlCmds;
	int offsetEnd;
} Md2Header;

// Vertex and texcoord indices
typedef struct
{
	unsigned short vertex[3];
	unsigned short texCoord[3];
} Md2Polygon;

// For each keyframe, stores the vertices, normals, and the string name
typedef struct Md2Frame
{
	std::string name;
	Vector3 *vertices;
	Vector3 *normals;

	Md2Frame()
	{
		vertices = NULL;
		normals = NULL;
	}

	~Md2Frame()
	{
		delete[] vertices;
		delete[] normals;
	}
} Md2Frame;

typedef struct
{
	std::string name;
	unsigned int startFrame;
	unsigned int endFrame;
} Md2Animation;

class Md2
{
public:
	Md2();
	virtual ~Md2()                                  { Release(); }

	void Release();
	bool Load(const std::string &file);
	bool ConvertToMesh(const std::string &file);

	int GetNumFrames()                              { return m_numFrames; }
	int GetNumVertices()                            { return m_numVertices; }
	int GetNumTexCoords()                           { return m_numTexCoords; }
	int GetNumPolys()                               { return m_numPolys; }
	int GetNumSkins()                               { return m_numSkins; }
	Md2Frame* GetFrames()                           { return m_frames; }
	Md2Polygon* GetPolygons()                       { return m_polys; }
	Vector2* GetTexCoords()                         { return m_texCoords; }

private:
	int m_numFrames;
	int m_numVertices;
	int m_numTexCoords;
	int m_numPolys;
	int m_numSkins;
	Md2Frame *m_frames;
	Md2Polygon *m_polys;
	Vector2 *m_texCoords;
	std::string *m_skins;
	std::vector<Md2Animation> m_animations;
};

#endif