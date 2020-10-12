#pragma once

#include "Graphics/Shader.h"

#include <QWindow>
#include <QOffscreenSurface>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLContext>

class ModelDescriptor;
class Camera;

class OffscreenBuffer : public QOffscreenSurface, public QOpenGLFunctions_3_3_Core
{
	Q_OBJECT
public:
	OffscreenBuffer();

	virtual void initialize() = 0;

protected:
	void bindContext();
	void releaseContext();

	QOpenGLContext* m_context;
};

class Projector : public OffscreenBuffer
{
public:
	Projector();
	void initialize() override;
	void render(ModelDescriptor& _modelDescriptor);
	void cleanup();

private:
	std::shared_ptr<Camera> m_camera;
	ShaderProgram m_modelShader;

	GLuint m_fboImage;
	GLuint m_fbo;
	unsigned int m_imageDim;
};
