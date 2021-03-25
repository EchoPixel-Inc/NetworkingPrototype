#ifndef networkMessage_h
#define networkMessage_h

#include <QByteArray>
#include <QMetaType>

#include <vector>

class NetworkMessage
{
public:
	enum MessageType : std::uint16_t {
		REQUEST_CREDENTIALS,
		PEER_CREDENTIALS,
		FULL_STATE,
		AUTHORIZATION_SUCCEEDED,
		AUTHORIZATION_FAILED,
		PEER_ADDED,
		PEER_REMOVED,
		LASER_UPDATED,
		VOLUME_UPDATED,
		WIDGET_EVENT,
		PLANE_EVENT
	};

	using HeaderType = std::uint8_t;
	using DescriptorType = std::uint16_t;
	using SizeType = std::uint32_t;
	using PayloadDataType = std::vector<std::uint8_t>;
	using ChecksumType = std::uint32_t;

	NetworkMessage() = default;
	~NetworkMessage() = default;

	QByteArray serialize() const;

	HeaderType header = 0x00;
	DescriptorType type = 0x0000;
	SizeType size = 0x00000000;	 // size of payload data
	PayloadDataType data;
	ChecksumType checksum = 0x00000000;
};

Q_DECLARE_METATYPE(NetworkMessage);

#endif