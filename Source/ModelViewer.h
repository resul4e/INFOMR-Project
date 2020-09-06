#pragma once

#include "Shader.h"

#include <glm/glm.hpp>

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

#include <memory>

class Model;
class Camera;

class ModelViewer : public QOpenGLWidget, QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    ModelViewer(QWidget* parent = 0) :
        m_model(nullptr),
        m_projMatrix(1.0f),
        m_viewMatrix(1.0f),
        m_modelMatrix(1.0f)
    {
        //setStyleSheet("background-color:black;");
    }

    void setModel(std::shared_ptr<Model> model);
    std::shared_ptr<Model> getModel() const;

protected:
    void initializeGL()         Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;

    void wheelEvent(QWheelEvent* event) override;

private:
    std::shared_ptr<Model> m_model;
    std::shared_ptr<Model> m_planeModel;
    std::shared_ptr<Camera> m_camera;
    glm::mat4 m_projMatrix;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_modelMatrix;

    float m_distance = 3;

    ShaderProgram m_modelShader;
    ShaderProgram m_planeShader;
};
