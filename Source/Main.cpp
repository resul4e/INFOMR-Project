#include "MainWindow.h"

#include <QApplication>
#include <QSurfaceFormat>

#include <iostream>

int main(int argc, char** argv)
{
	__noop(argc);
	__noop(argv);

	QCoreApplication::setApplicationName("Multimedia Retriever");

#ifdef __APPLE__
	// Ask for an OpenGL 3.3 Core Context as the default
	QSurfaceFormat defaultFormat;
	defaultFormat.setVersion(3, 3);
	defaultFormat.setProfile(QSurfaceFormat::CoreProfile);
	defaultFormat.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	QSurfaceFormat::setDefaultFormat(defaultFormat);
#endif

	QApplication app(argc, argv);

	// Retina display support for Mac OS and X11:
	// AA_UseHighDpiPixmaps attribute is off by default in Qt 5.1 but will most
	// likely be on by default in a future release of Qt.
	app.setAttribute(Qt::AA_UseHighDpiPixmaps);

	QIcon appIcon;
	appIcon.addFile(":/Icons/AppIcon32");
	appIcon.addFile(":/Icons/AppIcon64");
	appIcon.addFile(":/Icons/AppIcon128");
	appIcon.addFile(":/Icons/AppIcon256");
	appIcon.addFile(":/Icons/AppIcon512");
	appIcon.addFile(":/Icons/AppIcon1024");
	app.setWindowIcon(appIcon);

	MainWindow mainWindow;
	mainWindow.centerAndResize(0.75f);
	mainWindow.show();

	return app.exec();
}
