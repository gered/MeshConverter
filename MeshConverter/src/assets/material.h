#ifndef __MATERIAL_H_INCLUDED__
#define __MATERIAL_H_INCLUDED__

#include <string>

#define RGB_32(r, g, b, a)                                 ((b)|((g) << 8)|((r) << 16)|((a) << 24))
#define RGB_24(r, g, b)                                    ((b)|((g) << 8)|((r) << 16))
#define RGB_32_f(r, g, b, a)                               RGB_32((int)((r) * 255), (int)((g) * 255), (int)((b) * 255), (int)((a) * 255))
#define RGB_24_f(r, g, b)                                  RGB_24((int)((r) * 255), (int)((g) * 255), (int)((b) * 255))

class Material
{
public:
	Material(unsigned long ambient = 0, unsigned long diffuse = 0, unsigned long specular = 0, unsigned long emission = 0, std::string texture = "");
	virtual ~Material();

	void SetAmbient(unsigned long ambient)                 { m_ambient = ambient; }
	void SetDiffuse(unsigned long diffuse)                 { m_diffuse = diffuse; }
	void SetSpecular(unsigned long specular)               { m_specular = specular; }
	void SetEmission(unsigned long emission)               { m_emission = emission; }
	void SetTexture(std::string texture)                   { m_texture = texture; }
	unsigned long GetAmbient()                             { return m_ambient; }
	unsigned long GetDiffuse()                             { return m_diffuse; }
	unsigned long GetSpecular()                            { return m_specular; }
	unsigned long GetEmission()                            { return m_emission; }
	std::string GetTexture()                               { return m_texture; }

	static void ApplyDefault();
	static Material GetDefault();
	void Apply();

private:
	unsigned long m_ambient;
	unsigned long m_diffuse;
	unsigned long m_specular;
	unsigned long m_emission;
	std::string m_texture;
};

#endif