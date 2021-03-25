#ifndef connectionImpl_h
#define connectionImpl_h

#include "networkMessageParser.h"

#include <QObject>
#include <QTcpSocket>

class QHostAddress;
class NetworkMessage;

class ConnectionImpl : public QObject
{
	Q_OBJECT;
public:
	ConnectionImpl();
	~ConnectionImpl();

public slots:
	void sendMessage(const NetworkMessage&);
	void connectToClient(qintptr socketDescriptor);
	void connectToServer(const QHostAddress& hostAddress, quint16 portNumber);
	void close();

signals:
	void messageReceived(const NetworkMessage&, QPrivateSignal);
	void disconnected(QPrivateSignal);
	void error(const QString&, QPrivateSignal);

private:
	NetworkMessageParser m_MessageParser;
	QTcpSocket m_Socket;
};

#endif