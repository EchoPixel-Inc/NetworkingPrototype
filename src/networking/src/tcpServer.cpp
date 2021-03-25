#include "networking/tcpServer.h"

//=============================================================================
TcpServer::TcpServer(QObject* parent) : QTcpServer(parent)
{
}
//=============================================================================

//=============================================================================
void TcpServer::incomingConnection(qintptr socketDescriptor)
{
	emit newConnection(socketDescriptor, QPrivateSignal{});
}
//=============================================================================