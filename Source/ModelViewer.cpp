#include "ModelViewer.h"

#include "ModelLoader.h"
#include "Model.h"

#include <QDebug>

void ModelViewer::initializeGL()
{
    initializeOpenGLFunctions();
    qDebug() << "Initializing model loader with context: " << context();
    std::string versionString = std::string((const char*)glGetString(GL_VERSION));
    qDebug() << versionString.c_str();

    _model = ModelLoader::LoadModel("../Resources/Cube.obj");
    _model->Upload();
    //connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &ModelViewer::cleanup);
}

void ModelViewer::resizeGL(int w, int h)
{
    qDebug() << "Resize" << w << h;
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

    for (Mesh& mesh : _model->m_meshes)
    {
        glBindVertexArray(mesh.vao);
        glDrawArrays(GL_TRIANGLES, 0, mesh.faces.size() * 3);
    }
}
