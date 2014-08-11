#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QQueue>
#include <QDataStream>
#include <QTimer>
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
	void Read();
	/// called to update outgoing messages
	void Write();

	/// called whenever we want to abort the current file
	void AbortCurrent();

    /// returns true whether or not the sender is sending
    bool IsSending() const;

    /// toggles pausing
    void TogglePause();

private slots:
    /// called whenever the data sender invokes a keepalive
    void KeepAlive();
signals:
	/// called when the file is denied
	void FileDenied(const QString& file);
	/// called whenever a new connection is established
	void FileDone(const QString& file);
	/// emitted when a file has started downloading
	void FileStarted(const QString& file, int chunks);
	/// emitted when a file is progressing
	void FileProgress(const QString& file, int numBytes);
private:
	QDataStream stream;
	QList<QByteArray> messages;
	QMap<QString, FilePackage> pendingPackages;
	QHostAddress address;
    QTimer keepAliveTimer;
	quint16 port;
    bool isSending;
    bool isPaused;
	bool abortCurrent;
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

//------------------------------------------------------------------------------
/**
*/
inline void 
DataSender::AbortCurrent()
{
	this->abortCurrent = true;
}

//------------------------------------------------------------------------------
/**
*/
inline bool 
DataSender::IsSending() const
{
    return this->isSending;
}