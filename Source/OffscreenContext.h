#pragma once

#include <hdi/utils/glad/glad.h>

#include <QWindow>
#include <QOffscreenSurface>
#include <QOpenGLContext>

class OffscreenContext : public QOffscreenSurface
{
	Q_OBJECT
public:
	OffscreenContext();

	void bindContext();
	void releaseContext();

	QOpenGLContext* m_context;
};
