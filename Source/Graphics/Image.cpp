#include "Image.h"

#include <glm/glm.hpp>
#include <cmath>

Image::Image(unsigned int width, unsigned int height, unsigned int comp) :
	m_width(width),
	m_height(height),
	m_comp(comp)
{
	m_data = nullptr;// new unsigned char[width * height * comp];
}

unsigned char* Image::getData()
{
	return m_data;
}

void Image::SetData(unsigned char* data)
{
	m_data = data;
}

int Image::ComputeArea()
{
	int numForegroundPixels = 0;
	for (int i = 0; i < m_width * m_height * m_comp; i += m_comp)
	{
		if (m_data[i] == 0) numForegroundPixels++;
	}

	return numForegroundPixels;
}

int Image::ComputePerimeter()
{
	int numPerimeterPixels = 0;
	for (int i = 0; i < m_width * m_height * m_comp; i += m_comp)
	{
		if (m_data[i] == 0)
		{
			if (m_data[i - m_comp] == 255 ||
				m_data[i + m_comp] == 255 ||
				m_data[i - m_width * m_comp] == 255 ||
				m_data[i + m_width * m_comp] == 255)
			{
				numPerimeterPixels++;
			}
		}
	}
	return numPerimeterPixels;
}

float Image::ComputeCompactness()
{
	int I = ComputePerimeter();
	int A = ComputeArea();

	return (float) pow(I, 2) / (4 * 3.141592f * A);
}

void Image::ComputeAABB(glm::ivec2& min, glm::ivec2& max)
{
	bool foundTop = false;
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			int index = y * m_width * m_comp + x * m_comp;

			if (m_data[index] == 0)
			{
				if (foundTop)
				{
					max.y = y;
				}
				if (!foundTop)
				{
					foundTop = true;
					min.y = y;
				}
			}
		}
	}

	bool foundLeft = false;
	for (int x = 0; x < m_width; x++)
	{
		for (int y = 0; y < m_height; y++)
		{
			int index = y * m_width * m_comp + x * m_comp;

			if (m_data[index] == 0)
			{
				if (foundLeft)
				{
					max.x = x;
				}
				if (!foundLeft)
				{
					foundLeft = true;
					min.x = x;
				}
			}
		}
	}
}
