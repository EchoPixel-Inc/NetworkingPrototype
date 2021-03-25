#include "config/config.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QByteArray>

#include <filesystem>
#include <fstream>
#include <istream>
#include <sstream>

namespace
{
const std::filesystem::path defaultConfigPath{"../configuration/config.json"};

void createDefaultConfigFile(const std::filesystem::path& filename)
{
	QJsonDocument doc;
	QJsonObject rootObject;
	rootObject.insert("version", 1.0);
	rootObject.insert("head_target", "null");
	rootObject.insert("interaction_device", "null");
	doc.setObject(rootObject);

	std::ofstream outputFile(filename);
	outputFile << doc.toJson().toStdString();
}

Config getDefaultConfigFromFile(const std::filesystem::path& filename)
{
	std::filesystem::path configFilePath{filename};
	if (!std::filesystem::exists(configFilePath)) {
		if (!std::filesystem::is_directory(configFilePath.parent_path())) {
			std::filesystem::create_directory("../configuration");
		}

		createDefaultConfigFile(defaultConfigPath);
		return getDefaultConfigFromFile(defaultConfigPath);
	}

	Config defaultConfig;
	defaultConfig.headTarget = "null";
	defaultConfig.interactionDevice = "null";
	defaultConfig.barcoCOMPort = std::string();
	defaultConfig.defaultAlias = std::string();
	defaultConfig.serverPath = "serverApp.exe";

	std::ifstream inputFile(filename);
	std::stringstream buffer;
	buffer << inputFile.rdbuf();
	const QString jsonInput = QString::fromStdString(buffer.str());

	QJsonParseError parseError;
	auto jsonDoc = QJsonDocument::fromJson(jsonInput.toUtf8(), &parseError);
	if (parseError.error == QJsonParseError::NoError) {
		if (auto rootObject = jsonDoc.object(); !rootObject.isEmpty()) {
			if (auto it = rootObject.constFind("head_target");
				it != rootObject.end()) {
				if (auto val = *it; val.isString()) {
					defaultConfig.headTarget = val.toString().toStdString();
				}
			}

			if (auto it = rootObject.constFind("interaction_device");
				it != rootObject.end()) {
				if (auto val = *it; val.isString()) {
					defaultConfig.interactionDevice =
						val.toString().toStdString();
				}
			}

			if (auto it = rootObject.constFind("barco_com_port");
				it != rootObject.end()) {
				if (auto val = *it; val.isString()) {
					defaultConfig.barcoCOMPort = val.toString().toStdString();
				}
			}

			if (auto it = rootObject.constFind("default_alias");
				it != rootObject.end()) {
				if (auto val = *it; val.isString()) {
					defaultConfig.defaultAlias = val.toString().toStdString();
				}
			}

			if (auto it = rootObject.constFind("server_path");
				it != rootObject.end()) {
				if (auto val = *it; val.isString()) {
					defaultConfig.serverPath = val.toString().toStdString();
				}
			}
		}
	}

	return defaultConfig;
}
}  // end namespace

//==============================================================================
auto Config::getDefaultConfig() -> const Config&
{
	static const Config defaultConfig =
		getDefaultConfigFromFile(defaultConfigPath);
	return defaultConfig;
}
//==============================================================================
