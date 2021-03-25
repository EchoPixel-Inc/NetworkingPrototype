#include "barco/barcoUartFrame.h"
#include "barco/barcoV2Protocol.h"

#include <iostream>

using Protocol = BarcoV2Protocol;

//==============================================================================
BarcoUartFrame::BarcoUartFrame()
{
	MessageReset();
}
//==============================================================================

//==============================================================================
std::string BarcoUartFrame::ClassName()
{
	return "BarcoUartFrame";
}
//==============================================================================

//==============================================================================
bool BarcoUartFrame::SendUartMessage(const MessageType& msg)
{
	// This function assumes protocol version 0, if we add support for future
	// versions, we will have to add some stuff here.

	// Based on the protocol version, we add length and checksum

	// We start with a sanity check on protocol and length
	size_t msgLen = msg.size();

	if (msgLen < Protocol::V0_MINLEN) {
		std::cerr << this->ClassName() << "::SendUartMessage"
				  << " - message length (" << msgLen << " ) < "
				  << Protocol::V0_MINLEN << std::endl;

		return false;
	}

	if (msgLen > Protocol::V0_MAXLEN) {
		std::cerr << this->ClassName() << "::SendUartMessage"
				  << " - message length (" << msgLen << " ) > "
				  << Protocol::V0_MAXLEN << std::endl;

		return false;
	}

	// It seems we have a valid message here, so let's move on

	unsigned char checkSum = 0;

	// Here we create a function that sends a byte, doubling it when that byte
	// is the BarcoUartFrame ESC character.
	auto sendMsgByte = [this, &checkSum](unsigned char c) {
		if (c == ESC)
			SendByte(c);
		SendByte(c);
		checkSum += c;
	};

	// Begin with the frame start sequence
	SendByte(ESC);
	SendByte(START);

	// Then send the protocol version
	sendMsgByte(msg.at(0));

	// send the length
	sendMsgByte(static_cast<unsigned char>(msgLen));

	// Send the message bytes itself (not including the protocol version, the
	// length and the checksum)
	for (size_t i = 2; i < msgLen - 1; i++) {
		unsigned char c = msg.at(i);
		sendMsgByte(c);
	}

	// Add the checksum byte, it makes the sum of all bytes in the message equal
	// to 0
	unsigned char cs = static_cast<unsigned char>(0xFF) - checkSum + 1;
	sendMsgByte(cs);

	// Finally, send the frame end sequence
	SendByte(ESC);
	SendByte(STOP);

	return true;
}
//==============================================================================

//==============================================================================
bool BarcoUartFrame::SendByte(const unsigned char& byte)
{
	if (m_sendByteCallback) {
		m_sendByteCallback(byte);
	}

	return true;
}
//==============================================================================

//==============================================================================
bool BarcoUartFrame::ReceiveUartMessage(const MessageType& msg)
{
	if (m_receiveMessageCallback) {
		m_receiveMessageCallback(msg);
	}

	return true;
}
//==============================================================================

//==============================================================================
bool BarcoUartFrame::ReceiveByte(const unsigned char& byte)
{
	// handle incoming bytes here...
	if (m_msgEscState) {
		switch (byte) {
			case START:	 // esc S received => start
				MessageBegin();
				break;
			case STOP:	// esc P received => stop
				MessageComplete();
				break;
			case ESC:  // esc esc received => we received an ESC that is part of
					   // the message
				MessageTryAdd(byte);
				break;
			default:
				// if we have an invalid escape, just discard the message
				MessageReset();
				break;
		}

		m_msgEscState = false;
	}
	else {
		if (byte == ESC) {
			m_msgEscState = true;
		}
		else {
			MessageTryAdd(byte);
		}
	}

	return true;
}
//==============================================================================

//==============================================================================
void BarcoUartFrame::MessageBegin()
{
	MessageReset();
	m_msgReceiving = true;
}
//==============================================================================

//==============================================================================
void BarcoUartFrame::MessageTryAdd(unsigned char byte)
{
	if (m_msgReceiving) {
		// Some checks worth doing here
		// Check the protocol version (always encoded in the first byte)
		if (m_msgByteCnt == 0) {
			// For now, we only support version 0
			if (byte != 0) {
				std::cerr << "Barco Uart: "
						  << " - No support for protocol version " << int(byte)
						  << std::endl;

				MessageReset();
				return;
			}

			m_msgProtocolVersion = byte;
		}

		// Check the message length (for V0, encoded in the second byte)
		if (m_msgByteCnt == 1) {
			m_msgLength = byte;

			if (m_msgLength < Protocol::V0_MINLEN) {
				// cannot be smaller than Protocol::V0_MINLEN
				std::cerr << this->ClassName() << "::Msg_tryAdd"
						  << " - Message length (" << int(byte)
						  << "), cannot be smaller than " << Protocol::V0_MINLEN
						  << std::endl;

				MessageReset();
				return;
			}
		}

		inBuffer.push_back(byte);  // put the byte in the receive buffer...
		m_msgChecksum += byte;	// update the checksum
		m_msgByteCnt++;	 // and increase the byte count

		// Is our buffer size still in line with what our protocol requires?
		if ((m_msgProtocolVersion == 0) &&
			(m_msgByteCnt > Protocol::V0_MAXLEN)) {
			std::cerr << this->ClassName() << "::Msg_tryAdd"
					  << " - message length > " << Protocol::V0_MAXLEN
					  << std::endl;

			MessageReset();
			return;
		}

		// Also, aren't we overrunning our own message length?
		if ((int(m_msgLength) > 0) && (m_msgByteCnt > int(m_msgLength))) {
			std::cerr
				<< this->ClassName() << "::Msg_tryAdd"
				<< " - more bytes in message than message length indicates ["
				<< m_msgByteCnt << " > " << int(m_msgLength) << "]"
				<< std::endl;

			MessageReset();
			return;
		}
	}

	// Not doing anything when not in receiving state, just waiting for a START
	// sequence...
}
//==============================================================================

//==============================================================================
void BarcoUartFrame::MessageComplete()
{
	if (m_msgReceiving) {  // are we in receiving state?
		// Time to check length and checksum consistency, only use if OK
		if (inBuffer.size() <= 2) {
			std::cerr << this->ClassName() << "::Msg_complete"
					  << " - not enough message information" << std::endl;

			MessageReset();
			return;
		}

		// For protocol V0, our length is the second byte of the message
		if (static_cast<size_t>(inBuffer.at(1)) != inBuffer.size()) {
			std::cerr << this->ClassName() << "::Msg_complete"
					  << " - number of bytes in message != what message length "
					  << "indicates [" << m_msgByteCnt
					  << " != " << int(m_msgLength) << "]" << std::endl;

			MessageReset();
			return;
		}

		// For protocol V0, our checksum of all received bytes should be 0
		if (m_msgChecksum != 0) {
			std::cerr << this->ClassName() << "::Msg_complete"
					  << " - checksum of all bytes in the message is not zero ("
					  << int(m_msgChecksum) << ")" << std::endl;

			MessageReset();
			return;
		}

		ReceiveUartMessage(inBuffer);
	}

	MessageReset();
}
//==============================================================================

//==============================================================================
void BarcoUartFrame::MessageReset()
{
	m_msgEscState = false;
	m_msgReceiving = false;
	m_msgLength = 0;
	m_msgProtocolVersion = 0;
	m_msgByteCnt = 0;
	m_msgChecksum = 0;
	inBuffer.clear();
}
//==============================================================================