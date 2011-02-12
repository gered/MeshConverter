#include "ms3d.h"

#include <stdio.h>

#include "../util/files.h"

Ms3d::Ms3d()
{
	m_numVertices = 0;
	m_numTriangles = 0;
	m_numMeshes = 0;
	m_numMaterials = 0;
	m_numJoints = 0;
	m_vertices = NULL;
	m_triangles = NULL;
	m_meshes = NULL;
	m_materials = NULL;
	m_joints = NULL;
}

void Ms3d::Release()
{
	delete[] m_vertices;
	delete[] m_triangles;
	delete[] m_meshes;
	delete[] m_materials;
	delete[] m_joints;
	m_numVertices = 0;
	m_numTriangles = 0;
	m_numMeshes = 0;
	m_numMaterials = 0;
	m_numJoints = 0;
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
		ReadString(fp, mesh->name, 32);
		fread(&mesh->numTriangles, 2, 1, fp);
		mesh->triangles = new unsigned short[mesh->numTriangles];
		for (int j = 0; j < mesh->numTriangles; ++j)
			fread(&mesh->triangles[j], 2, 1, fp);
		fread(&mesh->materialIndex, 1, 1, fp);
	}

	// read material information
	fread(&m_numMaterials, 2, 1, fp);
	if (m_numMaterials > 0)
	{
		m_materials = new Ms3dMaterial[m_numMaterials];

		for (int i = 0; i < m_numMaterials; ++i)
		{
			Ms3dMaterial *material = &m_materials[i];

			ReadString(fp, material->name, 32);
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
			ReadString(fp, material->texture, 128);
			ReadString(fp, material->alpha, 128);
		}
	}

	// read joints
	fread(&m_animationFps, 4, 1, fp);
	fread(&m_editorAnimationTime, 4, 1, fp);
	fread(&m_numFrames, 4, 1, fp);
	fread(&m_numJoints, 2, 1, fp);
	if (m_numJoints > 0)
	{
		m_joints = new Ms3dJoint[m_numJoints];

		for (int i = 0; i < m_numJoints; ++i)
		{
			Ms3dJoint *joint = &m_joints[i];

			fread(&joint->editorFlags, 1, 1, fp);
			ReadString(fp, joint->name, 32);
			ReadString(fp, joint->parentName, 32);
			fread(&joint->rotation.x, 4, 1, fp);
			fread(&joint->rotation.y, 4, 1, fp);
			fread(&joint->rotation.z, 4, 1, fp);
			fread(&joint->position.x, 4, 1, fp);
			fread(&joint->position.y, 4, 1, fp);
			fread(&joint->position.z, 4, 1, fp);
			fread(&joint->numRotationFrames, 2, 1, fp);
			fread(&joint->numTranslationFrames, 2, 1, fp);
			joint->rotationFrames = new Ms3dKeyFrame[joint->numRotationFrames];
			for (int j = 0; j < joint->numRotationFrames; ++j)
			{
				Ms3dKeyFrame *frame = &joint->rotationFrames[j];
				fread(&frame->time, 4, 1, fp);
				fread(&frame->param.x, 4, 1, fp);
				fread(&frame->param.y, 4, 1, fp);
				fread(&frame->param.z, 4, 1, fp);
			}
			joint->translationFrames = new Ms3dKeyFrame[joint->numTranslationFrames];
			for (int j = 0; j < joint->numTranslationFrames; ++j)
			{
				Ms3dKeyFrame *frame = &joint->translationFrames[j];
				fread(&frame->time, 4, 1, fp);
				fread(&frame->param.x, 4, 1, fp);
				fread(&frame->param.y, 4, 1, fp);
				fread(&frame->param.z, 4, 1, fp);
			}
		}
	}

	fclose(fp);

	return true;
}

bool Ms3d::ConvertToMesh(const std::string &file)
{
	return false;
}
