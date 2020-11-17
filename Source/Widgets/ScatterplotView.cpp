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

	template <typename T1, typename T2>
	struct greater_second {
		typedef std::pair<T1, T2> type;
		bool operator ()(type const& a, type const& b) const {
			return a.second > b.second;
		}
	};

	glm::vec3 hexToVec(std::string hexCode)
	{
		float r = (strtol(hexCode.c_str(), nullptr, 16) >> 16 & 0x000000FF) / 255.0f;
		float g = (strtol(&hexCode.c_str()[2], nullptr, 16) >> 8 & 0x000000FF) / 255.0f;
		float b = (strtol(&hexCode.c_str()[4], nullptr, 16) >> 0 & 0x000000FF) / 255.0f;

		return glm::vec3(r, g, b);
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

	// Use Kelly's 22 colors of maximum contrast, apart from white which is the background color, and grey which is used for the rest
	std::vector<std::string> kellyColors = { "222222", "F3C300", "875692", "F38400", "A1CAF1", "BE0032", "C2B280", "008856", "E68FAC", "0067A5", "F99379", "604E97", "F6A600", "B3446C", "DCD300", "882D17", "8DB600", "654522", "E25822", "2B3D26" };
	
	auto& modelDatabase = m_context.GetDatabase()->GetModelDatabase();
	auto& classCountsMap = m_context.GetDatabase()->GetClassCounts();

	std::vector<std::pair<std::string, int>> classCounts(classCountsMap.begin(), classCountsMap.end());
	sort(classCounts.begin(), classCounts.end(), greater_second<std::string, int>());
	std::cout << classCounts[0].second << std::endl;
	std::unordered_map<std::string, glm::vec3> classColors;
	for (int i = 0; i < classCounts.size(); i++)
	{
		std::string c = classCounts[i].first;
		int count = classCounts[i].second;
		if (i < kellyColors.size())
		{

			classColors[c] = hexToVec(kellyColors[i]); qDebug() << QString::fromStdString(c) << (int)(classColors[c].r * 255) << (int)(classColors[c].g * 255) << (int)(classColors[c].b * 255);
	}
		else
			classColors[c] = glm::vec3(0.5, 0.5, 0.5);
	}

	// Compute colors
	//std::default_random_engine generator;
	//std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	//
	//std::vector<std::string> classes;
	//std::vector<int> indices;
	//for (ModelDescriptor& md : m_context.GetDatabase()->GetModelDatabase())
	//{
	//	std::string c = md.m_class;
	//	bool found = false;
	//	for (int i = 0; i < classes.size(); i++)
	//	{
	//		if (classes[i] == c)
	//		{
	//			indices.push_back(i);
	//			found = true;
	//		}
	//	}
	//	if (!found)
	//	{
	//		classes.push_back(c);
	//		indices.push_back(classes.size() - 1);
	//	}
	//}

	//std::vector<glm::vec3> colorSet;
	//for (int i = 0; i < classes.size(); i++)
	//{
	//	float r = distribution(generator);
	//	float g = distribution(generator);
	//	float b = distribution(generator);
	//	colorSet.push_back(glm::vec3(r, g, b));
	//}

	std::vector<glm::vec3> colors;
	//for (int& index : indices)
	//{
	//	colors.push_back(colorSet[index]);
	//}
	for (auto& p : classColors)
	{
		qDebug() << QString::fromStdString(p.first) << p.second.r << p.second.g << p.second.b;
	}

	for (int i = 0; i < modelDatabase.size(); i++)
	{
		const ModelDescriptor& md = modelDatabase[i];
		colors.push_back(classColors[md.m_class]);
	}
	setColors(colors);

	// Pass bounds and data to renderer
	_pointRenderer.setBounds(_dataBounds);
	_pointRenderer.setData(*points);
	pointSizeChanged(15);
	_pointRenderer.setAlpha(1.0);
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
