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

ScatterplotView::ScatterplotView(Context& _context) :
	m_context(_context)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	connect(&m_context, &Context::embeddingChanged, this, &ScatterplotView::onEmbeddingChanged);
}

bool ScatterplotView::isInitialized()
{
	return _isInitialized;
}

void ScatterplotView::pointSizeChanged(const int size)
{
	_pointRenderer.setPointSize(size / 1000.0f);
	update();
}

void ScatterplotView::pointOpacityChanged(const int opacity)
{
	_pointRenderer.setAlpha(opacity / 100.0f);
	update();
}

void ScatterplotView::onEmbeddingChanged()
{
	setData(&m_context.GetEmbedding());
}
#include <random>
// Positions need to be passed as a pointer as we need to store them locally in order
// to be able to find the subset of data that's part of a selection. If passed
// by reference then we can upload the data to the GPU, but not store it in the widget.
void ScatterplotView::setData(const std::vector<glm::vec2>* points)
{
	Bounds2D bounds = getDataBounds(*points);
	bounds.ensureMinimumSize(1e-07f, 1e-07f);
	bounds.makeSquare();
	bounds.expand(0.1f);
	_dataBounds = bounds;

	// Compute colors
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

	std::vector<std::string> classes;
	std::vector<int> indices;
	for (ModelDescriptor& md : m_context.GetDatabase()->GetModelDatabase())
	{
		std::string c = md.m_class;
		bool found = false;
		for (int i = 0; i < classes.size(); i++)
		{
			if (classes[i] == c)
			{
				indices.push_back(i);
				found = true;
			}
		}
		if (!found)
		{
			classes.push_back(c);
			indices.push_back(classes.size() - 1);
		}
	}

	std::vector<glm::vec3> colorSet;
	for (int i = 0; i < classes.size(); i++)
	{
		float r = distribution(generator);
		float g = distribution(generator);
		float b = distribution(generator);
		colorSet.push_back(glm::vec3(r, g, b));
	}

	std::vector<glm::vec3> colors;
	for (int& index : indices)
	{
		colors.push_back(colorSet[index]);
	}
	setColors(colors);

	// Pass bounds and data to renderer
	_pointRenderer.setBounds(_dataBounds);
	_pointRenderer.setData(*points);
	pointSizeChanged(15);
	update();
}

void ScatterplotView::setHighlights(const std::vector<char>& highlights)
{
	_pointRenderer.setHighlights(highlights);

	update();
}

void ScatterplotView::setScalars(const std::vector<float>& scalars)
{
	_pointRenderer.setScalars(scalars);
	
	update();
}

void ScatterplotView::setColors(const std::vector<glm::vec3>& colors)
{
	_pointRenderer.setColors(colors);
	_pointRenderer.setScalarEffect(None);

	update();
}

void ScatterplotView::setPointSize(const float size)
{
	_pointRenderer.setPointSize(size);

	update();
}

void ScatterplotView::setAlpha(const float alpha)
{
	_pointRenderer.setAlpha(alpha);
}

void ScatterplotView::setPointScaling(PointScaling scalingMode)
{
	_pointRenderer.setPointScaling(scalingMode);
}

void ScatterplotView::setScalarEffect(PointEffect effect)
{
	_pointRenderer.setScalarEffect(effect);
}

void ScatterplotView::initializeGL()
{
	if (gladLoadGL()) qDebug() << "Good initialization of glad";
	qDebug() << "Initializing scatterplot widget with context: " << context();
	std::string versionString = std::string((const char*) glGetString(GL_VERSION));
	qDebug() << versionString.c_str();

	connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &ScatterplotView::cleanup);

	_pointRenderer.init();

	// Set a default color map for both renderers
	_pointRenderer.setScalarEffect(PointEffect::None);

	_isInitialized = true;
	emit initialized();
}

void ScatterplotView::resizeGL(int w, int h)
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

void ScatterplotView::paintGL()
{
	// Bind the framebuffer belonging to the widget
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

	// Clear the widget to the background color
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset the blending function
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	_pointRenderer.render();
}

void ScatterplotView::mousePressEvent(QMouseEvent *event)
{
 //   _selecting = true;

	//glm::vec2 point = toNormalisedCoordinates * glm::vec2(event->x(), _windowSize.height() - event->y());
 //   _selection.setStart(point);
}

void ScatterplotView::mouseMoveEvent(QMouseEvent *event)
{
 //   if (!_selecting) return;

	//glm::vec2 point = toNormalisedCoordinates * glm::vec2(event->x(), _windowSize.height() - event->y());
 //   _selection.setEnd(point);

 //   onSelecting(_selection);

	update();
}

void ScatterplotView::mouseReleaseEvent(QMouseEvent *event)
{
 //   _selecting = false;

	//glm::vec2 point = toNormalisedCoordinates * glm::vec2(event->x(), _windowSize.height() - event->y());
 //   _selection.setEnd(point);

 //   onSelection(_selection);

	update();
}

void ScatterplotView::cleanup()
{
	qDebug() << "Deleting scatterplot widget, performing clean up...";
	_isInitialized = false;

	makeCurrent();
	_pointRenderer.destroy();
}

ScatterplotView::~ScatterplotView()
{
	disconnect(QOpenGLWidget::context(), &QOpenGLContext::aboutToBeDestroyed, this, &ScatterplotView::cleanup);
	cleanup();
}
