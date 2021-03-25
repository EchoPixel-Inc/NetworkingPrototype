#include "barco/barcoV2Protocol.h"

#include <iostream>

//==============================================================================
BarcoV2Protocol::BarcoV2Protocol() {}
//==============================================================================

//==============================================================================
std::string BarcoV2Protocol::ClassName()
{
	return "BarcoV2Protocol";
}
//==============================================================================

//==============================================================================
bool BarcoV2Protocol::RequestGetProtocolVersion(
	RequestGetProtocolVersionCallbackType callback)
{
	// set our callback
	m_requestGetProtocolVersionCallback = callback;

	// make command (version, length and checksum left at 0x00)
	MessageType msg = {0x00, 0x00, 0x02, 0x02, 0x00};

	SendProtocolMessage(msg);

	return true;
}
//==============================================================================

//==============================================================================
bool BarcoV2Protocol::ResultGetProtocolVersion(const MessageType& msg)
{
	// check our cmd specific message length first
	if ((msg.size() < 6) || (msg.at(1) < 6)) {
		std::cerr << this->ClassName() << "::ResultGetProtocolVersion"
				  << " - message length (" << int(msg.at(1))
				  << ") should be at least 6" << std::endl;

		return false;
	}

	if (m_requestGetProtocolVersionCallback) {
		// return the status (0x00 is OK, error otherwise) and the version
		m_requestGetProtocolVersionCallback(msg[4], msg[5]);
		return true;
	}

	return false;
}
//==============================================================================

//==============================================================================
bool BarcoV2Protocol::HandleGetProtocolVersion(const MessageType& msg)
{
	// Check our cmd specific message length first
	if ((msg.size() != 5) || (msg.at(1) != 5)) {
		// We could send something to the other side here (in case this is not
		// a notification)
		std::cerr << this->ClassName() << "::HandleGetProtocolVersion"
				  << " - message length (" << int(msg.at(1)) << ") != 5"
				  << std::endl;

		return false;
	}

	// make command (version, length and checksum left at 0x00)
	MessageType reply = {
		0x00, 0x00,	 // version and length
		0x01,  // method: result
		msg[3],	 // requestId
		0x00,  // succeeded
		VERSION,  // actual return, our version
		0x00  // checksum
	};

	SendProtocolMessage(reply);

	return true;
}
//==============================================================================

//==============================================================================
bool BarcoV2Protocol::RequestSendEyeCoordinates(double xL, double yL, double zL,
	double xR, double yR, double zR,
	RequestSendEyeCoordinatesCallbackType callback)
{
	// Set our callback
	m_requestSendEyeCoordinatesCallback = callback;

	// Make command (version, length and checksum left at 0x00)
	MessageType msg = {};

	msg.push_back(0x00);
	msg.push_back(0x00);  // version and length
	msg.push_back(0x03);  // method

	// RequestId: naive handling: notification if no callback is set, method id
	// otherwise
	if (m_requestSendEyeCoordinatesCallback) {
		msg.push_back(0x03);  // requestId => method
	}
	else {
		msg.push_back(
			0x00);	// requestId => 0 => NOTIFICATION (no reply will be sent)
	}

	union
	{
		int u;
		unsigned char b[4];
	} u;

	// Push all coordinates in xL, yL, zL, xR, yR, zR order
	u.u = static_cast<int>(xL * 10000.0);
	msg.push_back(u.b[3]);
	msg.push_back(u.b[2]);
	msg.push_back(u.b[1]);
	msg.push_back(u.b[0]);

	u.u = static_cast<int>(yL * 10000.0);
	msg.push_back(u.b[3]);
	msg.push_back(u.b[2]);
	msg.push_back(u.b[1]);
	msg.push_back(u.b[0]);

	u.u = static_cast<int>(zL * 10000.0);
	msg.push_back(u.b[3]);
	msg.push_back(u.b[2]);
	msg.push_back(u.b[1]);
	msg.push_back(u.b[0]);

	u.u = static_cast<int>(xR * 10000.0);
	msg.push_back(u.b[3]);
	msg.push_back(u.b[2]);
	msg.push_back(u.b[1]);
	msg.push_back(u.b[0]);

	u.u = static_cast<int>(yR * 10000.0);
	msg.push_back(u.b[3]);
	msg.push_back(u.b[2]);
	msg.push_back(u.b[1]);
	msg.push_back(u.b[0]);

	u.u = static_cast<int>(zR * 10000.0);
	msg.push_back(u.b[3]);
	msg.push_back(u.b[2]);
	msg.push_back(u.b[1]);
	msg.push_back(u.b[0]);

	msg.push_back(0x00);  // checkSum

	SendProtocolMessage(msg);

	return true;
}
//==============================================================================

//==============================================================================
bool BarcoV2Protocol::ResultSendEyeCoordinates(const MessageType& msg)
{
	// Check our cmd specific message length first
	if ((msg.size() < 5) || (msg.at(1) < 5)) {
		std::cerr << this->ClassName() << "::ResultSendEyeCoordinates"
				  << "message length (" << int(msg.at(1))
				  << ") should be at least 5" << std::endl;

		return false;
	}

	if (m_requestSendEyeCoordinatesCallback) {
		m_requestSendEyeCoordinatesCallback(msg[4]);
		return true;
	}

	return false;
}
//==============================================================================

//==============================================================================
bool BarcoV2Protocol::HandleSendEyeCoordinates(const MessageType& msg)
{
	// Check our cmd specific message length first
	if (msg.at(1) != 29) {
		std::cerr << this->ClassName() << "::HandleSendEyeCoordinates"
				  << "message length (" << int(msg.at(1)) << ") != 29"
				  << std::endl;

		// We could send something to the other side here (in case this is not
		// a notification)
		return false;
	}

	EyeCoordinatesContainerType coords = {};

	// Pull all coordinates in xL, yL, zL, xR, yR, zR order
	union
	{
		int u;
		unsigned char b[4];
	} u;

	unsigned int b{3};

	u.b[3] = msg[++b];
	u.b[2] = msg[++b];
	u.b[1] = msg[++b];
	u.b[0] = msg[++b];
	coords.push_back(static_cast<double>(u.u) / 10000.0);

	u.b[3] = msg[++b];
	u.b[2] = msg[++b];
	u.b[1] = msg[++b];
	u.b[0] = msg[++b];
	coords.push_back(static_cast<double>(u.u) / 10000.0);

	u.b[3] = msg[++b];
	u.b[2] = msg[++b];
	u.b[1] = msg[++b];
	u.b[0] = msg[++b];
	coords.push_back(static_cast<double>(u.u) / 10000.0);

	u.b[3] = msg[++b];
	u.b[2] = msg[++b];
	u.b[1] = msg[++b];
	u.b[0] = msg[++b];
	coords.push_back(static_cast<double>(u.u) / 10000.0);

	u.b[3] = msg[++b];
	u.b[2] = msg[++b];
	u.b[1] = msg[++b];
	u.b[0] = msg[++b];
	coords.push_back(static_cast<double>(u.u) / 10000.0);

	u.b[3] = msg[++b];
	u.b[2] = msg[++b];
	u.b[1] = msg[++b];
	u.b[0] = msg[++b];
	coords.push_back(static_cast<double>(u.u) / 10000.0);

	if (m_handleSendEyeCoordinatesCallback) {
		m_handleSendEyeCoordinatesCallback(coords);
	}

	// React if not a notification
	if (msg[3] != 0) {
		// make command (version, length and checksum left at 0x00)
		MessageType reply = {
			0x00, 0x00,	 // version and length
			0x01,  // method: result
			msg[3],	 // requestId
			0x00,  // succeeded
			0x00  // checksum
		};

		SendProtocolMessage(reply);
	}

	return true;
}
//==============================================================================

//==============================================================================
bool BarcoV2Protocol::RequestSelectTracker(
	unsigned char tracker, RequestSelectTrackerCallbackType callback)
{
	// Set our callback
	m_requestSelectTrackerCallback = callback;

	// Make command (version, length and checksum left at 0x00)
	MessageType msg = {};

	msg.push_back(0x00);
	msg.push_back(0x00);  // version and length
	msg.push_back(0x04);  // method

	// RequestId: naive handling: notification if no callback is set,
	// method id otherwise
	if (m_requestSelectTrackerCallback) {
		msg.push_back(0x04);  // requestId => method
	}
	else {
		msg.push_back(0x00);  // requestId => 0 => NOTIFICATION
		// (no reply will be sent)
	}

	// Set our tracker
	msg.push_back(tracker);
	msg.push_back(0x00);  // checkSum

	SendProtocolMessage(msg);

	return true;
}
//==============================================================================

//==============================================================================
bool BarcoV2Protocol::ResultSelectTracker(const MessageType& msg)
{
	// check our cmd specific message length first
	if ((msg.size() < 5) || (msg.at(1) < 5)) {
		std::cerr << this->ClassName() << "::ResultSelectTracker"
				  << " - message length (" << int(msg.at(1))
				  << ") should be at least equal to 5" << std::endl;

		return false;
	}

	if (m_requestSelectTrackerCallback) {
		m_requestSelectTrackerCallback(msg[4]);
		return true;
	}

	return false;
}
//==============================================================================

//==============================================================================
bool BarcoV2Protocol::HandleSelectTracker(const MessageType& msg)
{
	// Check our cmd specific message length first
	if ((msg.size() != 6) || (msg.at(1) != 6)) {
		std::cerr << this->ClassName() << "::HandleSelectTracker"
				  << " - message length (" << int(msg.at(1)) << ") != 6"
				  << std::endl;

		// We could send something to the other side here (in case this is not
		// a notification)
		return false;
	}

	bool result{false};

	if (m_handleSelectTrackerCallback) {
		result = m_handleSelectTrackerCallback(msg[4]);
	}

	// React if not a notification
	if (msg[3] != 0) {
		// Make command (version, length and checksum left at 0x00)
		MessageType reply = {
			0x00, 0x00,	 // version and length
			0x01,  // method: result
			msg[3],	 // requestId
			static_cast<unsigned char>(
				result ? 0x00 : 0x01),	// succeeded or not
			0x00  // checksum
		};

		SendProtocolMessage(reply);
	}

	return true;
}
//==============================================================================

//==============================================================================
bool BarcoV2Protocol::SendProtocolMessage(const MessageType& msg)
{
	if (m_sendMessageCallback) {
		m_sendMessageCallback(msg);
	}

	return true;
}
//==============================================================================

//==============================================================================
void BarcoV2Protocol::ReceiveProtocolMessage(MessageType msg)
{
	// When we get here, we assume that the level below (e.g. UartFrame)
	// already checked the basic message consistency (length and checksum)
	switch (msg[2]) {
		case 0x01:	// result message

			// simplified handling: RequestId is our Method...
			switch (msg[3]) {
				case 0x02: ResultGetProtocolVersion(msg); break;

				case 0x03: ResultSendEyeCoordinates(msg); break;

				case 0x04: ResultSelectTracker(msg); break;

				default:
					std::cerr
						<< "Barco system protocol (V2): "
						<< "don't know how to handle Result with RequestId "
						<< int(msg[3]) << std::endl;
			}

			break;

		case 0x02:	// GetProtocolVersion call to be handled
			HandleGetProtocolVersion(msg);
			break;

		case 0x03:	// SendEyeCoordinates call to be handled
			HandleSendEyeCoordinates(msg);
			break;

		case 0x04:	// SelectTracker call to be handled
			HandleSelectTracker(msg);
			break;

		default:
			std::cerr << "Barco system protocol (V2): "
					  << "don't know how to handle Method " << int(msg[2])
					  << std::endl;
	}
}
//==============================================================================
