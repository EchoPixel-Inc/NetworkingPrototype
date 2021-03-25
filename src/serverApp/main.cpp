#include "serverApp/serverApp.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QHostAddress>

#include <iostream>

auto main(int argc, char* argv[]) -> int
{
	QApplication app(argc, argv);
	QApplication::setApplicationName("serverApp");
	QApplication::setOrganizationName("JeffCo");
	QApplication::setApplicationVersion("Version 1.0");

	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addVersionOption();
	parser.setApplicationDescription(
		"server application for network image viewer");

	QCommandLineOption ipOption({{"i", "ipAddress"},
		"The IP address to be used", "ipAddress", "127.0.0.1"});

	QCommandLineOption portOption(
		{{"p", "portNumber"}, "Port number", "port", "3760"});

	QCommandLineOption launcherIPOption({{"l", "launcherIP"},
		"IP address of the launching entity", "launcherIP", "127.0.0.1"});

	parser.addOption(ipOption);
	parser.addOption(portOption);
	parser.addOption(launcherIPOption);
	parser.process(app);

	auto hostAddress = QHostAddress(parser.value(ipOption));
	auto portNumber = parser.value(portOption).toULong();
	auto launcherIPAddress = QHostAddress(parser.value(launcherIPOption));

	std::cout << "IP address: " << hostAddress.toString().toStdString() << "\n";
	std::cout << "Port number: " << portNumber << "\n";
	std::cout << "Session host IP address: "
		<< launcherIPAddress.toString().toStdString()
		<< std::endl;

	if (hostAddress.isNull()) {
		std::cerr << "IP address is not valid" << std::endl;
		return EXIT_FAILURE;
	}

	if (launcherIPAddress.isNull()) {
		std::cerr << "Session host IP address is not valid" << std::endl;
		return EXIT_FAILURE;
	}

	ServerApp serverApp(launcherIPAddress);
	if (!serverApp.listen(hostAddress, portNumber)) {
		std::cerr << "Could not launch server" << std::endl;
		return EXIT_FAILURE;
	}

	return app.exec();
}