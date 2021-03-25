#ifndef config_h
#define config_h

#include <string>

class Config {
public:
	std::string headTarget; // head target name
	std::string interactionDevice; // interaction device name
	std::string barcoCOMPort; // port used for Barco display communication
	std::string defaultAlias; // alias (name) used when joining a network session
	std::string serverPath; // path to the server application

	static const Config& getDefaultConfig();
};

#endif