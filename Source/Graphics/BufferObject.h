#pragma once

#include <QOpenGLFunctions_3_3_Core>

#include <vector>

class BufferObject : protected QOpenGLFunctions_3_3_Core
{
public:
	BufferObject();
	~BufferObject() override;

	void create();
	void bind();
	template<typename T>
	void setData(const std::vector<T>& data)
	{
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
	}

	void destroy();
private:
	GLuint _object;
};
