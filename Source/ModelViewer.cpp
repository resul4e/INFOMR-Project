#include "ModelViewer.h"

#include "ModelLoader.h"
#include "Model.h"
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <QTimer>
#include <QWheelEvent>
#include <QDebug>
#include <iostream>
#include <cmath>

void ModelViewer::setModel(std::shared_ptr<Model> model)
{
    m_model = model;
}

std::shared_ptr<Model> ModelViewer::getModel() const
{
    return m_model;
}

void ModelViewer::initializeGL()
{
    initializeOpenGLFunctions();
    qDebug() << "Initializing model loader with context: " << context();
    std::string versionString = std::string((const char*)glGetString(GL_VERSION));
    qDebug() << versionString.c_str();

    m_planeModel = ModelLoader::LoadModel("../Resources/Plane.obj");
    m_planeModel->Upload();

    m_camera = std::make_shared<Camera>(glm::radians(60.0f), 1, 0.1f, 100.0f);
    m_camera->loadProjectionMatrix(m_projMatrix);

    m_modelShader.loadShaderFromFile("../Resources/Model.vert", "../Resources/Model.frag");
    m_planeShader.loadShaderFromFile("../Resources/Model.vert", "../Resources/Plane.frag");
    //connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &ModelViewer::cleanup);

    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(10);

    // Set the background color to a light grey
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);

    glEnable(GL_DEPTH_TEST);
}

void ModelViewer::resizeGL(int w, int h)
{
    m_camera->SetAspectRatio((float) w / h);
    m_camera->loadProjectionMatrix(m_projMatrix);

    //_windowSize.setWidth(w);
    //_windowSize.setHeight(h);
}
float t = 0;
void ModelViewer::paintGL()
{
    // Bind the framebuffer belonging to the widget
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    // Clear the widget to the background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // If no model is set, don't call any rendering functions
    if (!m_model)
        return;

    // If a model is set, but has not been uploaded yet, do so
    if (!m_model->isUploaded())
        m_model->Upload();

    // Reset the blending function
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    t += 0.01f;
    m_viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, -1, -m_distance));
    m_modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    m_modelMatrix = glm::rotate(m_modelMatrix, t, glm::vec3(0, 1, 0));

    m_modelShader.bind();
    m_modelShader.uniformMatrix4f("projMatrix", m_projMatrix);
    m_modelShader.uniformMatrix4f("viewMatrix", m_viewMatrix);
    m_modelShader.uniformMatrix4f("modelMatrix", m_modelMatrix);

    for (Mesh& mesh : m_model->m_meshes)
    {
        glBindVertexArray(mesh.vao);
        glDrawArrays(GL_TRIANGLES, 0, mesh.faces.size() * 3);
    }

    m_modelShader.release();

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

void ModelViewer::wheelEvent(QWheelEvent* event)
{
    float d = event->delta() < 0 ? 0.2f : -0.2f;

    m_distance += d;

    if (m_distance < 1) m_distance = 1;
}
