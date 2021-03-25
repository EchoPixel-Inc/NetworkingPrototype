#include "networking/networkMessageParser.h"
#include "networking/networkMessage.h"
#include "gtest/gtest.h"

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

#pragma comment(lib, "Ws2_32.lib")

//=============================================================================
class NetworkMessageParserTest : public ::testing::Test
{
protected:
	NetworkMessageParser m_MessageParser;
};
//=============================================================================

//=============================================================================
TEST_F(NetworkMessageParserTest, TestCRCValue)
{
	std::uint32_t value = 5032056;

	QByteArray arr;
	arr.push_back(htonl(value) >> 24);
	arr.push_back(htonl(value) >> 16);
	arr.push_back(htonl(value) >> 8);
	arr.push_back(htonl(value) >> 0);

	std::uint32_t decoded = 0x00000000;
	decoded = static_cast<std::uint32_t>(static_cast<unsigned char>(arr[0]))
		<< 24;
	decoded |= static_cast<std::uint32_t>(static_cast<unsigned char>(arr[1]))
		<< 16;
	decoded |= static_cast<std::uint32_t>(static_cast<unsigned char>(arr[2]))
		<< 8;
	decoded |= static_cast<std::uint32_t>(static_cast<unsigned char>(arr[3]))
		<< 0;
	decoded = ntohl(decoded);

	ASSERT_TRUE(decoded == value);
}
//=============================================================================

//=============================================================================
TEST_F(NetworkMessageParserTest, TestMessageType)
{
	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::MessageType::PEER_ADDED;

	auto byteArray = msg.serialize();

	NetworkMessage decodedMsg;
	decodedMsg.type = NetworkMessage::MessageType::FULL_STATE;

	m_MessageParser.setMessageReadyCallback(
		[&decodedMsg](const NetworkMessage& msg) { decodedMsg = msg; });
	m_MessageParser.parse(byteArray);

	ASSERT_TRUE(decodedMsg.type == NetworkMessage::MessageType::PEER_ADDED);
}
//=============================================================================

//=============================================================================
TEST_F(NetworkMessageParserTest, TestNoPayloadMessage)
{
	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::MessageType::PEER_CREDENTIALS;
	msg.size = 0;

	auto byteArray = msg.serialize();

	NetworkMessage decodedMsg;
	decodedMsg.size = 100;

	m_MessageParser.setMessageReadyCallback(
		[&decodedMsg](const NetworkMessage& msg) { decodedMsg = msg; });
	m_MessageParser.parse(byteArray);

	ASSERT_TRUE(decodedMsg.size == 0);
}
//=============================================================================

//=============================================================================
TEST_F(NetworkMessageParserTest, TestMessageSize)
{
	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::MessageType::PEER_ADDED;
	std::string message{"Hello world"};
	msg.data = {message.begin(), message.end()};
	msg.size = msg.data.size();

	auto byteArray = msg.serialize();

	NetworkMessage decodedMsg;

	m_MessageParser.setMessageReadyCallback(
		[&decodedMsg](const NetworkMessage& msg) { decodedMsg = msg; });
	m_MessageParser.parse(byteArray);

	ASSERT_TRUE(decodedMsg.size == msg.size);
}
//=============================================================================

//=============================================================================
TEST_F(NetworkMessageParserTest, TestMessagePayload)
{
	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::MessageType::PEER_ADDED;
	std::string message{"Hello world"};
	msg.data = {message.begin(), message.end()};
	msg.size = msg.data.size();

	auto byteArray = msg.serialize();

	NetworkMessage decodedMsg;

	m_MessageParser.setMessageReadyCallback(
		[&decodedMsg](const NetworkMessage& msg) { decodedMsg = msg; });
	m_MessageParser.parse(byteArray);
	std::string payload = {decodedMsg.data.begin(), decodedMsg.data.end()};

	ASSERT_TRUE(message == payload);
}
//=============================================================================

//=============================================================================
TEST_F(NetworkMessageParserTest, TestCorruptedMessage)
{
	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::MessageType::PEER_ADDED;
	std::string message{"Hello world"};
	msg.data = {message.begin(), message.end()};
	msg.size = msg.data.size();

	auto byteArray = msg.serialize();
	*(byteArray.begin() + 5) = 0xAF;

	NetworkMessage decodedMsg;
	bool msgReceived{false};

	m_MessageParser.setMessageReadyCallback(
		[&decodedMsg, &msgReceived](const NetworkMessage& msg) {
			decodedMsg = msg;
			msgReceived = true;
		});
	m_MessageParser.parse(byteArray);

	ASSERT_FALSE(msgReceived);
}
//=============================================================================

//=============================================================================
TEST_F(NetworkMessageParserTest, TestMessageSizeTooLarge)
{
	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::MessageType::PEER_ADDED;
	std::string message{"Hello world"};
	msg.data = {message.begin(), message.end()};
	msg.size = msg.data.size() + 1;

	auto byteArray = msg.serialize();

	NetworkMessage decodedMsg;
	bool msgReceived{false};

	m_MessageParser.setMessageReadyCallback(
		[&decodedMsg, &msgReceived](const NetworkMessage& msg) {
			decodedMsg = msg;
			msgReceived = true;
		});
	m_MessageParser.parse(byteArray);

	ASSERT_FALSE(msgReceived);
}
//=============================================================================

//=============================================================================
TEST_F(NetworkMessageParserTest, TestMessageSizeTooSmall)
{
	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::MessageType::PEER_ADDED;
	std::string message{"Hello world"};
	msg.data = {message.begin(), message.end()};
	msg.size = msg.data.size() - 1;

	auto byteArray = msg.serialize();

	NetworkMessage decodedMsg;
	bool msgReceived{false};

	m_MessageParser.setMessageReadyCallback(
		[&decodedMsg, &msgReceived](const NetworkMessage& msg) {
			decodedMsg = msg;
			msgReceived = true;
		});
	m_MessageParser.parse(byteArray);

	ASSERT_FALSE(msgReceived);
}
//=============================================================================

//=============================================================================
int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}