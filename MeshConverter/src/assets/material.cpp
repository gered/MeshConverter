#include "material.h"

Material::Material(unsigned long ambient, unsigned long diffuse, unsigned long specular, unsigned long emission, std::string texture)
{
	m_ambient = ambient;
	m_diffuse = diffuse;
	m_specular = specular;
	m_emission = emission;
	m_texture = texture;
}

Material::~Material()
{
}
