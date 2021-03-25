#ifndef messageEncoder_h
#define messageEncoder_h

#include "common/coreTypes.h"
#include "networking/networkMessage.h"

#include <functional>
#include <array>
#include <optional>

class PeerInfo;
class PeerCredentials;
class LaserUpdate;
class VolumeUpdate;
class WidgetUpdate;
class PlaneUpdate;
class ApplicationObjects;

class MessageEncoder
{
public:
	using ColorVectorType = common::ColorVectorType;
	using IdType = common::IdType;
	using PeerCredentialsRequetedCallbackType = std::function<void()>;
	using PeerCredentialsReceivedCallbackType =
		std::function<void(const PeerCredentials&, IdType)>;

	using AuthorizationSuccessCallbackType =
		std::function<void(const PeerInfo&)>;

	using AuthorizationFailedCallbackType = std::function<void()>;
	using PeerAddedCallbackType = std::function<void(const PeerInfo&)>;
	using PeerRemovedCallbackType = std::function<void(const PeerInfo&)>;
	using LaserUpdateCallbackType =
		std::function<void(const LaserUpdate&, IdType)>;

	using VolumeUpdateCallbackType =
		std::function<void(const VolumeUpdate&, IdType)>;

	using WidgetUpdateCallbackType =
		std::function<void(const WidgetUpdate&, IdType)>;

	using PlaneUpdateCallbackType =
		std::function<void(const PlaneUpdate&, IdType)>;

	// expect the destination to take ownership of these items as we want to 
	// avoid a fully copy
	using FullStateUpdateCallbackType =
		std::function<void(const std::vector<PeerInfo>&, ApplicationObjects&&)>;

	MessageEncoder() = default;
	~MessageEncoder() = default;

	MessageEncoder(const MessageEncoder&) = default;
	MessageEncoder& operator=(const MessageEncoder&) = default;

	MessageEncoder(MessageEncoder&&) = default;
	MessageEncoder& operator=(MessageEncoder&&) = default;

	void processMessage(const NetworkMessage&, IdType senderId = 0);

	NetworkMessage createLaserUpdateMsg(const LaserUpdate&);
	NetworkMessage createVolumeUpdateMsg(const VolumeUpdate&);
	NetworkMessage createWidgetUpdateMsg(const WidgetUpdate&);
	NetworkMessage createPlaneUpdateMsg(const PlaneUpdate&);
	NetworkMessage createPeerAddedMsg(const PeerInfo&);
	NetworkMessage createPeerRemovedMsg(const PeerInfo&);
	NetworkMessage createRequestCredentialsMsg();
	NetworkMessage createPeerCredentialsMsg(const PeerCredentials&);
	NetworkMessage createAuthenticationSucceededMsg(const PeerInfo&);
	NetworkMessage createAuthenticationFailedMsg();
	NetworkMessage createFullStateMsg(const std::vector<PeerInfo>&,
		const ApplicationObjects&);

	void setOnCredentialsRequestedCallback(
		PeerCredentialsRequetedCallbackType clbk);
	void setOnPeerCredentialsReceivedCallback(
		PeerCredentialsReceivedCallbackType clbk);
	void setOnAuthorizationSuccessCallback(
		AuthorizationSuccessCallbackType clbk);
	void setOnAuthorizationFailedCallback(AuthorizationFailedCallbackType clbk);
	void setOnPeerAddedCallback(PeerAddedCallbackType);
	void setOnPeerRemovedCallback(PeerRemovedCallbackType);
	void setOnLaserUpdatedCallback(LaserUpdateCallbackType);
	void setOnVolumeUpdatedCallback(VolumeUpdateCallbackType);
	void setOnWidgetUpdatedCallback(WidgetUpdateCallbackType);
	void setOnPlaneUpdatedCallback(PlaneUpdateCallbackType);
	void setOnFullStateUpdatedCallback(FullStateUpdateCallbackType);

private:
	PeerCredentialsRequetedCallbackType m_PeerCredentialsRequestedCallback;
	PeerCredentialsReceivedCallbackType m_PeerCredentialsReceivedCallback;
	AuthorizationSuccessCallbackType m_PeerAuthSuccessCallback;
	AuthorizationFailedCallbackType m_PeerAuthFailedCallback;
	PeerAddedCallbackType m_PeerAddedCallback;
	PeerRemovedCallbackType m_PeerRemovedCallback;
	LaserUpdateCallbackType m_LaserUpdateCallback;
	VolumeUpdateCallbackType m_VolumeUpdateCallback;
	WidgetUpdateCallbackType m_WidgetUpdateCallback;
	PlaneUpdateCallbackType m_PlaneUpdateCallback;
	FullStateUpdateCallbackType m_FullStateUpdateCallback;
};

#endif
