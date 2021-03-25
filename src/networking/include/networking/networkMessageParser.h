#ifndef networkMessageParser_h
#define networkMessageParser_h

#include "networking/networkMessage.h"

#include <QByteArray>

#include <functional>
#include <cstdint>

class NetworkMessageParser
{
public:
	using MessageReadyCallbackType = std::function<void(NetworkMessage&)>;

	void parse(const QByteArray&);
	void setMessageReadyCallback(MessageReadyCallbackType);
	NetworkMessage::SizeType getCurrentMessageSize() const;

private:
	enum class MessageSection {
		HEADER,
		TYPE_BYTE1,
		TYPE_BYTE2,
		SIZE_BYTE1,
		SIZE_BYTE2,
		SIZE_BYTE3,
		SIZE_BYTE4,
		DATA,
		CHECKSUM_BYTE1,
		CHECKSUM_BYTE2,
		CHECKSUM_BYTE3,
		CHECKSUM_BYTE4
	};

	MessageReadyCallbackType m_MessageReadyCallback;
	NetworkMessage m_Message;
	NetworkMessage::SizeType m_CurrentMessageSize = 0;
	MessageSection m_ParseStep = MessageSection::HEADER;
	std::uint32_t m_ChecksumValue = 0x00000000;
};

#endif