#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QQueue>
#include "filepackage.h"

//------------------------------------------------------------------------------------
/**
	Sends data to host DataReceiver
*/
//------------------------------------------------------------------------------------

class DataSender :
	public QTcpSocket
{
	Q_OBJECT
public:
	/// constructor
	DataSender(void);
	/// destructor
	~DataSender(void);

	/// open sender
	bool Open();
	/// closes sender
	void Close();
	/// updates sender
	void Update();

	/// sends data
	void Send(const FilePackage& data);

	/// sets address
	void SetAddress(const QHostAddress& address);
	/// sets port
	void SetPort(const quint16 port);

	/// called whenever there is anything to read
	void OnRead();
private:
	QMap<QString, FilePackage> pendingPackages;
	QHostAddress address;
	quint16 port;
};


//------------------------------------------------------------------------------
/**
*/
inline void 
DataSender::SetAddress( const QHostAddress& address )
{
	this->address = address;
}

//------------------------------------------------------------------------------
/**
*/
inline void 
DataSender::SetPort( const quint16 port )
{
	this->port = port;
}
