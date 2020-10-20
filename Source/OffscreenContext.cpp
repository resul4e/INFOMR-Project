#include "OffscreenContext.h"

OffscreenContext::OffscreenContext()
{
	requestedFormat().setVersion(4, 3);
	setFormat(requestedFormat());
	create();

	m_context = new QOpenGLContext(this);
	m_context->setFormat(format());

	if (m_context->create())
	{
		m_context->makeCurrent(this);
		if (!gladLoadGL()) {
			qFatal("No OpenGL context is currently bound, therefore OpenGL function loading has failed.");
		}
	}
	else
	{
		delete m_context;
		m_context = Q_NULLPTR;
		throw("Failed to create OpenGL context for OffscreenContext");
	}
	if (!m_context->isValid())
	{
		throw("OffscreenContext OpenGL context is not valid");
	}
}

void OffscreenContext::bindContext()
{
	m_context->makeCurrent(this);
}

void OffscreenContext::releaseContext()
{
	m_context->doneCurrent();
}
