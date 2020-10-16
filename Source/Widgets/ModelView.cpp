#include "ModelView.h"

#include "ModelLoader.h"
#include "Model.h"
#include "Graphics/Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <QTimer>
#include <QWheelEvent>
#include <QDebug>
#include <iostream>
#include <cmath>

ModelView::ModelView(Context& _context, QWidget* parent) :
	m_context(_context),
	m_projMatrix(1.0f),
	m_viewMatrix(1.0f),
	m_modelMatrix(1.0f)
{
	//setStyleSheet("background-color:black;");
	connect(&m_context, &Context::modelChanged, this, &ModelView::onModelChanged);
}

void ModelView::onModelChanged()
{

}

void ModelView::initializeGL()
{
	initializeOpenGLFunctions();
	qDebug() << "Initializing model loader with context: " << context();
	std::string versionString = std::string((const char*)glGetString(GL_VERSION));
	qDebug() << versionString.c_str();

	m_planeModel = ModelLoader::LoadModel("../Resources/Plane.obj");
	m_planeModel->Upload();

	m_camera = std::make_shared<Camera>(glm::radians(60.0f), 1, 0.1f, 100.0f);
	m_camera->loadProjectionMatrix(m_projMatrix);
	m_camera->RecomputePosition();

	m_modelShader.loadShaderFromFile("../Resources/Model.vert", "../Resources/Model.frag");
	m_wireframeShader.loadShaderFromFile("../Resources/Model.vert", "../Resources/Color.frag");
	m_planeShader.loadShaderFromFile("../Resources/Model.vert", "../Resources/Plane.frag");
	//connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &ModelView::cleanup);

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	timer->start(10);

	// Set the background color to a light grey
	glClearColor(0.9f, 0.9f, 0.9f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0, -1.0);
}

void ModelView::resizeGL(int w, int h)
{
	m_camera->SetAspectRatio((float) w / h);
	m_camera->loadProjectionMatrix(m_projMatrix);
}

void ModelView::paintGL()
{
	const ModelDescriptor& modelDescriptor = m_context.GetActiveModel();

	// Bind the framebuffer belonging to the widget
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

	// Clear the widget to the background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// If no model is set, don't call any rendering functions
	if (!modelDescriptor.m_model)
		return;

	// If a model is set, but has not been uploaded yet, do so
	if (!modelDescriptor.m_model->isUploaded())
		modelDescriptor.m_model->Upload();

	// Reset the blending function
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_camera->LookAt(m_viewMatrix, m_camera->position, m_camera->center, glm::vec3(0, 1, 0));

	drawGroundPlane();
	drawModel();
	drawModel(true);
}

void ModelView::drawGroundPlane()
{
	m_modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, -0.5f, 0));
	m_modelMatrix = glm::scale(m_modelMatrix, glm::vec3(40, 1, 40));

	m_planeShader.bind();
	m_planeShader.uniformMatrix4f("projMatrix", m_projMatrix);
	m_planeShader.uniformMatrix4f("viewMatrix", m_viewMatrix);
	m_planeShader.uniformMatrix4f("modelMatrix", m_modelMatrix);

	for (Mesh& mesh : m_planeModel->m_meshes)
	{
		glBindVertexArray(mesh.vao);
		glDrawArrays(GL_TRIANGLES, 0, mesh.faces.size() * 3);
	}

	m_planeShader.release();
}

void ModelView::drawModel(bool wireframe)
{
	const ModelDescriptor& modelDescriptor = m_context.GetActiveModel();

	ShaderProgram& shader = wireframe ? m_wireframeShader : m_modelShader;

	m_modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

	shader.bind();
	shader.uniformMatrix4f("projMatrix", m_projMatrix);
	shader.uniformMatrix4f("viewMatrix", m_viewMatrix);
	shader.uniformMatrix4f("modelMatrix", m_modelMatrix);

	if (wireframe) {
		shader.uniform3f("u_Color", glm::vec3(1, 0, 1));
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	for (Mesh& mesh : modelDescriptor.m_model->m_meshes)
	{
		glBindVertexArray(mesh.vao);
		glDrawArrays(GL_TRIANGLES, 0, mesh.faces.size() * 3);
	}

	shader.release();

	// Reset polygon mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void ModelView::wheelEvent(QWheelEvent* event)
{
	float d = event->delta() < 0 ? 0.2f : -0.2f;

	m_distance += d;

	if (m_distance < 1) m_distance = 1;

	m_camera->distance += d;
	if (m_camera->distance < 1) m_camera->distance = 1;

	m_camera->RecomputePosition();
}

void ModelView::mouseMoveEvent(QMouseEvent* event)
{
	m_arcBall.Move(*m_camera, width(), height(), event->x(), event->y());
}

void ModelView::mousePressEvent(QMouseEvent* event)
{
	m_arcBall.Engage();
}

void ModelView::mouseReleaseEvent(QMouseEvent* event)
{
	m_arcBall.Release();
}
