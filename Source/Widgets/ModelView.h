#pragma once

#include "Context.h"

#include "Graphics/Shader.h"
#include "Graphics/ArcBall.h"

#include <glm/glm.hpp>

#include <QOpenGLWidget>

#include <memory>

class Model;
class Camera;

class ModelView : public QOpenGLWidget
{
	Q_OBJECT
public:
	ModelView(Context& _context, QWidget* parent = 0);

public slots:
	void onModelChanged();

protected:
	void initializeGL()         Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	void paintGL()              Q_DECL_OVERRIDE;

	void wheelEvent(QWheelEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

private:
	void drawGroundPlane();
	void drawModel(bool wireframe = false);

	Context& m_context;

	std::shared_ptr<Model> m_planeModel;
	std::shared_ptr<Camera> m_camera;
	ArcBall m_arcBall;
	glm::mat4 m_projMatrix;
	glm::mat4 m_viewMatrix;
	glm::mat4 m_modelMatrix;

	float m_distance = 3;

	ShaderProgram m_modelShader;
	ShaderProgram m_wireframeShader;
	ShaderProgram m_planeShader;
};
