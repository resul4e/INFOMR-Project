#include "ModelViewer.h"

#include "ModelLoader.h"
#include "Model.h"
#include "Camera.h"

#include <QDebug>

void ModelViewer::initializeGL()
{
    initializeOpenGLFunctions();
    qDebug() << "Initializing model loader with context: " << context();
    std::string versionString = std::string((const char*)glGetString(GL_VERSION));
    qDebug() << versionString.c_str();

    _model = ModelLoader::LoadModel("../Resources/Cube.obj");
    _model->Upload();
    m_camera = std::make_shared<Camera>(60, 1, 0.1f, 100.0f);
    m_camera->loadProjectionMatrix(m_projMatrix);

    m_modelShader.loadShaderFromFile("../Resources/Model.vert", "../Resources/Model.frag");
    //connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &ModelViewer::cleanup);
}

void ModelViewer::resizeGL(int w, int h)
{
    qDebug() << "Resize" << w << h;
    m_camera->SetAspectRatio((float) w / h);
    m_camera->loadProjectionMatrix(m_projMatrix);
    //_windowSize.setWidth(w);
    //_windowSize.setHeight(h);
}

void ModelViewer::paintGL()
{
    // Bind the framebuffer belonging to the widget
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    // Clear the widget to the background color
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset the blending function
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_modelShader.bind();
    m_modelShader.uniformMatrix4f("projMatrix", m_projMatrix);

    for (Mesh& mesh : _model->m_meshes)
    {
        glBindVertexArray(mesh.vao);
        glDrawArrays(GL_TRIANGLES, 0, mesh.faces.size() * 3);
    }

    m_modelShader.release();
}
