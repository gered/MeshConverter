#include "md2.h"

#include <stdio.h>

Md2::Md2()
{
	m_numFrames = 0;
	m_numPolys = 0;
	m_numTexCoords = 0;
	m_numVertices = 0;
	m_numSkins = 0;
	m_frames = NULL;
	m_polys = NULL;
	m_texCoords = NULL;
	m_skins = NULL;
}

void Md2::Release()
{
	delete[] m_frames;
	delete[] m_polys;
	delete[] m_texCoords;
	delete[] m_skins;
	m_numFrames = 0;
	m_numPolys = 0;
	m_numTexCoords = 0;
	m_numVertices = 0;
	m_numSkins = 0;
	m_frames = NULL;
	m_polys = NULL;
	m_texCoords = NULL;
	m_skins = NULL;
}

bool Md2::Load(const std::string &file)
{
	FILE *fp;
	unsigned char c;
	unsigned short u, v, t;
	float x, y, z;
	Md2Header header;
	Vector3 scale, translate;

	fp = fopen(file.c_str(), "rb");
	if (!fp)
		return false;

	// Simple filetype verification
	fread(&header.ident, 4, 1, fp);
	if (header.ident[0] != 'I' || header.ident[1] != 'D' || header.ident[2] != 'P' || header.ident[3] != '2')
	{
		fclose(fp);
		return false;
	}
	fread(&header.version, 4, 1, fp);
	if (header.version != 8)
	{
		fclose(fp);
		return false;
	}

	Release();

	// Read rest of the MD2 header
	fread(&header.skinWidth, 4, 1, fp);
	fread(&header.skinHeight, 4, 1, fp);
	fread(&header.frameSize, 4, 1, fp);
	fread(&header.numSkins, 4, 1, fp);
	fread(&header.numVertices, 4, 1, fp);
	fread(&header.numTexCoords, 4, 1, fp);
	fread(&header.numPolys, 4, 1, fp);
	fread(&header.numGlCmds, 4, 1, fp);
	fread(&header.numFrames, 4, 1, fp);
	fread(&header.offsetSkins, 4, 1, fp);
	fread(&header.offsetTexCoords, 4, 1, fp);
	fread(&header.offsetPolys, 4, 1, fp);
	fread(&header.offsetFrames, 4, 1, fp);
	fread(&header.offsetGlCmds, 4, 1, fp);
	fread(&header.offsetEnd, 4, 1, fp);

	// Allocate memory
	if (header.numSkins > 0)
	{
		m_skins = new std::string[header.numSkins];
		//ASSERT(m_skins != NULL);
	}
	m_texCoords = new Vector2[header.numTexCoords];
	m_polys = new Md2Polygon[header.numPolys];
	m_frames = new Md2Frame[header.numFrames];

	//ASSERT(m_texCoords != NULL);
	//ASSERT(m_polys != NULL);
	//ASSERT(m_frames != NULL);

	// Save model properties
	m_numFrames = header.numFrames;
	m_numPolys = header.numPolys;
	m_numSkins = header.numSkins;
	m_numTexCoords = header.numTexCoords;
	m_numVertices = header.numVertices;

	// Read skin info
	fseek(fp, header.offsetSkins, SEEK_SET);
	for (int i = 0; i < header.numSkins; ++i)
	{
		// Not wasting the full 64 characters stored in the file here
		for (int j = 0; j < MD2_SKIN_NAME_LENGTH; ++j)
		{
			fread(&c, 1, 1, fp);
			if (!c)
			{
				fseek(fp, MD2_SKIN_NAME_LENGTH - j - 1, SEEK_CUR);
				break;
			}
			else
				m_skins[i].append(1, c);
		}
	}

	// Read texture coordinates
	fseek(fp, header.offsetTexCoords, SEEK_SET);
	for (int i = 0; i < header.numTexCoords; ++i)
	{
		fread(&u, 2, 1, fp);
		fread(&v, 2, 1, fp);
		m_texCoords[i].x = u / (float)header.skinWidth;
		m_texCoords[i].y = v / (float)header.skinHeight;
	}

	// Read polygons (this is all just indexes into m_texCoords and m_frames[].vertices)
	fseek(fp, header.offsetPolys, SEEK_SET);
	for (int i = 0; i < header.numPolys; ++i)
	{
		fread(&t, 2, 1, fp);
		m_polys[i].vertex[0] = t;
		fread(&t, 2, 1, fp);
		m_polys[i].vertex[2] = t;
		fread(&t, 2, 1, fp);
		m_polys[i].vertex[1] = t;

		// HACK: Not sure why some of these indexes are invalid? This seems to fix the problem
		fread(&t, 2, 1, fp);
		m_polys[i].texCoord[0] = (t == 65535 ? 0 : t);
		fread(&t, 2, 1, fp);
		m_polys[i].texCoord[2] = (t == 65535 ? 0 : t);
		fread(&t, 2, 1, fp);
		m_polys[i].texCoord[1] = (t == 65535 ? 0 : t);
	}

	// Read frames
	fseek(fp, header.offsetFrames, SEEK_SET);
	for (int i = 0; i < header.numFrames; ++i)
	{
		// Allocate enough memory for this frame's vertex/normal indexes
		m_frames[i].vertices = new Vector3[header.numVertices];
		//m_frames[i].normals = new Vector3[header.numPolys];
		m_frames[i].normals = new Vector3[header.numVertices];
		//ASSERT(m_frames[i].vertices != NULL);
		//ASSERT(m_frames[i].normals != NULL);

		fread(&scale.x, 4, 1, fp);
		fread(&scale.y, 4, 1, fp);
		fread(&scale.z, 4, 1, fp);

		fread(&translate.x, 4, 1, fp);
		fread(&translate.y, 4, 1, fp);
		fread(&translate.z, 4, 1, fp);

		// Store the text name of the frame (we won't waste the full 16 characters
		// reserved in the file here)
		for (int j = 0; j < MD2_FRAME_NAME_LENGTH; ++j)
		{
			fread(&c, 1, 1, fp);
			if (!c)
			{
				fseek(fp, MD2_FRAME_NAME_LENGTH - j - 1, SEEK_CUR);
				break;
			}
			else
				m_frames[i].name += c;
		}

		// Read vertices, and decompress as we load them for performance when rendering
		for (int j = 0; j < header.numVertices; ++j)
		{
			fread(&c, 1, 1, fp);
			x = (float)c;
			fread(&c, 1, 1, fp);
			y = (float)c;
			fread(&c, 1, 1, fp);
			z = (float)c;

			// Convert to OpenGL's coordinate system, otherwise models will need to be rotated to be drawn upright
			m_frames[i].vertices[j].x = (x * scale.x) + translate.x;
			m_frames[i].vertices[j].y = (z * scale.z) + translate.z;
			m_frames[i].vertices[j].z = -1.0f * ((y * scale.y) + translate.y);

			fread(&c, 1, 1, fp);		// Dummy command to increment file pointer (we don't care about the normal index)
		}

		//m_frameMap[m_frames[i].name] = i;
	}

	fclose(fp);

	// Cleanup and finishing touches.
	// Vertex coordinates, as of now, are waaay out of range (most likely, unless the model is tiny).
	// We could've scaled them down above while reading them in, but I noticed issues calculating normals
	// when that was done (probably due to lacking precision). So, we calculate the normals using the 
	// un-touched coordinates (get the most accurate normal calc that way), then scale the vertex down.
	for (int i = 0; i < header.numFrames; ++i)
	{
		// Calculate vertex normals
		Vector3 sumNormal;
		int sum;
		for (int j = 0; j < header.numVertices; ++j)
		{
			sum = 0;
			sumNormal = Vector3(0, 0, 0);
			for (int k = 0; k < header.numPolys; ++k)
			{
				if (m_polys[k].vertex[0] == j || m_polys[k].vertex[1] == j || m_polys[k].vertex[2] == j)
				{
					++sum;
					sumNormal += Vector3::SurfaceNormal(m_frames[i].vertices[m_polys[k].vertex[0]], 
						m_frames[i].vertices[m_polys[k].vertex[1]], 
						m_frames[i].vertices[m_polys[k].vertex[2]]);
				}
			}
			m_frames[i].normals[j] = sumNormal / (float)sum;
		}

		//// Done, now scale the vertices down
		//for (int j = 0; j < header.numVertices; ++j)
		//{
		//	m_frames[i].vertices[j] /= (float)(MD2_SCALE_FACTOR);
		//}
	}

	// check for an animation definition file
	std::string animationFile = file;
	animationFile.erase(animationFile.find_last_of('.', std::string::npos));
	animationFile.append(".animations");

	fp = fopen(animationFile.c_str(), "r");
	if (fp != NULL)
	{
		char *buffer = new char[80];
		std::string line;
		std::string name;
		std::string temp;
		int start;
		int end;

		while (!feof(fp))
		{
			fgets(buffer, 80, fp);
			line = buffer;

			if (strlen(buffer) > 5)		// minimum length for a viable frame definition
			{
				// get animation name
				int nameEnd = line.find_first_of(',');
				if (nameEnd == std::string::npos)
					continue;
				name = line.substr(0, nameEnd);

				// get start frame index
				int startEnd = line.find_first_of(',', nameEnd + 1);
				if (startEnd == std::string::npos)
					continue;
				temp = line.substr(nameEnd + 1, startEnd);
				start = atoi(temp.c_str());

				// get end frame index
				temp = line.substr(startEnd + 1, std::string::npos);
				end = atoi(temp.c_str());

				Md2Animation *animation = new Md2Animation();
				animation->name = name;
				animation->startFrame = start;
				animation->endFrame = end;
				m_animations.push_back(*animation);
			}
		}
		delete[] buffer;

		fclose(fp);
	}

	return true;
}

bool Md2::ConvertToMesh(const std::string &file)
{
	FILE *fp = fopen(file.c_str(), "wb");

	fputs("MESH", fp);
	unsigned char version = 1;
	fwrite(&version, 1, 1, fp);

	// keyframes chunk
	fputs("KFR", fp);
	long numFrames = m_numFrames;
	long sizeofFrames = (sizeof(float) * 3 * 2) * numFrames + sizeof(long);
	fwrite(&sizeofFrames, sizeof(long), 1, fp);
	fwrite(&numFrames, sizeof(long), 1, fp);
	for (long i = 0; i < numFrames; ++i)
	{
		// vertices
		for (int j = 0; j < m_numVertices; ++j)
		{
			const Vector3 *vertex = &m_frames[i].vertices[j];
			fwrite(&vertex->x, sizeof(float), 1, fp);
			fwrite(&vertex->y, sizeof(float), 1, fp);
			fwrite(&vertex->z, sizeof(float), 1, fp);
		}

		// normals
		for (int j = 0; j < m_numVertices; ++j)
		{
			const Vector3 *normal = &m_frames[i].normals[j];
			fwrite(&normal->x, sizeof(float), 1, fp);
			fwrite(&normal->y, sizeof(float), 1, fp);
			fwrite(&normal->z, sizeof(float), 1, fp);
		}
	}

	// textures chunk
	fputs("KTX", fp);
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

	// triangles chunk
	fputs("KTR", fp);
	long numPolys = m_numPolys;
	long sizeofPolys = (sizeof(long) * 3 * 2) * numPolys + sizeof(long);
	fwrite(&sizeofPolys, sizeof(long), 1, fp);
	fwrite(&numPolys, sizeof(long), 1, fp);
	for (long i = 0; i < numPolys; ++i)
	{
		long data;

		// vertex indices
		data = m_polys[i].vertex[0];
		fwrite(&data, sizeof(long), 1, fp);
		data = m_polys[i].vertex[1];
		fwrite(&data, sizeof(long), 1, fp);
		data = m_polys[i].vertex[2];
		fwrite(&data, sizeof(long), 1, fp);

		// tex coord indices
		data = m_polys[i].texCoord[0];
		fwrite(&data, sizeof(long), 1, fp);
		data = m_polys[i].texCoord[1];
		fwrite(&data, sizeof(long), 1, fp);
		data = m_polys[i].texCoord[2];
		fwrite(&data, sizeof(long), 1, fp);
	}

	if (m_animations.size() > 0)
	{
		// figure out the size of all the animation name strings
		long sizeofNames = 0;
		for (int i = 0; i < m_animations.size(); ++i)
			sizeofNames += m_animations[i].name.length() + 1;

		// animations chunk
		fputs("ANI", fp);
		long numAnimations = m_animations.size();
		long sizeofAnimations = (sizeof(long) * 2) * numAnimations + sizeofNames + sizeof(long);
		fwrite(&sizeofAnimations, sizeof(long), 1, fp);
		fwrite(&numAnimations, sizeof(long), 1, fp);
		for (long i = 0; i < numAnimations; ++i)
		{
			long data;
			const Md2Animation *animation = &m_animations[i];
			//fputs(animation->name.c_str(), fp);
			fputs(animation->name.c_str(), fp);
			fwrite("\0", 1, 1, fp);
			data = animation->startFrame;
			fwrite(&data, sizeof(long), 1, fp);
			data = animation->endFrame;
			fwrite(&data, sizeof(long), 1, fp);
		}
	}

	fclose(fp);
	return true;
}
