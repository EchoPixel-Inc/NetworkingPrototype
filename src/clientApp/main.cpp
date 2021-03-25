#include "clientApp/clientApp.h"
#include "clientApp/appInitializer.h"
#include "clientApp/darkStyle.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QHostAddress>

#include <iostream>

auto main(int argc, char* argv[]) -> int
{
	QApplication app(argc, argv);
	QApplication::setApplicationName("clientApp");
	QApplication::setOrganizationName("JeffCo");
	QApplication::setApplicationVersion("Version 1.0");

	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addVersionOption();
	parser.setApplicationDescription(
		"server application for network image viewer");

	parser.process(app);

	AppInitializer appInitializer;
	if (appInitializer.initialize()) {
		app.setStyle(new DarkStyle);
		return app.exec();
	}
	else {
		std::cerr << "Application failed to initialize" << std::endl;
		return EXIT_FAILURE;
	}
}