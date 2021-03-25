#ifndef messages_h
#define messages_h

#include "common/coreTypes.h"

#include <string>

struct PeerInfo
{
	using IdType = common::IdType;
	using ColorVectorType = common::ColorVectorType;

	PeerInfo(IdType id = 0, const std::string& alias = std::string(),
		const ColorVectorType& color = {0.0, 0.0, 0.0}) :
		id{id},
		alias{alias},
		color{color}
	{}
	IdType id;
	std::string alias;
	ColorVectorType color;
};

struct PeerCredentials
{
	PeerCredentials(const std::string& sessionCode = std::string(),
		const std::string& alias = std::string()) :
		sessionCode{sessionCode},
		alias{alias}
	{}
	std::string sessionCode;
	std::string alias;
};

struct LaserUpdate
{
	using IdType = common::IdType;
	using PropertyListType = common::PropertyListType;
	explicit LaserUpdate(
		const PropertyListType& propList = PropertyListType(), IdType id = 0) :
		id{id},
		propList{propList}
	{}

	IdType id;
	PropertyListType propList;
};

struct VolumeUpdate
{
	using IdType = common::IdType;
	using PropertyListType = common::PropertyListType;

	enum class MessageType {
		INTERACTION_STARTED,
		PROPERTY_UPDATE,
		INTERACTION_ENDED
	};

	explicit VolumeUpdate(
		MessageType messageType = MessageType::PROPERTY_UPDATE,
		const PropertyListType& propList = PropertyListType(), IdType id = 0) :
		id{id},
		propList{propList},
		msgType{messageType}
	{}

	MessageType msgType;
	IdType id;
	PropertyListType propList;
};

struct WidgetUpdate
{
	using IdType = common::IdType;
	using PropertyListType = common::PropertyListType;

	enum class MessageType {
		CREATE,
		DESTROY,
		PROPERTY_UPDATE,
		INTERACTION_ENDED
	};

	explicit WidgetUpdate(
		MessageType messageType = MessageType::PROPERTY_UPDATE,
		IdType widgetId = 0,
		const PropertyListType& propList = PropertyListType(),
		IdType ownerId = 0) :
		widgetId{widgetId},
		ownerId{ownerId},
		propList{propList},
		msgType{messageType}
	{}

	MessageType msgType;
	IdType widgetId;
	IdType ownerId;
	PropertyListType propList;
};

struct PlaneUpdate
{
	using PropertyListType = common::PropertyListType;
	enum class MessageType {
		PROPERTY_UPDATE,
		INTERACTION_ENDED
	};

	explicit PlaneUpdate(MessageType msg = MessageType::PROPERTY_UPDATE,
		const PropertyListType& propList = PropertyListType()) :
		msgType{ msg },
		propList{ propList } {}

	MessageType msgType;
	PropertyListType propList;
};

#endif