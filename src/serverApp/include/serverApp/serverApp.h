#ifndef serverApp_h
#define serverApp_h

#include "common/coreTypes.h"
#include "networking/networkMessage.h"
#include "appcore/applicationObjects.h"
#include "appcore/messageEncoder.h"

#include <QHostAddress>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <optional>

class TcpServer;
class Connection;

class ServerApp
{
public:
	explicit ServerApp(const QHostAddress& hostIP,
		std::optional<quint16> hostPort = std::nullopt);
	~ServerApp();

	bool listen(const QHostAddress& address, quint16 port);
	bool isListening() const;
	void close();

protected:
	using MessageType = NetworkMessage;
	using ColorVectorType = common::ColorVectorType;
	using IdType = common::IdType;

	void messageAllClients(const NetworkMessage&);
	void messageOneClient(const NetworkMessage&, IdType);
	void authenticatePeer(IdType connectionId,
		const std::string& sessionCode, const std::string& nickname);
	bool removePeer(IdType peerId);

	void onNewConnection(qintptr socketDescriptor);

	void onLaserUpdated(const LaserUpdate&, IdType connectionId);
	void onVolumeUpdated(const VolumeUpdate&, IdType connectionId);
	void onWidgetUpdated(const WidgetUpdate&, IdType connectionId);
	void onPlaneUpdated(const PlaneUpdate&, IdType connectionId);

private:
	void shutdown();

	// Custom struct to hold all the relevant connection information
	struct ConnectionInfo
	{
		std::unique_ptr<Connection> connection;
		std::string alias;
		ColorVectorType color;
		bool validated;
	};

	using ConnectionMap = std::unordered_map<IdType, ConnectionInfo>;
	using WidgetOwnershipMap = std::unordered_map<IdType,
		std::optional<IdType>>;

	QHostAddress m_HostIP;
	std::optional<quint16> m_HostPort;
	std::unique_ptr<TcpServer> m_TcpServer;
	bool m_Listening;
	std::string m_SessionCode;
	ConnectionMap m_Connections;
	WidgetOwnershipMap m_WidgetOwnershipMap;
	std::optional<IdType> m_VolumeOwner;
	std::optional<IdType> m_PlaneOwner;
	ApplicationObjects m_ApplicationObjects;
	MessageEncoder m_MessageEncoder;
	IdType m_NextAvailableConnectionId;
	IdType m_NextAvailableWidgetId;
};

#endif