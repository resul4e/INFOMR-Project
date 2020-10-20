#pragma once

#include "OffscreenContext.h"
#include "Graphics/Shader.h"

struct ModelDescriptor;
class Camera;

class Projector : public OffscreenContext
{
public:
	Projector();
	void initialize();
	void render(ModelDescriptor& _modelDescriptor);
	void cleanup();

private:
	std::shared_ptr<Camera> m_camera;
	ShaderProgram m_modelShader;

	GLuint m_fboImage;
	GLuint m_fbo;
	unsigned int m_imageDim;
};
