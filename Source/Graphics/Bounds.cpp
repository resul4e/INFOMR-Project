#include "Bounds.h"

#include <limits>

const Bounds2D Bounds2D::Max = Bounds2D (
	std::numeric_limits<float>::infinity(),
	-std::numeric_limits<float>::infinity(),
	std::numeric_limits<float>::infinity(),
	-std::numeric_limits<float>::infinity()
);

Bounds2D::Bounds2D() :
	Bounds2D(-1, 1, -1, 1)
{ }

Bounds2D::Bounds2D(float left, float right, float bottom, float top) :
	_left(left),
	_right(right),
	_bottom(bottom),
	_top(top)
{ }

void Bounds2D::setBounds(float left, float right, float bottom, float top)
{
	_left = left;
	_right = right;
	_bottom = bottom;
	_top = top;
}

void Bounds2D::ensureMinimumSize(float width, float height)
{
	glm::vec2 center = getCenter();

	if (getWidth() < width)
	{
		_left = center.x - width / 2;
		_right = center.y + width / 2;
	}
	if (getHeight() < height)
	{
		_bottom = center.y - height / 2;
		_top = center.y + height / 2;
	}
}

void Bounds2D::moveToOrigin()
{
	glm::vec2 center = getCenter();
	_left -= center.x;
	_right -= center.x;
	_bottom -= center.y;
	_top -= center.y;
}

void Bounds2D::makeSquare()
{
	glm::vec2 center = getCenter();
	float halfSize = getWidth() > getHeight() ? getWidth() / 2 : getHeight() / 2;

	_left = center.x - halfSize;
	_right = center.x + halfSize;
	_bottom = center.y - halfSize;
	_top = center.y + halfSize;
}

void Bounds2D::expand(float fraction)
{
	float widthOffset = (getWidth() * fraction) / 2;
	float heightOffset = (getHeight() * fraction) / 2;

	_left -= widthOffset;
	_right += widthOffset;
	_bottom -= heightOffset;
	_top += heightOffset;
}
