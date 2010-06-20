#include <stdio.h>
#include <string>
#include <exception>

#include "md2/md2.h"
#include "obj/obj.h"

int main(int argc, char **argv)
{
	printf("MESH Converter\n");

	if (argc == 1)
	{
		printf("No input file specified.\n");
		printf("Usage: meshconverter.exe [inputfile]\n\n");
		return 1;
	}

	std::string file = argv[1];
	std::string extension;

	try
	{
		extension = file.substr(file.find_last_of('.'), std::string::npos);
		for (int i = 0; i < extension.size(); ++i)
			extension[i] = tolower(extension[i]);
	}
	catch (std::exception &e)
	{
		extension = "";
	}

	std::string meshFile = "";
	if (extension.length() > 0)
	{
		meshFile = file;
		meshFile.erase(meshFile.find_last_of('.'), std::string::npos);
		meshFile.append(".mesh");
	}

	if (extension == ".obj")
	{
		Obj *obj = new Obj();
		if (!obj->Load(file, "./"))
		{
			printf("Error loading OBJ file.\n\n");
			return 1;
		}
		if (!obj->ConvertToMesh(meshFile))
		{
			printf("Error converting OBJ to MESH.\n\n");
			return 1;
		}
	}
	else if (extension == ".md2")
	{
		Md2 *md2 = new Md2();
		if (!md2->Load(file))
		{
			printf("Error loading MD2 file.\n\n");
			return 1;
		}
		if (!md2->ConvertToMesh(meshFile))
		{
			printf("Error converting MD2 to MESH.\n\n");
			return 1;
		}
	}
	else
	{
		printf("Unrecognized file type.\n\n");
		return 1;
	}

	return 0;

}