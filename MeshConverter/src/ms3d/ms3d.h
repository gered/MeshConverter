#ifndef __MS3D_H_INCLUDED__
#define __MS3D_H_INCLUDED__

#include <string>

class Ms3d
{
public:
	Ms3d();
	virtual ~Ms3d()                                        { Release(); }

	void Release();
	bool Load(const std::string &file);
	bool ConvertToMesh(const std::string &file);

};

#endif
