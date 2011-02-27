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

				Ms3dAnimation *animation = new Ms3dAnimation();
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

bool Ms3d::ConvertToMesh(const std::string &file)
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
	long sizeOfVertices = ((sizeof(float) * 3)) * numVertices + sizeof(long);
	fwrite(&sizeOfVertices, sizeof(long), 1, fp);
	fwrite(&numVertices, sizeof(long), 1, fp);
	for (long i = 0; i < numVertices; ++i)
	{
		Ms3dVertex *vertex = &m_vertices[i];
		fwrite(&vertex->vertex.x, sizeof(float), 1, fp);
		fwrite(&vertex->vertex.y, sizeof(float), 1, fp);
		fwrite(&vertex->vertex.z, sizeof(float), 1, fp);
	}

	// triangles chunk
	fputs("TRI", fp);
	long numTriangles = m_numTriangles;
	long sizeOfTriangles = (sizeof(int) * 4 + (sizeof(float) * 3) * 3 + (sizeof(float) * 2) * 3) * numTriangles + sizeof(long);
	fwrite(&sizeOfTriangles, sizeof(long), 1, fp);
	fwrite(&numTriangles, sizeof(long), 1, fp);
	for (long i = 0; i < numTriangles; ++i)
	{
		Ms3dTriangle *triangle = &m_triangles[i];
		int index = triangle->vertices[0];
		fwrite(&index, sizeof(int), 1, fp);
		index = triangle->vertices[1];
		fwrite(&index, sizeof(int), 1, fp);
		index = triangle->vertices[2];
		fwrite(&index, sizeof(int), 1, fp);

		index = triangle->meshIndex;
		fwrite(&index, sizeof(int), 1, fp);

		for (int j = 0; j < 3; ++j)
		{
			fwrite(&triangle->normals[j].x, sizeof(float), 1, fp);
			fwrite(&triangle->normals[j].y, sizeof(float), 1, fp);
			fwrite(&triangle->normals[j].z, sizeof(float), 1, fp);
		}
		for (int j = 0; j < 3; ++j)
		{
			fwrite(&triangle->texCoords[j].x, sizeof(float), 1, fp);
			fwrite(&triangle->texCoords[j].y, sizeof(float), 1, fp);
		}
	}

	// sub-meshes / groups chunk
	fputs("GRP", fp);
	long numGroups = m_numMeshes;
	long sizeOfGroupNames = 0;
	for (long i = 0; i < numGroups; ++i)
		sizeOfGroupNames += (m_meshes[i].name.length() + 1);
	long sizeOfGroups = sizeOfGroupNames + (sizeof(int)) * numGroups + sizeof(long);
	fwrite(&sizeOfGroups, sizeof(long), 1, fp);
	fwrite(&numGroups, sizeof(long), 1, fp);
	for (long i = 0; i < numGroups; ++i)
	{
		Ms3dMesh *mesh = &m_meshes[i];
		fwrite(mesh->name.c_str(), mesh->name.length(), 1, fp);
		char c = '\0';
		fwrite(&c, 1, 1, fp);
		int numTriangles = mesh->numTriangles;
		fwrite(&numTriangles, sizeof(int), 1, fp);
	}

	// joints chunk
	fputs("JNT", fp);
	long numJoints = m_numJoints;
	long sizeOfJointNames = 0;
	for (long i = 0; i < numJoints; ++i)
		sizeOfJointNames += (m_joints[i].name.length() + 1);
	long sizeOfJoints = sizeOfJointNames + (sizeof(int) + sizeof(float) * 3 + sizeof(float) * 3) * numJoints + sizeof(long);
	fwrite(&sizeOfJoints, sizeof(long), 1, fp);
	fwrite(&numJoints, sizeof(long), 1, fp);
	for (long i = 0; i < numJoints; ++i)
	{
		Ms3dJoint *joint = &m_joints[i];
		fwrite(joint->name.c_str(), joint->name.length(), 1, fp);
		char c = '\0';
		fwrite(&c, 1, 1, fp);
		int parentIndex = FindIndexOfJoint(joint->parentName);
		fwrite(&parentIndex, sizeof(int), 1, fp);
		fwrite(&joint->position.x, sizeof(float), 1, fp);
		fwrite(&joint->position.y, sizeof(float), 1, fp);
		fwrite(&joint->position.z, sizeof(float), 1, fp);
		fwrite(&joint->rotation.x, sizeof(float), 1, fp);
		fwrite(&joint->rotation.y, sizeof(float), 1, fp);
		fwrite(&joint->rotation.z, sizeof(float), 1, fp);
	}

	// joints to vertices mapping chunk
	fputs("JTV", fp);
	long numMappings = numVertices;
	long sizeOfJointMappings = (sizeof(int) + sizeof(float)) * numMappings + sizeof(long);
	fwrite(&sizeOfJointMappings, sizeof(long), 1, fp);
	fwrite(&numMappings, sizeof(long), 1, fp);
	for (long i = 0; i < numMappings; ++i)
	{
		Ms3dVertex *vertex = &m_vertices[i];
		int jointIndex = vertex->jointIndex;
		fwrite(&jointIndex, sizeof(int), 1, fp);
		float weight = 1.0f;
		fwrite(&weight, sizeof(float), 1, fp);
	}

	// joint animation keyframes
	fputs("JKF", fp);
	long numFrames = m_numFrames;
	long sizeOfJointFrames = ((6 * sizeof(float)) * m_numJoints) * numFrames + sizeof(long);
	fwrite(&sizeOfJointFrames, sizeof(long), 1, fp);
	fwrite(&numFrames, sizeof(long), 1, fp);
	for (long i = 0; i < numFrames; ++i)
	{
		for (int j = 0; j < m_numJoints; ++j)
		{
			Ms3dJoint *joint = &m_joints[j];
			Ms3dKeyFrame *position;
			Ms3dKeyFrame *rotation;

			if (i >= joint->numTranslationFrames)
				position = &joint->translationFrames[0];
			else
				position = &joint->translationFrames[i];
			if (i >= joint->numRotationFrames)
				rotation = &joint->rotationFrames[0];
			else
				rotation = &joint->rotationFrames[i];

			fwrite(&position->param.x, sizeof(float), 1, fp);
			fwrite(&position->param.y, sizeof(float), 1, fp);
			fwrite(&position->param.z, sizeof(float), 1, fp);
			fwrite(&rotation->param.x, sizeof(float), 1, fp);
			fwrite(&rotation->param.y, sizeof(float), 1, fp);
			fwrite(&rotation->param.z, sizeof(float), 1, fp);
		}
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
			const Ms3dAnimation *animation = &m_animations[i];
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

int Ms3d::FindIndexOfJoint(const std::string &jointName)
{
	if (jointName.length() == 0)
		return -1;

	for (int i = 0; i < m_numJoints; ++i)
	{
		Ms3dJoint *joint = &m_joints[i];
		if (joint->name == jointName)
			return i;
	}

	return -1;
}
