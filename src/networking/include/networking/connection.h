#ifndef connection_h
#define connection_h

#include "networkMessage.h"

#include <QObject>

#include <memory>

class NetworkMessage;
class QHostAddress;

class Connection : public QObject
{
	Q_OBJECT;

public:
	explicit Connection();
	~Connection();

signals:
	void connectToClient(qintptr socketDescriptor);
	void connectToServer(const QHostAddress& clientAddress, quint16 portNumber);
	void sendMessage(const NetworkMessage&);
	void close();
	
	void error(const QString&, QPrivateSignal);
	void disconnected(QPrivateSignal);
	void messageReceived(const NetworkMessage&, QPrivateSignal);

private:
	class ConnectionThread;
	std::unique_ptr<ConnectionThread> m_ConnectionThread;
};

#endif