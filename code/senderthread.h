#pragma once
#include <QThread>
#include <QQueue>
#include "filepackage.h"
#include "datasender.h"

//------------------------------------------------------------------------------------
/**
	Thread which sends data
*/
//------------------------------------------------------------------------------------
class SenderThread :
	public QThread
{
	Q_OBJECT
public:
	/// constructor
	SenderThread(void);
	/// destructor
	~SenderThread(void);

	/// sets the address
	void SetAddress(const QHostAddress& address);
	/// gets address
	const QHostAddress& GetAddress() const;
	/// sets the port
	void SetPort(const quint16 port);
	/// gets the port
	const quint16 GetPort() const;

	/// queues message
	void Enqueue(const FilePackage& data);
	/// returns true if the sender is connected
	bool IsConnected() const;

	/// starts thread
	void Start();
	/// stops thread
	void Stop();



public slots:
	/// called whenever the data sender is connected
	void OnConnected();
	/// called whenever the data sender is disconnected
	void OnDisconnected();

signals:
	/// emit when connection is successful
	void ConnectionSuccessful();
	/// emit when connection is killed
	void Disconnected();
private:
	/// runs thread
	void run();

	QQueue<FilePackage> dataQueue;
	DataSender* sender;
	QHostAddress address;
	QMutex mutex;
	quint16 port;
	bool connectionOpen;
	bool shouldStop;
};

//------------------------------------------------------------------------------
/**
*/
inline void 
SenderThread::SetAddress( const QHostAddress& address )
{
	this->address = address;
}

//------------------------------------------------------------------------------
/**
*/
inline const QHostAddress& 
SenderThread::GetAddress() const
{
	return this->address;
}


//------------------------------------------------------------------------------
/**
*/
inline void 
SenderThread::SetPort( const quint16 port )
{
	this->port = port;
}

//------------------------------------------------------------------------------
/**
*/
inline const quint16 
SenderThread::GetPort() const
{
	return this->port;
}

//------------------------------------------------------------------------------
/**
*/
inline bool 
SenderThread::IsConnected() const
{
	return this->connectionOpen;
}