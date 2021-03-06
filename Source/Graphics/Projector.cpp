#include "Projector.h"

#include "ModelDescriptor.h"
#include "Camera.h"
#include "Graphics/Image.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <QDebug>
#include <QImage>

Projector::Projector() :
	OffscreenContext(),
	m_fbo(0),
	m_fboImage(0),
	m_imageDim(512)
{

}

void Projector::initialize()
{
	bindContext();

	m_camera = std::make_shared<Camera>(glm::radians(60.0f), 1, 0.1f, 100.0f);
	bool success = m_modelShader.loadShaderFromFile("Resources/Model.vert", "Resources/Black.frag");
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
	glm::mat4 projMatrix = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	glm::mat4 viewMatrix = glm::mat4(1.0f);

	// Bind shader
	ShaderProgram& shader = m_modelShader;
	shader.bind();
	shader.uniformMatrix4f("projMatrix", projMatrix);
	shader.uniformMatrix4f("viewMatrix", viewMatrix);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glm::mat4 frontView = glm::mat4(1.0f);
	glm::mat4 sideView = glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), glm::vec3(0, 1, 0));
	glm::mat4 topView = glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), glm::vec3(1, 0, 0));
	std::vector<glm::mat4> cameraViews = { frontView, sideView, topView };

	for (int i = 0; i < 3; i++)
	{
		glm::mat4 modelMatrix = cameraViews[i];
		shader.uniformMatrix4f("modelMatrix", modelMatrix);

		// Clear the widget to the background color
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw model
		for (Mesh& mesh : modelCopy.m_meshes)
		{
			glBindVertexArray(mesh.vao);
			glDrawArrays(GL_TRIANGLES, 0, mesh.faces.size() * 3);
		}

		// Write framebuffer image to file
		unsigned char* buff = new unsigned char[(size_t)m_imageDim * m_imageDim * 3];

		// Now, get pixels.
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(0, 0, m_imageDim, m_imageDim, GL_RGB, GL_UNSIGNED_BYTE, buff);

		Image image(m_imageDim, m_imageDim, 3);
		image.SetData(buff);

		//QImage fboImage(buff, m_imageDim, m_imageDim, QImage::Format_RGB888);
		//fboImage.save(QString("beep") + QString::number(i) + QString(".png"));

		_modelDescriptor.m_projections.push_back(image);
	}
	
	shader.release();

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
