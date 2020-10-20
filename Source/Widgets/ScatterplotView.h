#pragma once

#include "Context.h"

#include "Graphics/PointRenderer.h"

#include <glm/glm.hpp>
#include "Graphics/Bounds.h"

#include <QOpenGLWidget>

#include <QMouseEvent>

class ScatterplotView : public QOpenGLWidget
{
	Q_OBJECT
public:
	ScatterplotView(Context& _context);

	~ScatterplotView();

	/** Returns true when the widget was initialized and is ready to be used. */
	bool isInitialized();

	/**
	 * Feed 2-dimensional data to the scatterplot.
	 */
	void setData(const std::vector<glm::vec2>* data);
	void setHighlights(const std::vector<char>& highlights);
	void setScalars(const std::vector<float>& scalars);
	void setColors(const std::vector < glm::vec3> & colors);

	void setPointSize(const float size);
	void setScalarEffect(PointEffect effect);
	void setAlpha(const float alpha);
	void setPointScaling(PointScaling scalingMode);
	void setSigma(const float sigma);

protected:
	void initializeGL()         Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	void paintGL()              Q_DECL_OVERRIDE;

	void mousePressEvent(QMouseEvent *event)   Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *event)    Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

	void cleanup();

signals:
	void initialized();

public slots:
	void pointSizeChanged(const int size);
	void pointOpacityChanged(const int opacity);
	void onEmbeddingChanged();

private:
	const glm::mat3 toClipCoordinates = glm::mat3(2.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, -1.0f, -1.0f, 1.0f);
	glm::mat3 toNormalisedCoordinates;
	glm::mat3 toIsotropicCoordinates;

	Context& m_context;

	bool _isInitialized = false;

	/* Renderers */
	PointRenderer _pointRenderer;

	/* Size of the scatterplot widget */
	QSize _windowSize;

	Bounds2D _dataBounds;
};
