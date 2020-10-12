#pragma once

#include <glm/fwd.hpp>

class Image
{
public:
	Image(unsigned int width, unsigned int height, unsigned int comp);

	unsigned char* getData();
	void SetData(unsigned char* data);

	int ComputeArea();
	int ComputePerimeter();
	float ComputeCompactness();
	void ComputeAABB(glm::ivec2& min, glm::ivec2& max);

private:
	unsigned int m_width, m_height, m_comp;
	unsigned char* m_data;
};
