#include "networking/networkMessageParser.h"
#include "common/crcUtils.h"

#ifdef _WIN32
#	include <WinSock2.h>
#else
#	include <arpa/inet.h>
#endif

#pragma comment(lib, "Ws2_32.lib")

//=============================================================================
void NetworkMessageParser::parse(const QByteArray& data)
{
	for (const auto& arrayElem : data) {
		const auto& byte = static_cast<const std::uint8_t&>(arrayElem);

		switch (m_ParseStep) {
			case MessageSection::HEADER: {
				if (byte != 0x00) {
					m_ParseStep = MessageSection::HEADER;
					m_ChecksumValue = 0x00000000;
					m_CurrentMessageSize = 0;

					continue;
				}
				m_Message.header = byte;
				crc::updateCRC32(byte, m_ChecksumValue);
				m_ParseStep = MessageSection::TYPE_BYTE1;
				break;
			}
			case MessageSection::TYPE_BYTE1: {
				m_Message.type = static_cast<std::uint16_t>(byte) << 8;
				crc::updateCRC32(byte, m_ChecksumValue);
				m_ParseStep = MessageSection::TYPE_BYTE2;
				break;
			}
			case MessageSection::TYPE_BYTE2: {
				m_Message.type |= static_cast<std::uint16_t>(byte) << 0;
				m_Message.type = ntohs(m_Message.type);
				crc::updateCRC32(byte, m_ChecksumValue);
				m_ParseStep = MessageSection::SIZE_BYTE1;
				break;
			}
			case MessageSection::SIZE_BYTE1: {
				m_Message.size = static_cast<std::uint32_t>(byte) << 24;
				crc::updateCRC32(byte, m_ChecksumValue);
				m_ParseStep = MessageSection::SIZE_BYTE2;
				break;
			}
			case MessageSection::SIZE_BYTE2: {
				m_Message.size |= static_cast<std::uint32_t>(byte) << 16;
				crc::updateCRC32(byte, m_ChecksumValue);
				m_ParseStep = MessageSection::SIZE_BYTE3;
				break;
			}
			case MessageSection::SIZE_BYTE3: {
				m_Message.size |= static_cast<std::uint32_t>(byte) << 8;
				crc::updateCRC32(byte, m_ChecksumValue);
				m_ParseStep = MessageSection::SIZE_BYTE4;
				break;
			}
			case MessageSection::SIZE_BYTE4: {
				m_Message.size |= static_cast<std::uint32_t>(byte) << 0;
				m_Message.size = ntohl(m_Message.size);
				crc::updateCRC32(byte, m_ChecksumValue);
				m_CurrentMessageSize = m_Message.size;
				m_ParseStep = MessageSection::DATA;

				m_Message.data.clear();
				if (m_Message.size > 0) {
					m_Message.data.reserve(m_Message.size);
				}
				else {
					m_ParseStep = MessageSection::CHECKSUM_BYTE1;
				}
				break;
			}
			case MessageSection::DATA: {
				m_Message.data.push_back(byte);
				crc::updateCRC32(byte, m_ChecksumValue);

				if (m_Message.data.size() == m_Message.size) {
					m_ParseStep = MessageSection::CHECKSUM_BYTE1;
				}
				break;
			}
			case MessageSection::CHECKSUM_BYTE1: {
				m_Message.checksum = static_cast<std::uint32_t>(byte) << 24;
				m_ParseStep = MessageSection::CHECKSUM_BYTE2;
				break;
			}
			case MessageSection::CHECKSUM_BYTE2: {
				m_Message.checksum |= static_cast<std::uint32_t>(byte) << 16;
				m_ParseStep = MessageSection::CHECKSUM_BYTE3;
				break;
			}
			case MessageSection::CHECKSUM_BYTE3: {
				m_Message.checksum |= static_cast<std::uint32_t>(byte) << 8;
				m_ParseStep = MessageSection::CHECKSUM_BYTE4;
				break;
			}
			case MessageSection::CHECKSUM_BYTE4: {
				m_Message.checksum |= static_cast<std::uint32_t>(byte) << 0;
				m_Message.checksum = ntohl(m_Message.checksum);

				// Does checksum match the internally-calculated checksum?
				 if ((m_Message.checksum == m_ChecksumValue) &&
					 m_MessageReadyCallback) {
					 std::invoke(m_MessageReadyCallback, m_Message);
				}

				m_ParseStep = MessageSection::HEADER;
				m_ChecksumValue = 0x00000000;
				m_CurrentMessageSize = 0;

				break;
			}
		}  // end switch
	}
}
//=============================================================================

//=============================================================================
NetworkMessage::SizeType NetworkMessageParser::getCurrentMessageSize() const
{
	return m_CurrentMessageSize;
}
//=============================================================================

//=============================================================================
void NetworkMessageParser::setMessageReadyCallback(
	MessageReadyCallbackType callback)
{
	m_MessageReadyCallback = callback;
}
//=============================================================================
