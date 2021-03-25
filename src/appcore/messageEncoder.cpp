#include "appcore/messageEncoder.h"
#include "appcore/messages.h"
#include "appcore/serializationHelper.h"
#include "appcore/serializationTypes.h"
#include "appcore/applicationObjects.h"
#include "widgets/laserWidget.h"
#include "widgets/volumeWidget.h"
#include "widgets/splineWidget.h"
#include "widgets/planeWidget.h"

#include <sstream>
#include <vector>
//=============================================================================
void MessageEncoder::processMessage(const NetworkMessage& msg, IdType senderId)
{
	using MessageType = NetworkMessage::MessageType;

	switch (msg.type) {
		case MessageType::REQUEST_CREDENTIALS: {
			if (m_PeerCredentialsRequestedCallback) {
				m_PeerCredentialsRequestedCallback();
			}

			break;
		}
		case MessageType::PEER_CREDENTIALS: {
			std::istringstream ss(
				std::string(msg.data.cbegin(), msg.data.cend()));

			serialization::InputArchiveType iarchive(ss);
			PeerCredentials credentials;
			iarchive(credentials);

			if (m_PeerCredentialsReceivedCallback) {
				m_PeerCredentialsReceivedCallback(credentials, senderId);
			}

			break;
		}
		case MessageType::FULL_STATE: {
			std::istringstream ss(
				std::string(msg.data.cbegin(), msg.data.cend()));

			std::vector<PeerInfo> peers;
			ApplicationObjects entities;

			serialization::InputArchiveType iarchive(ss);
			iarchive(peers, entities);

			if (m_FullStateUpdateCallback) {
				m_FullStateUpdateCallback(peers, std::move(entities));
			}

			break;
		}
		case MessageType::AUTHORIZATION_SUCCEEDED: {
			std::istringstream ss(
				std::string(msg.data.cbegin(), msg.data.cend()));

			serialization::InputArchiveType iarchive(ss);
			PeerInfo peerInfo;
			iarchive(peerInfo);

			if (m_PeerAuthSuccessCallback) {
				m_PeerAuthSuccessCallback(peerInfo);
			}

			break;
		}
		case MessageType::AUTHORIZATION_FAILED: {
			if (m_PeerAuthFailedCallback) {
				m_PeerAuthFailedCallback();
			}

			break;
		}
		case MessageType::PEER_ADDED: {
			std::istringstream ss(
				std::string(msg.data.cbegin(), msg.data.cend()));

			serialization::InputArchiveType iarchive(ss);
			PeerInfo peerInfo;
			iarchive(peerInfo);

			if (m_PeerAddedCallback) {
				m_PeerAddedCallback(peerInfo);
			}

			break;
		}
		case MessageType::PEER_REMOVED: {
			std::istringstream ss(
				std::string(msg.data.cbegin(), msg.data.cend()));

			serialization::InputArchiveType iarchive(ss);
			PeerInfo peerInfo;
			iarchive(peerInfo);

			if (m_PeerRemovedCallback) {
				m_PeerRemovedCallback(peerInfo);
			}

			break;
		}
		case MessageType::LASER_UPDATED: {
			std::istringstream ss(
				std::string(msg.data.cbegin(), msg.data.cend()));

			serialization::InputArchiveType iarchive(ss);
			LaserUpdate laserUpdate;
			iarchive(laserUpdate);

			if (m_LaserUpdateCallback) {
				m_LaserUpdateCallback(laserUpdate, senderId);
			}

			break;
		}
		case MessageType::VOLUME_UPDATED: {
			std::istringstream ss(
				std::string(msg.data.cbegin(), msg.data.cend()));

			serialization::InputArchiveType iarchive(ss);
			VolumeUpdate volumeUpdate;
			iarchive(volumeUpdate);

			if (m_VolumeUpdateCallback) {
				m_VolumeUpdateCallback(volumeUpdate, senderId);
			}

			break;
		}
		case MessageType::WIDGET_EVENT: {
			std::istringstream ss(
				std::string(msg.data.cbegin(), msg.data.cend()));

			serialization::InputArchiveType iarchive(ss);
			WidgetUpdate widgetUpdate;
			iarchive(widgetUpdate);

			if (m_WidgetUpdateCallback) {
				m_WidgetUpdateCallback(widgetUpdate, senderId);
			}

			break;
		}
		case MessageType::PLANE_EVENT: {
			std::istringstream ss(
				std::string(msg.data.cbegin(), msg.data.cend()));

			serialization::InputArchiveType iarchive(ss);
			PlaneUpdate planeUpdate;
			iarchive(planeUpdate);

			if (m_PlaneUpdateCallback) {
				m_PlaneUpdateCallback(planeUpdate, senderId);
			}

			break;
		}
	}  // end switch
}
//=============================================================================

//=============================================================================
auto MessageEncoder::createLaserUpdateMsg(const LaserUpdate& laserUpdate)
	-> NetworkMessage
{
	std::ostringstream ss;
	{
		serialization::OutputArchiveType oarchive(ss);
		oarchive(laserUpdate);
	}
	auto byteString = ss.str();

	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::LASER_UPDATED;
	msg.data = {byteString.begin(), byteString.end()};
	msg.size = msg.data.size();

	return msg;
}
//=============================================================================

//=============================================================================
auto MessageEncoder::createVolumeUpdateMsg(const VolumeUpdate& volumeUpdate)
	-> NetworkMessage
{
	std::ostringstream ss;
	{
		serialization::OutputArchiveType oarchive(ss);
		oarchive(volumeUpdate);
	}
	auto byteString = ss.str();

	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::VOLUME_UPDATED;
	msg.data = {byteString.begin(), byteString.end()};
	msg.size = msg.data.size();

	return msg;
}
//=============================================================================

//=============================================================================
auto MessageEncoder::createWidgetUpdateMsg(const WidgetUpdate& widgetUpdate)
	-> NetworkMessage
{
	std::ostringstream ss;
	{
		serialization::OutputArchiveType oarchive(ss);
		oarchive(widgetUpdate);
	}
	auto byteString = ss.str();

	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::WIDGET_EVENT;
	msg.data = {byteString.begin(), byteString.end()};
	msg.size = msg.data.size();

	return msg;
}
//=============================================================================

//=============================================================================
auto MessageEncoder::createPlaneUpdateMsg(const PlaneUpdate& planeUpdate)
	-> NetworkMessage
{
	std::ostringstream ss;
	{
		serialization::OutputArchiveType oarchive(ss);
		oarchive(planeUpdate);
	}
	auto byteString = ss.str();

	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::PLANE_EVENT;
	msg.data = {byteString.begin(), byteString.end()};
	msg.size = msg.data.size();

	return msg;
}
//=============================================================================

//=============================================================================
auto MessageEncoder::createPeerAddedMsg(const PeerInfo& peerInfo)
	-> NetworkMessage
{
	std::ostringstream ss;
	{
		serialization::OutputArchiveType oarchive(ss);
		oarchive(peerInfo);
	}
	auto byteString = ss.str();

	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::PEER_ADDED;
	msg.data = {byteString.begin(), byteString.end()};
	msg.size = msg.data.size();

	return msg;
}
//=============================================================================

//=============================================================================
auto MessageEncoder::createPeerRemovedMsg(const PeerInfo& peerInfo)
	-> NetworkMessage
{
	std::ostringstream ss;
	{
		serialization::OutputArchiveType oarchive(ss);
		oarchive(peerInfo);
	}
	auto byteString = ss.str();

	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::PEER_REMOVED;
	msg.data = {byteString.begin(), byteString.end()};
	msg.size = msg.data.size();

	return msg;
}
//=============================================================================

//=============================================================================
auto MessageEncoder::createRequestCredentialsMsg() -> NetworkMessage
{
	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::REQUEST_CREDENTIALS;
	msg.size = 0;

	return msg;
}
//=============================================================================

//=============================================================================
auto MessageEncoder::createPeerCredentialsMsg(
	const PeerCredentials& credentials) -> NetworkMessage
{
	std::ostringstream ss;
	{
		serialization::OutputArchiveType oarchive(ss);
		oarchive(credentials);
	}
	auto byteString = ss.str();

	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::PEER_CREDENTIALS;
	msg.data = {byteString.begin(), byteString.end()};
	msg.size = msg.data.size();

	return msg;
}
//=============================================================================

//=============================================================================
auto MessageEncoder::createAuthenticationSucceededMsg(const PeerInfo& peerInfo)
	-> NetworkMessage
{
	std::ostringstream ss;
	{
		serialization::OutputArchiveType oarchive(ss);
		oarchive(peerInfo);
	}
	auto byteString = ss.str();

	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::AUTHORIZATION_SUCCEEDED;
	msg.data = {byteString.begin(), byteString.end()};
	msg.size = msg.data.size();

	return msg;
}
//=============================================================================

//=============================================================================
auto MessageEncoder::createAuthenticationFailedMsg() -> NetworkMessage
{
	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::AUTHORIZATION_FAILED;
	msg.size = 0;

	return msg;
}
//=============================================================================

//=============================================================================
auto MessageEncoder::createFullStateMsg(const std::vector<PeerInfo>& peers,
	const ApplicationObjects& entities) -> NetworkMessage {

	std::ostringstream ss;
	{
		serialization::OutputArchiveType oarchive(ss);
		oarchive(cereal::make_nvp("peers", peers),
			cereal::make_nvp("applicationEntities", entities));
	}
	auto byteString = ss.str();

	NetworkMessage msg;
	msg.header = 0x00;
	msg.type = NetworkMessage::FULL_STATE;
	msg.data = { byteString.begin(), byteString.end() };
	msg.size = msg.data.size();

	return msg;
}
//=============================================================================

//=============================================================================
void MessageEncoder::setOnCredentialsRequestedCallback(
	PeerCredentialsRequetedCallbackType clbk)
{
	m_PeerCredentialsRequestedCallback = clbk;
}
//=============================================================================

//=============================================================================
void MessageEncoder::setOnPeerCredentialsReceivedCallback(
	PeerCredentialsReceivedCallbackType clbk)
{
	m_PeerCredentialsReceivedCallback = clbk;
}
//=============================================================================

//=============================================================================
void MessageEncoder::setOnAuthorizationSuccessCallback(
	AuthorizationSuccessCallbackType clbk)
{
	m_PeerAuthSuccessCallback = clbk;
}
//=============================================================================

//=============================================================================
void MessageEncoder::setOnAuthorizationFailedCallback(
	AuthorizationFailedCallbackType clbk)
{
	m_PeerAuthFailedCallback = clbk;
}
//=============================================================================

//=============================================================================
void MessageEncoder::setOnPeerAddedCallback(PeerAddedCallbackType clbk)
{
	m_PeerAddedCallback = clbk;
}
//=============================================================================

//=============================================================================
void MessageEncoder::setOnPeerRemovedCallback(PeerRemovedCallbackType clbk)
{
	m_PeerRemovedCallback = clbk;
}
//=============================================================================

//=============================================================================
void MessageEncoder::setOnLaserUpdatedCallback(LaserUpdateCallbackType clbk)
{
	m_LaserUpdateCallback = clbk;
}
//=============================================================================

//=============================================================================
void MessageEncoder::setOnVolumeUpdatedCallback(VolumeUpdateCallbackType clbk)
{
	m_VolumeUpdateCallback = clbk;
}
//=============================================================================

//=============================================================================
void MessageEncoder::setOnWidgetUpdatedCallback(WidgetUpdateCallbackType clbk)
{
	m_WidgetUpdateCallback = clbk;
}
//=============================================================================

//=============================================================================
void MessageEncoder::setOnPlaneUpdatedCallback(PlaneUpdateCallbackType clbk)
{
	m_PlaneUpdateCallback = clbk;
}
//=============================================================================

//=============================================================================
void MessageEncoder::setOnFullStateUpdatedCallback(
	FullStateUpdateCallbackType clbk)
{
	m_FullStateUpdateCallback = clbk;
}
//=============================================================================
