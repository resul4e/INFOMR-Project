#include "ScatterplotView.h"

#include <vector>

#include <QSize>
#include <QDebug>
#include <cmath>

namespace
{
    Bounds2D getDataBounds(const std::vector<glm::vec2>& points)
    {
        Bounds2D bounds = Bounds2D::Max;

        for (const glm::vec2& point : points)
        {
            bounds.setLeft(std::min(point.x, bounds.getLeft()));
            bounds.setRight(std::max(point.x, bounds.getRight()));
            bounds.setBottom(std::min(point.y, bounds.getBottom()));
            bounds.setTop(std::max(point.y, bounds.getTop()));
        }

        return bounds;
    }
}

ScatterplotWidget::ScatterplotWidget()
{

}

bool ScatterplotWidget::isInitialized()
{
    return _isInitialized;
}

void ScatterplotWidget::setRenderMode(RenderMode renderMode)
{
    _renderMode = renderMode;

    update();
}

void ScatterplotWidget::renderModePicked(const int index)
{
    switch (index)
    {
    case 0: setRenderMode(ScatterplotWidget::RenderMode::SCATTERPLOT); break;
    case 1: setRenderMode(ScatterplotWidget::RenderMode::DENSITY); break;
    case 2: setRenderMode(ScatterplotWidget::RenderMode::LANDSCAPE); break;
    }
    qDebug() << "Render Mode Picked";
}

void ScatterplotWidget::pointSizeChanged(const int size)
{
    _pointRenderer.setPointSize(size / 1000.0f);
    update();
}

void ScatterplotWidget::pointOpacityChanged(const int opacity)
{
    _pointRenderer.setAlpha(opacity / 100.0f);
    update();
}

// Positions need to be passed as a pointer as we need to store them locally in order
// to be able to find the subset of data that's part of a selection. If passed
// by reference then we can upload the data to the GPU, but not store it in the widget.
void ScatterplotWidget::setData(const std::vector<glm::vec2>* points)
{
    Bounds2D bounds = getDataBounds(*points);
    bounds.ensureMinimumSize(1e-07f, 1e-07f);
    bounds.makeSquare();
    bounds.expand(0.1f);
    _dataBounds = bounds;

    // Pass bounds and data to renderer
    _pointRenderer.setBounds(_dataBounds);
    _pointRenderer.setData(*points);

    update();
}

void ScatterplotWidget::setHighlights(const std::vector<char>& highlights)
{
    _pointRenderer.setHighlights(highlights);

    update();
}

void ScatterplotWidget::setScalars(const std::vector<float>& scalars)
{
    _pointRenderer.setScalars(scalars);
    
    update();
}

void ScatterplotWidget::setColors(const std::vector<glm::vec3>& colors)
{
    _pointRenderer.setColors(colors);
    _pointRenderer.setScalarEffect(None);

    update();
}

void ScatterplotWidget::setPointSize(const float size)
{
    _pointRenderer.setPointSize(size);

    update();
}

void ScatterplotWidget::setAlpha(const float alpha)
{
    _pointRenderer.setAlpha(alpha);
}

void ScatterplotWidget::setPointScaling(PointScaling scalingMode)
{
    _pointRenderer.setPointScaling(scalingMode);
}

void ScatterplotWidget::setScalarEffect(PointEffect effect)
{
    _pointRenderer.setScalarEffect(effect);
}

void ScatterplotWidget::initializeGL()
{
    initializeOpenGLFunctions();
    qDebug() << "Initializing scatterplot widget with context: " << context();
    std::string versionString = std::string((const char*) glGetString(GL_VERSION));
    qDebug() << versionString.c_str();

    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &ScatterplotWidget::cleanup);

    _pointRenderer.init();

    // Set a default color map for both renderers
    _pointRenderer.setScalarEffect(PointEffect::Color);

    _isInitialized = true;
    emit initialized();
}

void ScatterplotWidget::resizeGL(int w, int h)
{
    _windowSize.setWidth(w);
    _windowSize.setHeight(h);

    _pointRenderer.resize(QSize(w, h));

    // Set matrix for normalizing from pixel coordinates to [0, 1]
    toNormalisedCoordinates = glm::mat3(1.0f / w, 0.0f, 0.0f, 0.0f, 1.0f / h, 0.0f, 0.0f, 0.0f, 1.0f);

    // Take the smallest dimensions in order to calculate the aspect ratio
    int size = w < h ? w : h;

    float wAspect = (float)w / size;
    float hAspect = (float)h / size;
    float wDiff = ((wAspect - 1) / 2.0);
    float hDiff = ((hAspect - 1) / 2.0);

    
    toIsotropicCoordinates = glm::mat3(wAspect, 0.0f, 0.0f, 0.0f, hAspect, 0.0f, -wDiff, -hDiff, 1.0f);
}

void ScatterplotWidget::paintGL()
{
    // Bind the framebuffer belonging to the widget
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    // Clear the widget to the background color
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset the blending function
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    switch (_renderMode)
    {
    case SCATTERPLOT: _pointRenderer.render(); break;
    }
}

void ScatterplotWidget::mousePressEvent(QMouseEvent *event)
{
 //   _selecting = true;

	//glm::vec2 point = toNormalisedCoordinates * glm::vec2(event->x(), _windowSize.height() - event->y());
 //   _selection.setStart(point);
}

void ScatterplotWidget::mouseMoveEvent(QMouseEvent *event)
{
 //   if (!_selecting) return;

	//glm::vec2 point = toNormalisedCoordinates * glm::vec2(event->x(), _windowSize.height() - event->y());
 //   _selection.setEnd(point);

 //   onSelecting(_selection);

    update();
}

void ScatterplotWidget::mouseReleaseEvent(QMouseEvent *event)
{
 //   _selecting = false;

	//glm::vec2 point = toNormalisedCoordinates * glm::vec2(event->x(), _windowSize.height() - event->y());
 //   _selection.setEnd(point);

 //   onSelection(_selection);

    update();
}

void ScatterplotWidget::cleanup()
{
    qDebug() << "Deleting scatterplot widget, performing clean up...";
    _isInitialized = false;

    makeCurrent();
    _pointRenderer.destroy();
}

ScatterplotWidget::~ScatterplotWidget()
{
    disconnect(QOpenGLWidget::context(), &QOpenGLContext::aboutToBeDestroyed, this, &ScatterplotWidget::cleanup);
    cleanup();
}
