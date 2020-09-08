#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.inl"

#include <QString>

#include <unordered_map>
#include <string>

class ShaderProgram : protected QOpenGLFunctions_3_3_Core
{
public:
	ShaderProgram();
	~ShaderProgram() override;
	void bind();
	void release();
	void destroy();

	void uniform1i(const char* name, int value);
	void uniform1iv(const char* name, int count, int* values);
	void uniform2i(const char* name, int v0, int v1);
	void uniform1f(const char* name, float value);
	void uniform2f(const char* name, float v0, float v1);
	void uniform3f(const char* name, float v0, float v1, float v2);
	void uniform3f(const char* name, glm::fvec3 v);
	void uniform3fv(const char* name, int count, glm::fvec3* v);
	void uniform4f(const char* name, float v0, float v1, float v2, float v3);
	void uniformMatrix3f(const char* name, glm::mat3& m);
	void uniformMatrix4f(const char* name, glm::mat4& m);

	bool loadShaderFromFile(QString vertPath, QString fragPath);
private:
	int location(const char* uniform);

	GLuint _handle;
	std::unordered_map<std::string, int> _locationMap;
};
