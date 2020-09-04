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
    ModelViewer(QWidget* parent = 0)
    {
        //setStyleSheet("background-color:black;");
    }

protected:
    void initializeGL()         Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;

private:
    std::shared_ptr<Model> _model;
    std::shared_ptr<Camera> m_camera;
    glm::mat4 m_projMatrix;
    glm::mat4 m_viewMatrix;

    ShaderProgram m_modelShader;
};
