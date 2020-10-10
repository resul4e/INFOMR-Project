#include "Projector.h"

#include "ModelDescriptor.h"
#include "Camera.h"
#include "Graphics/Image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <QDebug>
#include <QImage>

OffscreenBuffer::OffscreenBuffer()
{
	requestedFormat().setVersion(3, 3);
	setFormat(requestedFormat());
	create();

	m_context = new QOpenGLContext(this);
	m_context->setFormat(format());

	if (m_context->create())
	{
		m_context->makeCurrent(this);
		m_context->functions()->initializeOpenGLFunctions();
	}
	else
	{
		delete m_context;
		m_context = Q_NULLPTR;
		throw("Failed to create OpenGL context for OffscreenBuffer");
	}
	if (!m_context->isValid())
	{
		throw("OffscreenBuffer OpenGL context is not valid");
	}
}

void OffscreenBuffer::bindContext()
{
	m_context->makeCurrent(this);
}

void OffscreenBuffer::releaseContext()
{
	m_context->doneCurrent();
}

Projector::Projector() :
	OffscreenBuffer(),
	m_fbo(0),
	m_fboImage(0),
	m_imageDim(512)
{

}

void Projector::initialize()
{
	bindContext();
	initializeOpenGLFunctions();

	m_camera = std::make_shared<Camera>(glm::radians(60.0f), 1, 0.1f, 100.0f);
	bool success = m_modelShader.loadShaderFromFile("../Resources/Model.vert", "../Resources/Black.frag");
	if (!success)
		qDebug() << "Failed to load Projector shader";

	glGenTextures(1, &m_fboImage);
	glBindTexture(GL_TEXTURE_2D, m_fboImage);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_imageDim, m_imageDim, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboImage, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, m_context->defaultFramebufferObject());

	releaseContext();
}

void Projector::render(ModelDescriptor& _modelDescriptor)
{
	bindContext();
	initializeOpenGLFunctions();

	m_camera->RecomputePosition();
	glViewport(0, 0, m_imageDim, m_imageDim);
	// Set the background color to a light grey
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// Set rendering state
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// If a model is set, but has not been uploaded yet, do so
	//if (!model.isUploaded())
	Model modelCopy = *_modelDescriptor.m_model;
	modelCopy.Upload();

	// Load matrices
	glm::mat4 projMatrix = glm::ortho(-0.75f, 0.75f, -0.75f, 0.75f, -1.0f, 1.0f);

	glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
	//m_camera->LookAt(m_viewMatrix, m_camera->position, m_camera->center, glm::vec3(0, 1, 0));

	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

	// Bind shader
	ShaderProgram& shader = m_modelShader;
	shader.bind();
	shader.uniformMatrix4f("projMatrix", projMatrix);
	shader.uniformMatrix4f("viewMatrix", viewMatrix);
	shader.uniformMatrix4f("modelMatrix", modelMatrix);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	// Clear the widget to the background color
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw model
	for (Mesh& mesh : modelCopy.m_meshes)
	{
		glBindVertexArray(mesh.vao);
		glDrawArrays(GL_TRIANGLES, 0, mesh.faces.size() * 3);
	}

	shader.release();

	// Write framebuffer image to file
	unsigned char *buff = new unsigned char[(size_t) m_imageDim * m_imageDim * 3];

	// Now, get pixels.
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, m_imageDim, m_imageDim, GL_RGB, GL_UNSIGNED_BYTE, buff);

	QImage fboImage(buff, m_imageDim, m_imageDim, QImage::Format_RGB888);
	fboImage.save("beep.png");
	Image image(m_imageDim, m_imageDim, 3);
	image.setData(buff);
	_modelDescriptor.m_projections.push_back(image);

	glBindFramebuffer(GL_FRAMEBUFFER, m_context->defaultFramebufferObject());

	cleanup();
}

void Projector::cleanup()
{
	glDeleteTextures(1, &m_fboImage);
	glDeleteFramebuffers(1, &m_fbo);
	m_modelShader.destroy();
	releaseContext();
}
