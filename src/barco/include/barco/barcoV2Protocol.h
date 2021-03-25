#ifndef barcoV2Protocol_h
#define barcoV2Protocol_h

#include <iostream>
#include <string>
#include <vector>
#include <functional>

class BarcoV2Protocol
{
public:
	/// \brief Constructor
	BarcoV2Protocol();

	/// \brief Returns the name of this class as a string
	static std::string ClassName();

	// Some protocol defines
	static const unsigned char VERSION = 0;
	static const int V0_MAXLEN = 255;  // for V0, len is encoded in one byte
	static const int V0_MINLEN =
		5;	// for V0, the shotest possible message is 5 bytes
	static const int MAXLEN = V0_MAXLEN;
	static const int MINLEN = V0_MINLEN;

	using MessageType = std::vector<unsigned char>;
	using EyeCoordinatesContainerType = std::vector<double>;

	using RequestGetProtocolVersionCallbackType =
		std::function<void(unsigned char, unsigned char)>;

	using RequestSendEyeCoordinatesCallbackType =
		std::function<void(unsigned char)>;

	using RequestSelectTrackerCallbackType = std::function<void(unsigned char)>;

	using HandleSendEyeCoordinatesCallbackType =
		std::function<void(const EyeCoordinatesContainerType&)>;

	using HandleSelectTrackerCallbackType = std::function<bool(unsigned char)>;

	using SendMessageCallbackType = std::function<void(const MessageType&)>;

	/// \brief Request the protocol version
	bool RequestGetProtocolVersion(
		RequestGetProtocolVersionCallbackType = nullptr);
	// No setHandleGetProtocolVersionCallback, as we know our version

	/// \brief Sends eye coordinates to the display
	bool RequestSendEyeCoordinates(double xL, double yL, double zL, double xR,
		double yR, double zR, RequestSendEyeCoordinatesCallbackType = nullptr);

	/// \brief Sets the callback that should be invoked when eye coordinates
	/// are received from the tracker
	void SetHandleSendEyeCoordinatesCallback(
		HandleSendEyeCoordinatesCallbackType callback)
	{
		m_handleSendEyeCoordinatesCallback = callback;
	}

	/// \brief Selects the tracker (internal or external)
	bool RequestSelectTracker(
		unsigned char tracker, RequestSelectTrackerCallbackType = nullptr);

	/// \brief Handles requests to select the tracker
	void SetHandleSelectTrackerCallback(
		HandleSelectTrackerCallbackType callback)
	{
		m_handleSelectTrackerCallback = callback;
	}

	// Generic protocol handling
	void ReceiveProtocolMessage(MessageType msg);
	bool SendProtocolMessage(const MessageType& msg);

	// other connections
	void SetSendMessageCallback(SendMessageCallbackType callback)
	{
		m_sendMessageCallback = callback;
	}

private:
	// GetProtocolVersion callbacks and methods
	RequestGetProtocolVersionCallbackType m_requestGetProtocolVersionCallback;
	bool HandleGetProtocolVersion(const MessageType& msg);

	// no m_handleGetProtocolVersionCallback, as we know our version
	bool ResultSendEyeCoordinates(const MessageType& msg);

	// SendEyeCoordinates callbacks and methods
	RequestSendEyeCoordinatesCallbackType m_requestSendEyeCoordinatesCallback;
	bool HandleSendEyeCoordinates(const MessageType& msg);

	HandleSendEyeCoordinatesCallbackType m_handleSendEyeCoordinatesCallback =
		nullptr;
	bool ResultGetProtocolVersion(const MessageType& msg);

	// SelectTracker callbacks and methods
	RequestSelectTrackerCallbackType m_requestSelectTrackerCallback;
	bool HandleSelectTracker(const MessageType& msg);

	HandleSelectTrackerCallbackType m_handleSelectTrackerCallback = nullptr;
	bool ResultSelectTracker(const MessageType& msg);

	// other
	SendMessageCallbackType m_sendMessageCallback = nullptr;
};

#endif
