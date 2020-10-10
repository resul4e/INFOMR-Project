#pragma once

class Image
{
public:
	Image(unsigned int width, unsigned int height, unsigned int comp);

	void setData(unsigned char* data);

private:
	unsigned int m_width, m_height, m_comp;
	unsigned char* m_data;
};
