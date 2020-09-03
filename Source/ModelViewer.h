#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

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
    GLuint vao;
    GLuint vbo;
};
