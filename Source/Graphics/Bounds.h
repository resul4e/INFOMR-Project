#pragma once

/**
	Custom bounds class because the QRectF class in Qt should kindly remove itself from the library.

	This bounds class:
		* Has a bottom-left anchor point.
		* Does not allow negative sizes
		* Does not secretly clamp values set by the user
*/

#include <glm/glm.hpp>

class Bounds2D
{
public:
	const static Bounds2D Max;

	Bounds2D();
	Bounds2D(float left, float right, float bottom, float top);

	void setBounds(float left, float right, float bottom, float top);
	void ensureMinimumSize(float width, float height);
	void moveToOrigin();
	void makeSquare();
	void expand(float fraction);

	float getWidth() const { return _right - _left; }
	float getHeight() const { return _top - _bottom; }
	glm::vec2 getCenter() const { return glm::vec2( (_left + _right) / 2, (_bottom + _top) / 2 ); }

	float getLeft() const { return _left; }
	float getRight() const { return _right; }
	float getBottom() const { return _bottom; }
	float getTop() const { return _top; }

	void setLeft(float left) { _left = left; }
	void setRight(float right) { _right = right; }
	void setBottom(float bottom) { _bottom = bottom; }
	void setTop(float top) { _top = top; }

private:
	float _left;
	float _right;
	float _bottom;
	float _top;
};
