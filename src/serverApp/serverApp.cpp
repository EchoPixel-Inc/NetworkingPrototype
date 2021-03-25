#include "serverApp/serverApp.h"
#include "networking/tcpServer.h"
#include "networking/connection.h"
#include "appcore/messages.h"
#include "appcore/serializationHelper.h"
#include "appcore/serializationTypes.h"
#include "widgets/laserWidget.h"
#include "widgets/volumeWidget.h"
#include "widgets/splineWidget.h"
#include "widgets/planeWidget.h"

#include <cereal/types/string.hpp>
#include <cereal/types/array.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/vector.hpp>

#include <QTcpSocket>
#include <QColor>

#include <iostream>
#include <random>
#include <algorithm>
#include <sstream>
#include <vector>

namespace
{
QColor generateRandomColor()
{
	std::random_device rd;
	std::mt19937 generator{rd()};
	std::uniform_real_distribution<float> dist(0.0, 1.0);
	std::uniform_real_distribution<float> valDist(0.7, 1.0);

	QColor randomColor =
		QColor::fromHsvF(dist(generator), dist(generator), valDist(generator));

	return randomColor;
}

std::string generateSessionCode()
{
	const std::string chars =
		"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	std::random_device device;
	std::mt19937 generator(device());
	std::uniform_int_distribution<> distribution(0, chars.size() - 1);

	std::string randomString;
	const int strLength{2};

	for (int i = 0; i < strLength; i++) {
		randomString += chars[distribution(generator)];
	}

	return randomString;
}
}  // namespace

//==============================================================================
ServerApp::ServerApp(
	const QHostAddress& hostIP, std::optional<quint16> hostPort) :
	m_HostIP{hostIP},
	m_HostPort{hostPort},
	m_TcpServer{std::make_unique<TcpServer>()},
	m_NextAvailableConnectionId{0},
	m_NextAvailableWidgetId{0},
	m_Listening{false}
{
	QObject::connect(m_TcpServer.get(), &TcpServer::newConnection,
		[this](
			quintptr socketDescriptor) { onNewConnection(socketDescriptor); });

	QObject::connect(m_ApplicationObjects.volume.get(),
		&VolumeWidget::propertyUpdated, [this](const auto& propList) {
			messageAllClients(m_MessageEncoder.createVolumeUpdateMsg(VolumeUpdate(
				VolumeUpdate::MessageType::PROPERTY_UPDATE, propList)));
		});

	QObject::connect(m_ApplicationObjects.cutplane.get(),
		&PlaneWidget::propertyUpdated, [this](const auto& propList) {
			messageAllClients(m_MessageEncoder.createPlaneUpdateMsg(PlaneUpdate(
				PlaneUpdate::MessageType::PROPERTY_UPDATE, propList)));
		});

	m_MessageEncoder.setOnPeerCredentialsReceivedCallback(
		[this](const PeerCredentials& credentials, IdType connectionId) {
			authenticatePeer(
				connectionId, credentials.sessionCode, credentials.alias);
		});

	m_MessageEncoder.setOnLaserUpdatedCallback(
		[this](const LaserUpdate& laserUpdate, IdType connectionId) {
			onLaserUpdated(laserUpdate, connectionId);
		});

	m_MessageEncoder.setOnVolumeUpdatedCallback(
		[this](const VolumeUpdate& volumeUpdate, IdType connectionId) {
			onVolumeUpdated(volumeUpdate, connectionId);
		});

	m_MessageEncoder.setOnWidgetUpdatedCallback(
		[this](const WidgetUpdate& widgetUpdate, IdType connectionId) {
			onWidgetUpdated(widgetUpdate, connectionId);
		});

	m_MessageEncoder.setOnPlaneUpdatedCallback(
		[this](const PlaneUpdate& planeUpdate, IdType connectionId) {
			onPlaneUpdated(planeUpdate, connectionId);
		});
}
//==============================================================================

//==============================================================================
ServerApp::~ServerApp()
{
	m_TcpServer->close();
}
//==============================================================================

//==============================================================================
bool ServerApp::listen(const QHostAddress& address, quint16 portNumber)
{
	std::cout << "Server is attempting to listen at "
			  << address.toString().toStdString() << ", port " << portNumber
			  << std::endl;

	if (m_TcpServer->listen(address, portNumber)) {
		m_SessionCode = generateSessionCode();

		std::cout << "Server is listening at "
				  << address.toString().toStdString() << ", port " << portNumber
				  << std::endl;

		std::cout << "************************************************\n";
		std::cout << "   Session code is: " << m_SessionCode << "\n";
		std::cout << "************************************************"
				  << std::endl;
	}

	return isListening();
}
//==============================================================================

//==============================================================================
bool ServerApp::isListening() const
{
	return m_TcpServer->isListening();
}
//==============================================================================

//==============================================================================
void ServerApp::close()
{
	shutdown();
	m_Listening = false;
}
//==============================================================================

//==============================================================================
void ServerApp::shutdown()
{
	std::cout << "Shutting down the server..." << std::endl;
	m_TcpServer->close();
}
//==============================================================================

//==============================================================================
void ServerApp::onNewConnection(qintptr socketDescriptor)
{
	if (!m_TcpServer->isListening()) {
		return;
	}

	auto newConnection = std::make_unique<Connection>();
	auto connectionId = m_NextAvailableConnectionId++;

	std::cout << "Creating new connection with id = " << connectionId
			  << std::endl;

	QObject::connect(
		newConnection.get(), &Connection::messageReceived, newConnection.get(),
		[this, connectionId](
			const auto& msg) { m_MessageEncoder.processMessage(msg, connectionId); },
		Qt::AutoConnection);

	QObject::connect(
		newConnection.get(), &Connection::disconnected, newConnection.get(),
		[this, connectionId] {
			if (removePeer(connectionId)) {
				messageAllClients(
					m_MessageEncoder.createPeerRemovedMsg(PeerInfo{connectionId}));
			}
		},
		Qt::AutoConnection);

	QObject::connect(
		newConnection.get(), &Connection::error, newConnection.get(),
		[this, connectionId](const QString& errorMsg) {
			std::cerr << "Error received from peer " << connectionId << ": "
					  << errorMsg.toStdString() << std::endl;

			// TODO: What if the error message isn't related to the socket
			// closing?
			if (removePeer(connectionId)) {
				messageAllClients(
					m_MessageEncoder.createPeerRemovedMsg(PeerInfo{connectionId}));
			}
		},
		Qt::AutoConnection);

	newConnection->connectToClient(socketDescriptor);

	ConnectionInfo newConnectionInfo;
	newConnectionInfo.connection = std::move(newConnection);
	newConnectionInfo.validated = false;
	newConnectionInfo.color = ColorVectorType{0.0, 0.0, 0.0};
	newConnectionInfo.alias = "";

	m_Connections.insert({connectionId, std::move(newConnectionInfo)});

	messageOneClient(m_MessageEncoder.createRequestCredentialsMsg(), connectionId);
}
//==============================================================================

//==============================================================================
void ServerApp::messageAllClients(const NetworkMessage& msg)
{
	for (const auto& [id, connectionInfo] : m_Connections) {
		if (connectionInfo.validated) {
			connectionInfo.connection->sendMessage(msg);
		}
	}
}
//==============================================================================

//==============================================================================
void ServerApp::messageOneClient(const NetworkMessage& msg, IdType connectionId)
{
	if (auto it = m_Connections.find(connectionId); it != m_Connections.end()) {
		auto& connectionInfo = it->second;
		connectionInfo.connection->sendMessage(msg);
	}
}
//==============================================================================

//==============================================================================
void ServerApp::authenticatePeer(IdType connectionId,
	const std::string& sessionCode, const std::string& alias)
{
	std::cout << "Received credentials from connection " << connectionId
			  << " : " << sessionCode << ", " << alias << std::endl;

	if (auto it = m_Connections.find(connectionId); it != m_Connections.end()) {
		auto& connectionInfo = it->second;

		if (sessionCode != m_SessionCode) {
			// peer provided bad credentials. Kick them off
			messageOneClient(
				m_MessageEncoder.createAuthenticationFailedMsg(), connectionId);

			connectionInfo.connection->close();
		}
		else {
			// setup the new peer

			// Add a new laser
			auto newLaser = std::make_unique<LaserWidget>();
			auto newColor = generateRandomColor();
			ColorVectorType color{
				newColor.redF(), newColor.greenF(), newColor.blueF()};

			std::cout << "Assigning new peer color: (" << color[0] << ", "
					  << color[1] << ", " << color[2] << ")" << std::endl;

			connectionInfo.alias = alias;
			connectionInfo.color = color;
			connectionInfo.validated = true;

			newLaser->setColor(color[0], color[1], color[2]);

			QObject::connect(newLaser.get(), &LaserWidget::propertyUpdated,
				[this, id = connectionId](const auto& propList) {
					messageAllClients(m_MessageEncoder.createLaserUpdateMsg(
						LaserUpdate(propList, id)));
				});

			PeerInfo info{connectionId, alias, color};

			m_ApplicationObjects.lasers.insert(
				{connectionId, std::move(newLaser)});

			messageOneClient(m_MessageEncoder.createAuthenticationSucceededMsg(info),
				connectionId);

			std::vector<PeerInfo> peers;
			for (const auto& [id, connectionInfo] : m_Connections) {
				peers.push_back(
					PeerInfo{ id, connectionInfo.alias, connectionInfo.color });
			}
			messageOneClient(m_MessageEncoder.createFullStateMsg(peers,
				m_ApplicationObjects), connectionId);

			// Notify the other peers
			messageAllClients(m_MessageEncoder.createPeerAddedMsg(info));
		}
	}
}
//==============================================================================

//==============================================================================
bool ServerApp::removePeer(IdType connectionId)
{
	std::cout << "Removing peer connection with id = " << connectionId
			  << std::endl;

	m_ApplicationObjects.lasers.erase(connectionId);

	// Make sure to release any lingering object ownership
	if (m_VolumeOwner.has_value() && (m_VolumeOwner.value() == connectionId)) {
		m_VolumeOwner = std::nullopt;
	}

	if (m_PlaneOwner.has_value() && (m_PlaneOwner.value() == connectionId)) {
		m_PlaneOwner = std::nullopt;
	}

	for (auto& [widgetId, widgetOwner] : m_WidgetOwnershipMap) {
		if (widgetOwner.has_value() && (widgetOwner.value() == connectionId)) {
			widgetOwner = std::nullopt;
		}
	}

	return (m_Connections.erase(connectionId) > 0);
}
//==============================================================================

//==============================================================================
void ServerApp::onLaserUpdated(
	const LaserUpdate& laserUpdate, IdType connectionId)
{
	if (auto it = m_ApplicationObjects.lasers.find(connectionId);
		it != m_ApplicationObjects.lasers.end()) {
		it->second->updateProperties(laserUpdate.propList);
	}
}
//==============================================================================

//==============================================================================
void ServerApp::onVolumeUpdated(
	const VolumeUpdate& volumeUpdate, IdType connectionId)
{
	switch (volumeUpdate.msgType) {
		case VolumeUpdate::MessageType::INTERACTION_ENDED: {
			if (m_VolumeOwner.has_value() &&
				(m_VolumeOwner.value() == connectionId)) {
				m_VolumeOwner = std::nullopt;

				messageAllClients(m_MessageEncoder.createVolumeUpdateMsg(
					VolumeUpdate(VolumeUpdate::MessageType::INTERACTION_ENDED,
						{}, connectionId)));
			}
			break;
		}
		case VolumeUpdate::MessageType::PROPERTY_UPDATE: {
			if (m_VolumeOwner.has_value()) {
				if (m_VolumeOwner.value() == connectionId) {
					m_ApplicationObjects.volume->updateProperties(
						volumeUpdate.propList);
				}
			}
			else {	// volume has a new owner
				m_VolumeOwner = connectionId;

				messageAllClients(m_MessageEncoder.createVolumeUpdateMsg(
					VolumeUpdate(VolumeUpdate::MessageType::INTERACTION_STARTED,
						{}, connectionId)));

				m_ApplicationObjects.volume->updateProperties(
					volumeUpdate.propList);
			}
			break;
		}
	}
}
//==============================================================================

//==============================================================================
void ServerApp::onWidgetUpdated(
	const WidgetUpdate& widgetUpdate, IdType connectionId)
{
	switch (widgetUpdate.msgType) {
		case WidgetUpdate::MessageType::CREATE: {
			auto widgetId = m_NextAvailableWidgetId++;

			auto newWidget = std::make_unique<SplineWidget>();
			newWidget->attach(m_ApplicationObjects.volume.get());

			QObject::connect(newWidget.get(), &SplineWidget::propertyUpdated,
				[this, id = widgetId](const auto& propList) {
					messageAllClients(m_MessageEncoder.createWidgetUpdateMsg(
						WidgetUpdate(WidgetUpdate::MessageType::PROPERTY_UPDATE,
							id, propList)));
				});

			// Immediately grant property request changes
			QObject::connect(newWidget.get(),
				&SplineWidget::requestPropertyUpdate, newWidget.get(),
				&SplineWidget::updateProperties);

			m_ApplicationObjects.widgets.insert(
				{widgetId, std::move(newWidget)});

			m_WidgetOwnershipMap.insert({widgetId, connectionId});

			messageAllClients(m_MessageEncoder.createWidgetUpdateMsg(WidgetUpdate(
				widgetUpdate.msgType, widgetId, {}, connectionId)));

			break;
		}
		case WidgetUpdate::MessageType::DESTROY: {
			m_WidgetOwnershipMap.erase(widgetUpdate.widgetId);
			if (m_ApplicationObjects.widgets.erase(
					widgetUpdate.widgetId) > 0) {
				messageAllClients(
					m_MessageEncoder.createWidgetUpdateMsg(widgetUpdate));
			}

			break;
		}
		case WidgetUpdate::MessageType::PROPERTY_UPDATE: {
			auto& widgets = m_ApplicationObjects.widgets;
			if (auto it = widgets.find(widgetUpdate.widgetId);
				it != widgets.end()) {
				if (m_WidgetOwnershipMap.at(widgetUpdate.widgetId)
						.has_value()) {
					if (m_WidgetOwnershipMap.at(widgetUpdate.widgetId)
							.value() == connectionId) {
						it->second->updateProperties(widgetUpdate.propList);
					}
				}
				else {	// widget has a new owner
					m_WidgetOwnershipMap.at(widgetUpdate.widgetId) =
						connectionId;
					it->second->updateProperties(widgetUpdate.propList);
				}
			}

			break;
		}
		case WidgetUpdate::MessageType::INTERACTION_ENDED: {
			if (auto it = m_WidgetOwnershipMap.find(widgetUpdate.widgetId);
				it != m_WidgetOwnershipMap.end()) {
				if (it->second.has_value() &&
					(it->second.value() == connectionId)) {
					it->second = std::nullopt;
				}
			}

			break;
		}
	}  // end widget update switch
}
//==============================================================================

//==============================================================================
void ServerApp::onPlaneUpdated(
	const PlaneUpdate& planeUpdate, IdType connectionId)
{
	switch (planeUpdate.msgType) {
		case PlaneUpdate::MessageType::INTERACTION_ENDED: {
			if (m_PlaneOwner.has_value() &&
				(m_PlaneOwner.value() == connectionId)) {
				m_PlaneOwner = std::nullopt;
			}
			break;
		}
		case PlaneUpdate::MessageType::PROPERTY_UPDATE: {
			if (m_PlaneOwner.has_value()) {
				if (m_PlaneOwner.value() == connectionId) {
					m_ApplicationObjects.cutplane->updateProperties(
						planeUpdate.propList);
				}
			}
			else {	// plane widget has a new owner
				m_PlaneOwner = connectionId;

				m_ApplicationObjects.cutplane->updateProperties(
					planeUpdate.propList);
			}
			break;
		}
	}  // end message type switch
}
//==============================================================================