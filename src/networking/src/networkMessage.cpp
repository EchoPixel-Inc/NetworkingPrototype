#include "networking/networkMessage.h"
#include "common/crcUtils.h"

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

#pragma comment(lib, "Ws2_32.lib")

//=============================================================================
QByteArray NetworkMessage::serialize() const
{
	QByteArray message;
	std::uint32_t crc{ 0 };

	message.push_back(this->header);
	crc::updateCRC32(message.back(), crc);

	message.push_back(htons(this->type) >> 8);
	crc::updateCRC32(message.back(), crc);

	message.push_back(htons(this->type) >> 0);
	crc::updateCRC32(message.back(), crc);

	message.push_back(htonl(this->size) >> 24);
	crc::updateCRC32(message.back(), crc);
	
	message.push_back(htonl(this->size) >> 16);
	crc::updateCRC32(message.back(), crc);

	message.push_back(htonl(this->size) >> 8);
	crc::updateCRC32(message.back(), crc);

	message.push_back(htonl(this->size) >> 0);
	crc::updateCRC32(message.back(), crc);
	
	for (const auto& byte : this->data) {
		message.push_back(byte);
		crc::updateCRC32(message.back(), crc);
	}

	message.push_back(htonl(crc) >> 24);
	message.push_back(htonl(crc) >> 16);
	message.push_back(htonl(crc) >> 8);
	message.push_back(htonl(crc) >> 0);

	return message;
}
//=============================================================================