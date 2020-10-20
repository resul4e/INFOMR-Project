#pragma once

#include "BufferObject.h"
#include "Bounds.h"
#include "Shader.h"

#include <QOpenGLFunctions_3_3_Core>
#include <glm/glm.hpp>

//#include "../graphics/Texture.h"

#include <QRectF>

enum PointScaling {
	Absolute, Relative
};

enum PointEffect {
	None, Color, Size, Outline
};

struct PointArrayObject : private QOpenGLFunctions_3_3_Core
{
public:
	GLuint _handle;

	BufferObject _positionBuffer;
	BufferObject _highlightBuffer;
	BufferObject _scalarBuffer;
	BufferObject _colorBuffer;

	void init();
	void setPositions(const std::vector<glm::vec2>& positions);
	void setHighlights(const std::vector<char>& highlights);
	void setScalars(const std::vector<float>& scalars);
	void setColors(const std::vector<glm::vec3>& colors);

	void enableAttribute(uint index, bool enable);

	bool hasHighlights() const { return !_highlights.empty(); }
	bool hasScalars() const { return !_scalars.empty(); }
	bool hasColors() const { return !_colors.empty(); }
	glm::vec3 getScalarRange() { return glm::vec3(_scalarLow, _scalarHigh, _scalarRange); }
	void draw();
	void destroy();

private:
	// Vertex array indices
	const uint ATTRIBUTE_VERTICES   = 0;
	const uint ATTRIBUTE_POSITIONS  = 1;
	const uint ATTRIBUTE_HIGHLIGHTS = 2;
	const uint ATTRIBUTE_SCALARS    = 3;
	const uint ATTRIBUTE_COLORS     = 4;

	/* Point attributes */
	std::vector<glm::vec2> _positions;
	std::vector<char>     _highlights;
	std::vector<float>    _scalars;
	std::vector<glm::vec3> _colors;

	float _scalarLow;
	float _scalarHigh;
	float _scalarRange;

	bool _dirtyPositions = false;
	bool _dirtyHighlights = false;
	bool _dirtyScalars = false;
	bool _dirtyColors = false;
};

struct PointSettings
{
	// Constants
	const float         DEFAULT_POINT_SIZE       = 15;
	const float         DEFAULT_ALPHA_VALUE      = 0.5f;
	const PointScaling  DEFAULT_POINT_SCALING    = PointScaling::Relative;

	PointScaling        _scalingMode             = DEFAULT_POINT_SCALING;
	float               _pointSize               = DEFAULT_POINT_SIZE;
	float               _alpha                   = DEFAULT_ALPHA_VALUE;
};

class PointRenderer : protected QOpenGLFunctions_3_3_Core
{
public:
	void setData(const std::vector<glm::vec2>& points);
	void setHighlights(const std::vector<char>& highlights);
	void setScalars(const std::vector<float>& scalars);
	void setColors(const std::vector<glm::vec3>& colors);

	void setScalarEffect(const PointEffect effect);
	void setColormap(const QString colormap);
	void setBounds(const Bounds2D& bounds);
	void setPointSize(const float size);
	void setAlpha(const float alpha);
	void setPointScaling(PointScaling scalingMode);
	void setOutlineColor(glm::vec3 color);

	void init();
	void resize(QSize renderSize);
	void render();
	void destroy();

private:
	/* Point properties */
	PointSettings _pointSettings;
	PointEffect   _pointEffect = PointEffect::Size;
	glm::vec3      _outlineColor = glm::vec3(0, 0, 1);

	/* Window properties */
	QSize _windowSize;

	/* Rendering variables */
	ShaderProgram _shader;

	PointArrayObject _gpuPoints;

	glm::mat3 _orthoM;
	Bounds2D _bounds = Bounds2D(-1, 1, -1, 1);
};
