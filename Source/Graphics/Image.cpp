#include "Image.h"

Image::Image(unsigned int width, unsigned int height, unsigned int comp) :
	m_width(width),
	m_height(height),
	m_comp(comp)
{
	m_data = nullptr;// new unsigned char[width * height * comp];
}

void Image::setData(unsigned char* data)
{
	m_data = data;
}
