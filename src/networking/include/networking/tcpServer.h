#ifndef tcpServer_h
#define tcpServer_h

#include <QTcpServer>

class TcpServer : public QTcpServer
{
	Q_OBJECT;

public:

	TcpServer(QObject* parent = nullptr);

signals:
		
	void newConnection(qintptr, QPrivateSignal);

protected:

	void incomingConnection(qintptr socketDescriptor) override;
};

#endif