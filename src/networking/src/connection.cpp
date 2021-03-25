#include "networking/connection.h"
#include "networking/connectionImpl.h"
#include "networking/networkMessage.h"

#include <QHostAddress>
#include <QThread>

#include <iostream>

static constexpr std::uint32_t maxMessageSize = 524288000;

class Connection::ConnectionThread : public QThread
{
public:
	explicit ConnectionThread(
		std::unique_ptr<ConnectionImpl> impl, QObject* parent = nullptr);

	~ConnectionThread();

protected:
	void run() override;

private:
	std::unique_ptr<ConnectionImpl> m_Impl;
};

//==============================================================================
// ConnectionImpl
//==============================================================================
ConnectionImpl::ConnectionImpl() : m_Socket{this}
{
	std::cout << "ConnectionImpl::constructor" << std::endl;

	m_MessageParser.setMessageReadyCallback([this](const auto& msg) {
		emit messageReceived(msg, QPrivateSignal{});
	});

	QObject::connect(&m_Socket, &QTcpSocket::readyRead, this, [this] {
		while (m_Socket.bytesAvailable()) {
			m_MessageParser.parse(m_Socket.readAll());
			if (m_MessageParser.getCurrentMessageSize() > maxMessageSize) {
				m_Socket.close();

				emit error("Max message length exceeded", QPrivateSignal{});
				break;
			}
		}
	});

	QObject::connect(
		&m_Socket, &QTcpSocket::disconnected, this,
		[this] { emit disconnected(QPrivateSignal{}); }, Qt::AutoConnection);

	QObject::connect(
		&m_Socket,
		QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
		this,
		[this](QAbstractSocket::SocketError) {
			emit error("Socket error", QPrivateSignal{});
		},
		Qt::AutoConnection);

	qRegisterMetaType<NetworkMessage>("NetworkMessage");
	qRegisterMetaType<QHostAddress>("QHostAddress");
	qRegisterMetaType<qintptr>("qintptr");
}
//==============================================================================

//==============================================================================
ConnectionImpl::~ConnectionImpl()
{
	std::cout << "ConnectionImpl::destructor" << std::endl;
	m_Socket.close();
}
//==============================================================================

//==============================================================================
void ConnectionImpl::connectToClient(qintptr socketDescriptor)
{
	std::cout << "ConnectionImpl::connectToClient" << std::endl;
	m_Socket.setSocketDescriptor(socketDescriptor);
}
//==============================================================================

//==============================================================================
void ConnectionImpl::connectToServer(
	const QHostAddress& hostAddress, quint16 portNumber)
{
	std::cout << "ConnectionImpl::connectToServer" << std::endl;
	m_Socket.connectToHost(hostAddress, portNumber);
}
//==============================================================================

//==============================================================================
void ConnectionImpl::sendMessage(const NetworkMessage& msg)
{
	m_Socket.write(msg.serialize());
}
//==============================================================================

//==============================================================================
void ConnectionImpl::close()
{
	m_Socket.disconnectFromHost();
}
//==============================================================================
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//==============================================================================
// ConnectionThread
//==============================================================================
Connection::ConnectionThread::ConnectionThread(
	std::unique_ptr<ConnectionImpl> connectionImpl, QObject* parent) :
	QThread{parent},
	m_Impl{std::move(connectionImpl)}
{
	std::cout << "ConnectionThread::constructor" << std::endl;
	m_Impl->moveToThread(this);
}
//==============================================================================

//==============================================================================
Connection::ConnectionThread::~ConnectionThread()
{
	std::cout << "ConnectionThread::destructor" << std::endl;
}
//==============================================================================

//==============================================================================
void Connection::ConnectionThread::run()
{
	std::cout << "ConnectionThread::run" << std::endl;
	QThread::run();
	m_Impl.reset();
}
//==============================================================================
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//==============================================================================
// Connection
//==============================================================================
Connection::Connection()
{
	std::cout << "Connection::constructor" << std::endl;
	auto connectionImpl = std::make_unique<ConnectionImpl>();

	QObject::connect(this, &Connection::connectToClient, connectionImpl.get(),
		&ConnectionImpl::connectToClient, Qt::AutoConnection);

	QObject::connect(this, &Connection::connectToServer, connectionImpl.get(),
		&ConnectionImpl::connectToServer, Qt::AutoConnection);

	QObject::connect(this, &Connection::sendMessage, connectionImpl.get(),
		&ConnectionImpl::sendMessage, Qt::AutoConnection);

	QObject::connect(this, &Connection::close, connectionImpl.get(),
		&ConnectionImpl::close, Qt::AutoConnection);

	QObject::connect(
		connectionImpl.get(), &ConnectionImpl::disconnected, this,
		[this] {
			m_ConnectionThread->quit();
			m_ConnectionThread->wait();

			emit disconnected(QPrivateSignal{});
		},
		Qt::AutoConnection);

	QObject::connect(
		connectionImpl.get(), &ConnectionImpl::error, this,
		[this](const QString& errorMsg) {
			emit error(errorMsg, QPrivateSignal{});
		},
		Qt::AutoConnection);

	QObject::connect(
		connectionImpl.get(), &ConnectionImpl::messageReceived, this,
		[this](const NetworkMessage& msg) {
			emit messageReceived(msg, QPrivateSignal{});
		},
		Qt::AutoConnection);

	m_ConnectionThread =
		std::make_unique<ConnectionThread>(std::move(connectionImpl));

	m_ConnectionThread->start();
}
//==============================================================================

//==============================================================================
Connection::~Connection()
{
	std::cout << "Connection::destructor" << std::endl;
	m_ConnectionThread->quit();
	m_ConnectionThread->wait();
}
//==============================================================================