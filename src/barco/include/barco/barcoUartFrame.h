#ifndef barcoUartFrame_h
#define barcoUartFrame_h

#include <iostream>
#include <vector>
#include <functional>
#include <string>

class BarcoUartFrame
{
public:
	using MessageType = std::vector<unsigned char>;
	using ReceiveMessageCallbackType = std::function<void(const MessageType&)>;

	using SendByteCallbackType = std::function<void(const unsigned char&)>;

	/// \brief Constructor
	BarcoUartFrame();

	/// \brief Returns the name of this class as a string
	static std::string ClassName();

	/// \brief Sends a message over the serial port
	bool SendUartMessage(const MessageType&);

	/// \brief Handles a received byte
	bool ReceiveByte(const unsigned char& byte);

	/// \brief Sets the callback to be invoked when a message is received
	void SetReceiveMessageCallback(ReceiveMessageCallbackType callback)
	{
		m_receiveMessageCallback = callback;
	}

	/// \brief Sets the callback to be invoked when a byte is sent
	void SetSendByteCallback(SendByteCallbackType callback)
	{
		m_sendByteCallback = callback;
	}

private:
	// receiving frame handling
	void MessageBegin();
	void MessageTryAdd(unsigned char byte);
	void MessageComplete();
	void MessageReset();

	// These methods call the callbacks
	bool ReceiveUartMessage(const MessageType&);
	bool SendByte(const unsigned char& byte);

	enum { START = 'S', STOP = 'P', ESC = 0x00 };

	ReceiveMessageCallbackType m_receiveMessageCallback = nullptr;
	SendByteCallbackType m_sendByteCallback = nullptr;

	bool m_msgEscState;	 // receiver escape state
	bool m_msgReceiving;  // true when receiving a message (start detected)
	unsigned char
		m_msgProtocolVersion;  // protocol version of the received message
	unsigned char m_msgLength;	// length of the received message (max 255 for
								// this version)
	unsigned char m_msgChecksum;  // checksum of this message
	int m_msgByteCnt;  // number of bytes received for this message
	std::vector<unsigned char> inBuffer;
};

#endif